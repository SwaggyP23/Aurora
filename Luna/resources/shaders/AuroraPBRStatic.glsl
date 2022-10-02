// Aurora Engine PBR Shader
// References: LearnOpenGL: https://learnopengl.com/
// References: Michal Siejak's PBR project: https://github.com/Nadrin
// References: Sparky Engine: https://github.com/TheCherno/Sparky
// References: Hazel Engine: https://hazelengine.com/

#pragma vertex
#version 450 core
layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec2 a_TexCoords;

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
    vec2 TexCoords;
};

layout(location = 0) out VertexOutput v_Output;

layout(push_constant) uniform Transform
{
    mat4 transform;
} u_Renderer;

invariant gl_Position;

void main()
{
    mat4 model = u_Renderer.transform;
    v_Output.TexCoords = a_TexCoords;
    gl_Position = u_Camera.ViewProjectionMatrix * model * vec4(a_Position, 1.0);
}  

#pragma fragment
#version 450 core

layout(early_fragment_tests) in;

layout(location = 0) out vec4 o_Color;

// PBR Texture inputs...
layout(binding = 0) uniform sampler2D u_AlbedoTexture;
//layout(binding = 1) uniform sampler2D u_NormalTexture;
//layout(binding = 2) uniform sampler2D u_MetalnessTexture;
//layout(binding = 3) uniform sampler2D u_RoughnessTexture;

// Environment Map inputs...
//layout(binding = 4) uniform samplerCube u_EnvRadianceTexture;
//layout(binding = 5) uniform samplerCube u_EnvIrradianceTexture;

// BRDF Lut
//layout(binding = 6) uniform sampler2D u_BRDFLutTexture;

struct VertexOutput
{
    vec2 TexCoords;
};

layout(location = 0) in VertexOutput v_Input;

layout(push_constant) uniform Materials
{
    vec4 AlbedoColor;
    float Metalness;
    float Roughness;

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

void main()
{    
    g_Params.Albedo = texture(u_AlbedoTexture, v_Input.TexCoords).rgb * u_MaterialUniforms.AlbedoColor.rgb;
//    g_Params.Metalness = texture(u_MetalnessTexture, v_Input.TexCoords).r * u_MaterialUniforms.Metalness;
//    g_Params.Roughness = texture(u_RoughnessTexture, v_Input.TexCoords).r * u_MaterialUniforms.Roughness;
//    g_Params.Roughness = max(g_Params.Roughness, 0.05f); ????

//    o_Color = vec4(g_Params.Albedo, 1.0f); // TODO: When switching to deferred this is what should be used!!
    o_Color = vec4(g_Params.Albedo, u_MaterialUniforms.AlbedoColor.a);
}