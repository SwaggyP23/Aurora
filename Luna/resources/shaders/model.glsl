#pragma vertex
#version 450 core

layout (location = 0) in vec3 a_Pos;
layout (location = 1) in vec3 a_Normals;
layout (location = 2) in vec2 a_TexCoords;

layout(std140, binding = 0) uniform Camera
{
    mat4 u_ViewProjMatrix;
    mat4 u_SkyVP;
};

layout(std140, binding = 1) uniform modelInfo
{
    mat4 model;
    int u_EntityID;
};

layout(location = 0) out flat int v_EntityID;
layout(location = 1) out vec3 FragPos;
layout(location = 2) out vec2 TexCoords;

void main()
{
    v_EntityID = u_EntityID;
    TexCoords = a_TexCoords;    
    FragPos = vec3(model * vec4(a_Pos, 1.0f));
    gl_Position = u_ViewProjMatrix * model * vec4(a_Pos, 1.0f);
}


#pragma fragment
#version 450 core
layout(location = 0) out vec4 FragColor;
layout(location = 1) out int o_EntityID;

layout(location = 0) in flat int v_EntityID;
layout(location = 1) in vec3 FragPos;
layout(location = 2) in vec2 TexCoords;

layout(binding = 0) uniform sampler2D texture_diffuse1;

void main()
{
    FragColor = texture(texture_diffuse1, TexCoords);

    o_EntityID = v_EntityID;
}