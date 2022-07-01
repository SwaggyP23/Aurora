#shader vertex
#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;
layout(location = 3) in vec4 aColor;

out vec2 TexCoords;
out vec3 WorldPos;
out vec3 Normal;
out vec3 v_FragPos;
out vec4 v_Color;

uniform mat4 pr_matrix;
uniform mat4 vw_matrix;
uniform mat4 ml_matrix;

void main()
{
    TexCoords = aTexCoords;
    WorldPos = vec3(ml_matrix * vec4(aPos, 1.0));
    Normal = mat3(ml_matrix) * aNormal;
    v_FragPos = vec3(ml_matrix * vec4(aPos, 1.0f));
    v_Color = aColor;

    gl_Position = pr_matrix * vw_matrix * ml_matrix * vec4(aPos, 1.0);
}

#shader fragment
#version 330 core

layout(location = 0) out vec4 FragColor;

in vec4 v_Color; // Object color
in vec3 v_FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform float ambientStrength;
uniform vec4 src_color; // Light color
uniform vec3 src_pos;
uniform vec3 view_pos;

void main()
{
    FragColor = v_Color;
}