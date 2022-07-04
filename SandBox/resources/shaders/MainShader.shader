#shader vertex
#version 330 core

layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec4 a_Color;

out vec4 v_Color;

uniform mat4 vw_pr_matrix;
uniform mat4 ml_matrix;

void main()
{
	v_Color = a_Color;
	gl_Position = vw_pr_matrix * ml_matrix * vec4(a_Position, 1.0f);
}

#shader fragment
#version 330 core

layout (location = 0) out vec4 FragColor;

in vec4 v_Color;

uniform vec4 u_CubeColor;

void main()
{
	FragColor = u_CubeColor;
}