// Aurora Engine PBR Shader
// References: LearnOpenGL: https://learnopengl.com/
// References: Michal Siejak's PBR project: https://github.com/Nadrin
// References: Sparky Engine: https://github.com/TheCherno/Sparky
// References: Hazel Engine: https://hazelengine.com/

#pragma vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec3 a_Tangent;
layout(location = 3) in vec3 a_Binormal;
layout(location = 4) in vec2 a_TexCoord;

layout(std140, binding = 0) uniform Camera
{
	mat4 ViewProjectionMatrix;
	mat4 InverseViewProjectionMatrix;
	mat4 ProjectionMatrix;
	mat4 InverseProjectionMatrix;
	mat4 ViewMatrix;
	mat4 InverseViewMatrix;
	float NearClip;
	float FarClip;
	float FOV; // Radians
	float Padding0;
} u_Camera;

struct VertexOutput
{
	vec3 WorldPosition;
	vec3 Normal;
	vec2 TexCoord;
	mat3 WorldNormals;
	mat3 WorldTransform;
	vec3 Binormal;

	mat3 CameraView;

	vec4 ViewPosition;
};

layout(location = 0) out VertexOutput Output;

layout(push_constant) uniform Transform
{
    mat4 Transform;
} u_Renderer;

invariant gl_Position;

void main()
{
	vec4 worldPos = u_Renderer.Transform * vec4(a_Position, 1.0f);
	Output.WorldPosition = worldPos.xyz;
	Output.Normal = mat3(u_Renderer.Transform) * a_Normal;
    Output.TexCoord = a_TexCoord; // Output.TexCoord = vec2(a_TexCoord.x, 1.0f - a_TexCoord.y) ?? Maybe need to invert the Y?
	Output.WorldNormals = mat3(u_Renderer.Transform) * mat3(a_Tangent, a_Binormal, a_Normal);
	Output.WorldTransform = mat3(u_Renderer.Transform);
	Output.Binormal = a_Binormal;

	Output.CameraView = mat3(u_Camera.ViewMatrix);
	Output.ViewPosition = u_Camera.ViewMatrix * worldPos;

	gl_Position = u_Camera.ViewProjectionMatrix * worldPos;
}    

#pragma fragment
#version 450 core

layout(early_fragment_tests) in;

layout(location = 0) out vec4 o_Color;
layout(location = 1) out vec4 o_ViewNormalsLuminance;
layout(location = 2) out vec4 o_MetalnessRoughness;

// PBR Texture inputs...
layout(binding = 0) uniform sampler2D u_AlbedoTexture;
layout(binding = 1) uniform sampler2D u_NormalTexture;
layout(binding = 2) uniform sampler2D u_MetalnessTexture;
layout(binding = 3) uniform sampler2D u_RoughnessTexture;

// Environment Map inputs...
layout(binding = 4) uniform samplerCube u_EnvRadianceTexture;
layout(binding = 5) uniform samplerCube u_EnvIrradianceTexture;

// BRDF Lut
layout(binding = 6) uniform sampler2D u_BRDFLutTexture;

layout(std140, binding = 0) uniform Camera
{
	mat4 ViewProjectionMatrix;
	mat4 InverseViewProjectionMatrix;
	mat4 ProjectionMatrix;
	mat4 InverseProjectionMatrix;
	mat4 ViewMatrix;
	mat4 InverseViewMatrix;
	float NearClip;
	float FarClip;
	float FOV; // Radians
	float Padding0;
} u_Camera;

layout(std140, binding = 1) uniform ScreenData
{
	vec2 InvFullResolution;
    vec2 FullResolution;
	vec2 InvHalfResolution;
    vec2 HalfResolution;
} u_ScreenData;

struct DirectionalLight
{
	vec3 Direction;
	vec3 Radiance;
	float Multiplier;
};

layout(std140, binding = 2) uniform SceneData
{
		DirectionalLight DirectionalLights;
		vec3 CameraPosition; // Offset = 32
		float EnvironmentMapIntensity;
} u_Scene;

struct VertexOutput
{
	vec3 WorldPosition;
	vec3 Normal;
	vec2 TexCoord;
	mat3 WorldNormals;
	mat3 WorldTransform;
	vec3 Binormal;

	mat3 CameraView;

	vec4 ViewPosition;
};

layout(location = 0) in VertexOutput Input;

layout(push_constant) uniform Materials
{
    vec3 AlbedoColor;
    float Metalness;
    float Roughness;
    // float Emission;

    bool UseNormalMap;
} u_MaterialUniforms;

struct PBRParameters
{
    vec3 Albedo;
    float Roughness;
    float Metalness;

    vec3 Normal;
    vec3 View;
    float NdotV;

} g_Params;

const float PI = 3.14159265358979323846f;
const float TwoPI = 2 * PI;
const float Epsilon = 0.00001f;
// Constant normal incidence Fresnel factor for all dielectrics.
const vec3 Fdielectric = vec3(0.04f);

// ----------------------------------------------------------------------------
vec3 RotateVectorAboutY(float angle, vec3 vec)
{
	angle = radians(angle);
	mat3 rotationMatrix = { vec3(cos(angle),  0.0f,   sin(angle)),
							vec3(0.0f,        1.0f,   0.0f),
							vec3(-sin(angle), 0.0f,   cos(angle)) };
	return rotationMatrix * vec;
}

// ----------------------------------------------------------------------------
// GGX/Towbridge-Reitz normal distribution function.
// Uses Disney's reparametrization of alpha = roughness^2.
float NDFGGX(float cosLh, float roughness)
{
	float alpha   = roughness * roughness;
	float alphaSq = alpha * alpha;

	float denom = (cosLh * cosLh) * (alphaSq - 1.0f) + 1.0f;

	return alphaSq / (PI * denom * denom);
}

// ----------------------------------------------------------------------------
// Single term for separable Schlick-GGX below.
float GaSchlickG1(float cosTheta, float k)
{
	return cosTheta / (cosTheta * (1.0f - k) + k);
}

// ----------------------------------------------------------------------------
// Schlick-GGX approximation of geometric attenuation function using Smith's method.
float GaSchlickGGX(float cosLi, float NdotV, float roughness)
{
	float r = roughness + 1.0f;
	float k = (r * r) / 8.0f; // Epic suggests using this roughness remapping for analytic lights.
	return GaSchlickG1(cosLi, k) * GaSchlickG1(NdotV, k);
}

// ----------------------------------------------------------------------------
float GeometrySchlickGGX(float NdotV, float roughness)
{
	float r = (roughness + 1.0f);
	float k = (r * r) / 8.0f;

	float nom = NdotV;
	float denom = NdotV * (1.0f - k) + k;

	return nom / denom;
}

// ----------------------------------------------------------------------------
// Shlick's approximation of the Fresnel factor.
vec3 FresnelSchlick(vec3 F0, float cosTheta)
{
	return F0 + (vec3(1.0f) - F0) * pow(1.0f - cosTheta, 5.0f);
}

// ----------------------------------------------------------------------------
vec3 FresnelSchlickRoughness(vec3 F0, float cosTheta, float roughness)
{
	return F0 + (max(vec3(1.0f - roughness), F0) - F0) * pow(1.0f - cosTheta, 5.0f);
}

vec3 CalculateDirectionalLights(vec3 F0)
{
	vec3 result = vec3(0.0f);
	for(int i = 0; i < 1; i++) // Only one dir light for now...
	{
		if(u_Scene.DirectionalLights.Multiplier == 0.0f)
			continue; // If multiplier is 0 basically there is no light

		vec3 Li = u_Scene.DirectionalLights.Direction;
		vec3 Lradiance = u_Scene.DirectionalLights.Radiance;// * u_Scene.DirectionalLights.Multiplier;

		// Half-vector between Li and Lo.
		vec3 Lh = normalize(Li + g_Params.View);

		// Calculate angles between surface normal and various light vectors.
		float cosLi = max(0.0f, dot(g_Params.Normal, Li));
		float cosLh = max(0.0f, dot(g_Params.Normal, Lh));

		// Calculate Fresnel term for direct lighting. 
		vec3 F = FresnelSchlickRoughness(F0, max(0.0f, dot(Lh, g_Params.View)), g_Params.Roughness);
		// vec3 F = FresnelSchlick(F0, max(0.0f, dot(Lh, g_Params.View)));

		// Calculate normal distribution for specular BRDF.
		float D = NDFGGX(cosLh, g_Params.Roughness);

		// Calculate geometric attenuation for specular BRDF.
		float G = GaSchlickGGX(cosLi, g_Params.NdotV, g_Params.Roughness);

		// Diffuse scattering happens due to light being refracted multiple times by a dielectric medium.
		// Metals on the other hand either reflect or absorb energy, so diffuse contribution is always zero.
		// To be energy conserving we must scale diffuse BRDF contribution based on Fresnel factor & metalness.
		vec3 kd = (1.0f - F) * (1.0f - g_Params.Metalness);
		// vec3 kd = mix(vec3(1.0f) - F, vec3(0.0f), g_Params.Metalness);

		// Lambert diffuse BRDF.
		// We don't scale by 1/PI for lighting & material units to be more convenient.
		// See: https://seblagarde.wordpress.com/2012/01/08/pi-or-not-to-pi-in-game-lighting-equation/
		vec3 diffuseBRDF = kd * g_Params.Albedo;

		// Cook-Torrance specular microfacet BRDF.
		vec3 specularBRDF = (F * D * G) / max(Epsilon, 4.0f * cosLi * g_Params.NdotV);
		specularBRDF = clamp(specularBRDF, vec3(0.0f), vec3(10.0f));
		
		// Total contribution for this light.
		result += (diffuseBRDF + specularBRDF) * Lradiance * cosLi;
	}

	return result;
}

vec3 IBL(vec3 F0, vec3 Lr)
{
	// Sample diffuse irradiance at normal direction.
	vec3 irradiance = texture(u_EnvIrradianceTexture, g_Params.Normal).rgb;

	// Calculate Fresnel term for ambient lighting.
	// Since we use pre-filtered cubemap(s) and irradiance is coming from many directions
	// use cosLo instead of angle with light's half-vector (cosLh above).
	// See: https://seblagarde.wordpress.com/2011/08/17/hello-world/
	vec3 F = FresnelSchlickRoughness(F0, g_Params.NdotV, g_Params.Roughness);
	// vec3 F = FresnelSchlick(F0, g_Params.NdotV);

	// Get diffuse contribution factor (as with direct lighting).
	vec3 kd = (1.0f - F) * (1.0f - g_Params.Metalness);
	// vec3 kd = mix(vec3(1.0f) - F, vec3(0.0f), g_Params.Metalness);

	// Irradiance map contains exitant radiance assuming Lambertian BRDF, no need to scale by 1/PI here either.
	vec3 diffuseIBL = kd * g_Params.Albedo * irradiance;

	// Sample pre-filtered specular reflection environment at correct mipmap level.
	int envRadianceTextureLevels = textureQueryLevels(u_EnvRadianceTexture);
	vec3 specularIrradiance = textureLod(u_EnvRadianceTexture, RotateVectorAboutY(0.0f, Lr), g_Params.Roughness * envRadianceTextureLevels).rgb;
	// vec3 specularIrradiance = textureLod(u_EnvRadianceTexture, Lr, g_Params.Roughness * envRadianceTextureLevels).rgb;

	// Split-sum approximation factors for Cook-Torrance specular BRDF. May need to invert the Roughness on the Y
	vec2 specularBRDF = texture(u_BRDFLutTexture, vec2(g_Params.NdotV, g_Params.Roughness)).rg;

	// Total specular IBL contribution.
	vec3 specularIBL = specularIrradiance * (F0 * specularBRDF.x + specularBRDF.y);
	// vec3 specularIBL = (F0 * specularBRDF.x + specularBRDF.y) * specularIrradiance;

	return kd * diffuseIBL + specularIBL;
	// return diffuseIBL + specularIBL;
}

void main()
{
	// Albedo
    g_Params.Albedo = texture(u_AlbedoTexture, Input.TexCoord).rgb * u_MaterialUniforms.AlbedoColor.rgb;
	// Metalness
    g_Params.Metalness = texture(u_MetalnessTexture, Input.TexCoord).r * u_MaterialUniforms.Metalness;
	// Roughness
    g_Params.Roughness = texture(u_RoughnessTexture, Input.TexCoord).r * u_MaterialUniforms.Roughness;
    g_Params.Roughness = max(g_Params.Roughness, 0.05f);
	o_MetalnessRoughness = vec4(g_Params.Metalness, g_Params.Roughness, 0.0f, 1.0f);

	// Normals (either from vertex or map)
	g_Params.Normal = normalize(Input.Normal);
	if(u_MaterialUniforms.UseNormalMap)
	{
		g_Params.Normal = normalize(texture(u_NormalTexture, Input.TexCoord).rgb * 2.0f - 1.0f);
		g_Params.Normal = normalize(Input.WorldNormals * g_Params.Normal);
	}

	// View normals
	o_ViewNormalsLuminance = vec4(Input.CameraView * g_Params.Normal, 1.0f);

	g_Params.View = normalize(u_Scene.CameraPosition - Input.WorldPosition);
	g_Params.NdotV = max(dot(g_Params.Normal, g_Params.View), 0.0f);

	// Specular reflection vector.
	vec3 Lr = 2.0f * g_Params.NdotV * g_Params.Normal - g_Params.View;

	// Fresnel reflectance, metals use albedo
	vec3 F0 = mix(Fdielectric, g_Params.Albedo, g_Params.Metalness);

	// Direct lighting calculations...
	vec3 LightContribution = CalculateDirectionalLights(F0);
//	LightContribution += g_Params.Albedo * u_MaterialUniforms.Emission; // TODO: Support for emissive materials

	// IBL lighting calculations...
	vec3 IBLContribution = IBL(F0, Lr);// * u_Scene.EnvironmentMapIntensity;

	// Final Color
	o_Color = vec4(IBLContribution + LightContribution, 1.0f);
}

//void main()
//{    
//    g_Params.Albedo = texture(u_AlbedoTexture, Input.TexCoord).rgb * u_MaterialUniforms.AlbedoColor.rgb;
////    g_Params.Metalness = texture(u_MetalnessTexture, Input.TexCoord).r * u_MaterialUniforms.Metalness;
////    g_Params.Roughness = texture(u_RoughnessTexture, Input.TexCoord).r * u_MaterialUniforms.Roughness;
////    g_Params.Roughness = max(g_Params.Roughness, 0.05f);
//
//    o_Color = vec4(g_Params.Albedo, 1.0f);
//    o_ViewNormalsLuminance = vec4(0.0f, 1.0f, 0.0f, 1.0f);
//    o_MetalnessRoughness = vec4(0.0f, 0.0f, 1.0f, 1.0f);
//}