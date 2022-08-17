#pragma vertex
#version 450 core

layout (location = 0) in vec3 a_Pos;
layout (location = 1) in vec3 a_Normals;
layout (location = 2) in vec2 a_TexCoords;

out vec3 FragPos;
out vec2 TexCoords;

uniform mat4 model;
uniform mat4 viewproj;
uniform int u_EntityID;

flat out int v_EntityID;

void main()
{
    v_EntityID = u_EntityID;
    TexCoords = a_TexCoords;    
    FragPos = vec3(model * vec4(a_Pos, 1.0f));
    gl_Position = viewproj * model * vec4(a_Pos, 1.0f);
}


#pragma fragment
#version 450 core
layout(location = 0) out vec4 FragColor;
layout(location = 1) out int o_EntityID;

flat in int v_EntityID;
in vec2 TexCoords;
in vec3 FragPos;

uniform sampler2D texture_diffuse1;

void main()
{
    FragColor = texture(texture_diffuse1, TexCoords);

    o_EntityID = v_EntityID;
}