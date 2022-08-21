#pragma vertex
#version 450 core
layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec2 a_TexCoords;

layout(location = 0) out vec2 v_TexCoords;

layout(std140, binding = 0) uniform Camera
{
    mat4 u_ViewProjMatrix;
    mat4 u_SkyVP;
};

layout(push_constant) uniform Transform
{
    mat4 transform;
} u_Renderer;

void main()
{
    mat4 model = mat4(1.0f); //u_Renderer.transform;
    v_TexCoords = a_TexCoords;
    gl_Position = u_ViewProjMatrix * model * vec4(a_Position, 1.0);
}  

#pragma fragment
#version 450 core
layout(location = 0) out vec4 o_Color;
layout(location = 1) out int o_EntityID;

layout(location = 0) in vec2 v_TexCoords;

layout(push_constant) uniform Mats
{
    vec3 AlbedoColor;
} u_Uniforms;

layout(binding = 0) uniform sampler2D u_AlbedoTexture;

void main()
{    
    vec4 color = vec4(u_Uniforms.AlbedoColor, 1.0f);
    o_Color = texture(u_AlbedoTexture, v_TexCoords) * color;
    o_EntityID = -1;
}
