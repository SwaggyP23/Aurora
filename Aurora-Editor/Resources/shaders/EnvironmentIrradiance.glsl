#pragma compute
#version 450 core

// Computes diffuse irradiance cubemap convolution for image-based lighting
// Uses quasi Monte Carlo sampling with Hammersley sequence
// Reference: Michal Siejak's PBR project: https://github.com/Nadrin
// Reference: Joey de Vries's LearnOpenGL Book: https://learnopengl.com/

const float PI = 3.14159265358979323846f;
const float TwoPI = 2 * PI;
const float Epsilon = 0.00001f;

layout(binding = 0) uniform samplerCube u_RadianceMap;
layout(binding = 0, rgba32f) restrict writeonly uniform imageCube o_IrradianceMap;

layout(push_constant) uniform Uniforms
{
	uint Samples;
} u_Uniforms;

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
// Sample i-th point from Hammersley point set of NumSamples points total.
vec2 SampleHammersley(uint i, uint samples)
{
	float invSamples = 1.0f / float(samples);

	return vec2(i * invSamples, RadicalInverse_VdC(i));
}

// ----------------------------------------------------------------------------
// Uniformly sample point on a hemisphere.
// Cosine-weighted sampling would be a better fit for Lambertian BRDF but since this
// compute shader runs only once as a pre-processing step performance is not *that* important.
// See: "Physically Based Rendering" 2nd ed., section 13.6.1.
vec3 SampleHemisphere(float u1, float u2)
{
	const float u1p = sqrt(max(0.0f, 1.0f - u1 * u1));
	return vec3(cos(TwoPI * u2) * u1p, sin(TwoPI * u2) * u1p, u1);
}

// ----------------------------------------------------------------------------
// Read EquiRectangularToCubeMap shader for more info on this!
vec3 GetCubeMapTexCoord(vec2 outputImageSize)
{
    vec2 st = gl_GlobalInvocationID.xy / outputImageSize;
    vec2 uv = 2.0f * vec2(st.x, 1.0f - st.y) - vec2(1.0f);

    vec3 ret;

    if (gl_GlobalInvocationID.z == 0u)      ret = vec3(  1.0f,  uv.y,  -uv.x);
    else if (gl_GlobalInvocationID.z == 1u) ret = vec3( -1.0f,  uv.y,   uv.x);
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
	vec3 N = GetCubeMapTexCoord(vec2(imageSize(o_IrradianceMap)));

	vec3 S;
	vec3 T;
	ComputeBasisVectors(N, S, T);

	const uint Samples = 64 * u_Uniforms.Samples;

	// Monte Carlo integration of hemispherical irradiance.
	// As a small optimization this also includes Lambertian BRDF assuming perfectly white surface (albedo of 1.0)
	// so we don't need to normalize in PBR fragment shader (so technically it encodes exitant radiance rather than irradiance).
	vec3 irradiance = vec3(0.0f);
	for(uint i = 0; i < Samples; i++)
	{
		vec2 u = SampleHammersley(i, Samples);
		vec3 Li = TangentToWorld(SampleHemisphere(u.x, u.y), N, S, T);
		float cosTheta = max(0.0f, dot(Li, N));

		// PIs here cancel out because of division by pdf.
		irradiance += 2.0f * textureLod(u_RadianceMap, Li, 0).rgb * cosTheta;
	}

	irradiance /= vec3(Samples);

	imageStore(o_IrradianceMap, ivec3(gl_GlobalInvocationID), vec4(irradiance, 1.0f));
}