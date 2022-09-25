#pragma compute
#version 450 core

// Pre-filters environment cube map using GGX NDF importance sampling
// Part of specular IBL split-sum approximation
// Reference: Michal Siejak's PBR project: https://github.com/Nadrin
// Reference: Joey de Vries's LearnOpenGL Book: https://learnopengl.com/

const float PI = 3.14159265358979323846f;
const float TwoPI = 2 * PI;
const float Epsilon = 0.00001f;

const uint Samples = 1024;
const float InvNumSamples = 1.0f / float(Samples);

const int NumMipLevels = 1;
layout(binding = 0) uniform samplerCube u_UnfilteredTexture;
layout(binding = 0, rgba32f) restrict writeonly uniform imageCube o_FilteredTexture[NumMipLevels];

layout (push_constant) uniform Uniforms
{
	float Roughness;
} u_Uniforms;

#define PARAM_LEVEL     0

// ----------------------------------------------------------------------------
// Compute Van Der Corpus radical inverse
// http://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html
// efficient VanDerCorpus calculation.
float RadicalInverse_VdC(uint bits)
{
	bits = (bits << 16u) | (bits >> 16u);
	bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
	bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
	bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
	bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);

	return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

// ----------------------------------------------------------------------------
// Sample ith point from Hammersley point set of NumSamples points total
vec2 Hammersley(uint i)
{
	return vec2(i * InvNumSamples, RadicalInverse_VdC(i));
}

// ----------------------------------------------------------------------------
// Importance sample GGX normal distribution function for a fixed roughness value.
// This returns normalized half-vector between Li & Lo.
// For derivation see: http://blog.tobias-franke.eu/2014/03/30/notes_on_importance_sampling.html
vec3 ImportanceSampleGGX(float u1, float u2, float roughness)
{
	float alpha = roughness * roughness;

	float cosTheta = sqrt((1.0f - u2) / (1.0f + (alpha * alpha - 1.0f) * u2));
	float sinTheta = sqrt(1.0f - cosTheta * cosTheta); // Trig. identity
	float phi = TwoPI * u1;

	// Convert to Cartesian upon return.
	return vec3(sinTheta * cos(phi), sinTheta * sin(phi), cosTheta);
}

// ----------------------------------------------------------------------------
// GGX/Towbridge-Reitz normal distribution function.
// Uses Disney's reparametrization of alpha = roughness^2.
float NDFGGX(float cosLh, float roughness)
{
	float alpha   = roughness * roughness;
	float alphaSq = alpha * alpha;

	float denom = (cosLh * cosLh) * (alphaSq - 1.0) + 1.0;

	return alphaSq / (PI * denom * denom);
}

// ----------------------------------------------------------------------------
// Read EquiRectangularToCubeMap shader for more info on this!
vec3 GetCubeMapTexCoord(vec2 outputImageSize)
{
    vec2 st = gl_GlobalInvocationID.xy / outputImageSize;
    vec2 uv = 2.0f * vec2(st.x, 1.0f - st.y) - vec2(1.0f);

    vec3 ret;

    if (gl_GlobalInvocationID.z == 0u)      ret = vec3(  1.0f, uv.y,  -uv.x);
    else if (gl_GlobalInvocationID.z == 1u) ret = vec3( -1.0f, uv.y,   uv.x);
    else if (gl_GlobalInvocationID.z == 2u) ret = vec3(  uv.x,  1.0f,  -uv.y);
    else if (gl_GlobalInvocationID.z == 3u) ret = vec3(  uv.x, -1.0f,   uv.y);
    else if (gl_GlobalInvocationID.z == 4u) ret = vec3(  uv.x,  uv.y,   1.0f);
    else if (gl_GlobalInvocationID.z == 5u) ret = vec3( -uv.x,  uv.y,  -1.0f);

    return normalize(ret);
}

// ----------------------------------------------------------------------------
// Compute orthonormal basis for converting from tanget/shading space to world space.
void ComputeBasisVectors(const vec3 N, out vec3 S, out vec3 T)
{
	// Branchless select non-degenerate T.
	T = cross(N, vec3(0.0f, 1.0f, 0.0f));
	T = mix(cross(N, vec3(1.0f, 0.0f, 0.0f)), T, step(Epsilon, dot(T, T)));

	T = normalize(T);
	S = normalize(cross(N, T));
}

// ----------------------------------------------------------------------------
// Convert point from tangent/shading space to world space.
vec3 TangentToWorld(const vec3 v, const vec3 N, const vec3 S, const vec3 T)
{
	return S * v.x + T * v.y + N * v.z;
}

layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;

void main()
{
	// Make sure we won't write past output when computing higher mipmap levels.
	ivec2 outputSize = imageSize(o_FilteredTexture[PARAM_LEVEL]);
	if(gl_GlobalInvocationID.x >= outputSize.x || gl_GlobalInvocationID.y >= outputSize.y) 
	{
		return;
	}

	// Solid angle associated with a single cubemap texel at zero mipmap level.
	// This will come in handy for importance sampling below.
	vec2 inputSize = vec2(textureSize(u_UnfilteredTexture, 0));
	float wt = 4.0f * PI / (6 * inputSize.x * inputSize.y);

	// Approximation: Assume zero viewing angle (isotropic filtering)
	vec3 N = GetCubeMapTexCoord(vec2(imageSize(o_FilteredTexture[PARAM_LEVEL])));
	vec3 Lo = N;

	vec3 S;
	vec3 T;
	ComputeBasisVectors(N, S, T);

	vec3 color = vec3(0.0f);
	float weight = 0.0f;

	// Convolve environment map using GGX NDF importance sampling
	// Weight by cosine term since Epic claims it generally improves quality
	for(uint i = 0; i < Samples; i++)
	{
		vec2 u = Hammersley(i);
		vec3 Lh = TangentToWorld(ImportanceSampleGGX(u.x, u.y, u_Uniforms.Roughness), N, S, T);

		// Compute incident direction (Li) by reflecting viewing direction (Lo) around half-vector (Lh)
		vec3 Li = 2.0f * dot(Lo, Lh) * Lh - Lo;

		float cosLi = dot(N, Li);
		if(cosLi > 0.0f)
		{
			// Use Mipmap Filtered Importance Sampling to improve convergence
			// See: https://developer.nvidia.com/gpugems/GPUGems3/gpugems3_ch20.html, section 20.4

			float cosLh = max(dot(N, Lh), 0.0f);

			// GGX normal distribution function (D term) probability density function
			// Scaling by 1/4 is due to change of density in terms of Lh to Li (and since N=V, rest of the scaling factor cancels out)
			float pdf = NDFGGX(cosLh, u_Uniforms.Roughness) * 0.25f;

			// Solid angle associated with this sample
			float ws = 1.0f / (Samples * pdf);

			// Mip level to sample from
			float mipLevel = max(0.5f * log2(ws / wt) + 1.0f, 0.0f);

			color += textureLod(u_UnfilteredTexture, Li, mipLevel).rgb * cosLi;
			weight += cosLi;
		}
	}

	color /= weight;

	imageStore(o_FilteredTexture[PARAM_LEVEL], ivec3(gl_GlobalInvocationID), vec4(color, 1.0f));
}