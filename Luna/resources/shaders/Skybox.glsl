#pragma vertex
#version 450 core
layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec2 a_TexCoords;

layout(location = 1) out vec3 v_TexCoords; // was 0

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

layout(location = 1) in vec3 v_TexCoords; // was 0

layout(binding = 0) uniform samplerCube skybox;

layout(push_constant) uniform Mats
{
    /*layout(offset = 0) */ vec4 a;
    /*layout(offset = 16)*/ mat4 b;
    /*layout(offset = 80)*/ float c;
    /*layout(offset = 84)*/ float d;
} u_MatsUniforms;

void main()
{    
    o_Color = texture(skybox, v_TexCoords);// * (u_MatsUniforms.c + u_MatsUniforms.d);
    o_EntityID = -1;
}
