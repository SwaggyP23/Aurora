#pragma vertex
#version 450 core
layout (location = 0) in vec3 a_Position;

layout(location = 0) out vec3 v_TexCoords;

layout(std140, binding = 0) uniform Camera
{
    mat4 u_ViewProjMatrix;
    mat4 u_SkyVP;
};

void main()
{
    v_TexCoords = a_Position;
    vec4 pos = u_SkyVP * vec4(a_Position, 1.0);
    gl_Position = pos.xyww;
}  

#pragma fragment
#version 450 core
layout(location = 0) out vec4 o_Color;
layout(location = 1) out int o_EntityID;

layout(location = 0) in vec3 v_TexCoords;

layout(binding = 0) uniform samplerCube skybox;

layout(push_constant) uniform Mats
{
    float a;
    float b;
    vec2 c;
    mat4 d;
} u_MatsUniforms;

void main()
{    
    o_Color = texture(skybox, v_TexCoords);
//    o_Color = vec4(0.8f, 0.2f, 0.3f, 1.0f);
    o_EntityID = -1;
}
