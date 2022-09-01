#pragma vertex
#version 450 core
layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec2 a_TexCoords;

layout(location = 1) out vec2 v_TexCoords; // was 0

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
    mat4 model = u_Renderer.transform;
//    mat4 model = mat4(1.0f);
    v_TexCoords = a_TexCoords;
    gl_Position = u_ViewProjMatrix * model * vec4(a_Position, 1.0);
}  

#pragma fragment
#version 450 core
layout(location = 0) out vec4 o_Color;
layout(location = 1) out int o_EntityID;

layout(location = 1) in vec2 v_TexCoords; // was 0

layout(binding = 0) uniform sampler2D u_AlbedoTexture;

layout(push_constant) uniform Mats
{
    vec4 AlbedoColor;
} u_Uniforms;

void main()
{    
    vec4 color = u_Uniforms.AlbedoColor;
    o_Color = texture(u_AlbedoTexture, v_TexCoords);// * color;
    o_EntityID = -1;
}