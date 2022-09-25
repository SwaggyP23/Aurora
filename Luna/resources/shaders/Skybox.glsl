#pragma vertex
#version 450 core
layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec2 a_TexCoords;

layout(location = 0) out vec3 v_TexCoords;

layout(std140, binding = 0) uniform Camera
{
    mat4 u_ViewProjMatrix;
    mat4 u_SkyVP;
};

void main()
{
    v_TexCoords = a_Position;
    vec4 pos = u_SkyVP * vec4(a_Position, 1.0f);
    gl_Position = pos.xyww;
}  

#pragma fragment
#version 450 core
layout(location = 0) out vec4 o_Color;
layout(location = 1) out int o_EntityID;

layout(location = 0) in vec3 v_TexCoords;

layout(binding = 0) uniform samplerCube u_EnvFiltered;

layout(push_constant) uniform Uniforms
{
    float TextureLOD;
    float Intensity;
} u_Uniforms;

void main()
{    
    vec3 envVector = normalize(v_TexCoords);
    o_Color = textureLod(u_EnvFiltered, envVector, u_Uniforms.TextureLOD) * u_Uniforms.Intensity;
    o_EntityID = -1;
}
