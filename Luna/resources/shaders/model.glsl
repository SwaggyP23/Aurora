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
out vec3 normals;

void main()
{
    v_EntityID = u_EntityID;
    TexCoords = a_TexCoords;    
    normals = a_Normals;
    FragPos = vec3(model * vec4(a_Pos, 1.0f));
    gl_Position = viewproj * model * vec4(a_Pos, 1.0f);
}


#pragma fragment
#version 450 core
layout(location = 0) out vec4 FragColor;
layout(location = 1) out int o_EntityID;

flat in int v_EntityID;
in vec2 TexCoords;
in vec3 normals;
in vec3 FragPos;

uniform sampler2D texture_diffuse1;
uniform vec3 lightPos;

float ambientIntensity = 0.5f;

void main()
{    
//    vec3 norm = normalize(normals);
//    vec3 lightColor = vec3(1.0f);

//    vec4 ambient = vec4(lightColor, 1.0f) * ambientIntensity;

//    vec3 lightDir = normalize(lightPos - FragPos);
//    float diff = max(dot(norm,  lightDir), 0.0f);
//    vec3 diffuse = diff * lightColor;

    FragColor = /*(ambient + vec4(diffuse, 1.0f)) **/ texture(texture_diffuse1, TexCoords);

    o_EntityID = v_EntityID;
}