#pragma compute
#version 450 core

// Pre-integrates Cook-Torrance specular BRDF for varying roughness and viewing directions.
// Results are saved into 2D LUT texture in the form of DFG1 and DFG2 split-sum approximation terms,
// which act as a scale and bias to F0 (Fresnel reflectance at normal incidence) during rendering.
// Reference: Michal Siejak's PBR project: https://github.com/Nadrin
// Reference: Joey de Vries's LearnOpenGL Book: https://learnopengl.com/

const float PI = 3.14159265358979323846f;
const float TwoPI = 2 * PI;
const float Epsilon = 0.001f; // This program needs a larger epsilon

const uint Samples = 1024;
const float InvNumSamples = 1.0f / float(Samples);

layout(binding = 0, rg16f) restrict writeonly uniform image2D u_Lut;

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
// Single term for separable Schlick-GGX below.
float GaSchlickG1(float cosTheta, float k)
{
	return cosTheta / (cosTheta * (1.0f - k) + k);
}

// ----------------------------------------------------------------------------
// Schlick-GGX approximation of geometric attenuation function using Smith's method (IBL version).
float GaSchlickGGX_IBL(float cosLi, float cosLo, float roughness)
{
	float k = (roughness * roughness) / 2.0f; // Epic suggests using this roughness remapping for IBL lighting.
	return GaSchlickG1(cosLi, k) * GaSchlickG1(cosLo, k);
}

layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;

void main()
{
	// Get integration parameters.
	float cosLo = gl_GlobalInvocationID.x / float(imageSize(u_Lut).x);
	float roughness = gl_GlobalInvocationID.y / float(imageSize(u_Lut).y);

	// Make sure viewing angle is non-zero to avoid divisions by zero (and subsequently NaNs).
	cosLo = max(cosLo, Epsilon);

	// Derive tangent-space viewing vector from angle to normal (pointing towards +Z in this reference frame).
	vec3 Lo = vec3(sqrt(1.0f - cosLo * cosLo), 0.0f, cosLo);

	// We will now pre-integrate Cook-Torrance BRDF for a solid white environment and save results into a 2D LUT.
	// DFG1 & DFG2 are terms of split-sum approximation of the reflectance integral.
	// For derivation see: "Moving Frostbite to Physically Based Rendering 3.0", SIGGRAPH 2014, section 4.9.2.
	float DFG1 = 0.0f;
	float DFG2 = 0.0f;

	for(uint i = 0; i < Samples; i++) 
	{
		vec2 u  = Hammersley(i);
		
		// Sample directly in tangent/shading space since we don't care about reference frame as long as it's consistent.
		vec3 Lh = ImportanceSampleGGX(u.x, u.y, roughness);

		// Compute incident direction (Li) by reflecting viewing direction (Lo) around half-vector (Lh).
		vec3 Li = 2.0f * dot(Lo, Lh) * Lh - Lo;

		float cosLi   = Li.z;
		float cosLh   = Lh.z;
		float cosLoLh = max(dot(Lo, Lh), 0.0f);

		if(cosLi > 0.0f)
		{
			float G  = GaSchlickGGX_IBL(cosLi, cosLo, roughness);
			float Gv = G * cosLoLh / (cosLh * cosLo);
			float Fc = pow(1.0f - cosLoLh, 5);

			DFG1 += (1 - Fc) * Gv;
			DFG2 += Fc * Gv;
		}
	}

	imageStore(u_Lut, ivec2(gl_GlobalInvocationID), vec4(DFG1, DFG2, 0.0f, 0.0f) * InvNumSamples);
}