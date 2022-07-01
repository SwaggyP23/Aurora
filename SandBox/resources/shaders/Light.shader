#shader vertex
#version 330 core

layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec4 a_Color;
layout (location = 2) in vec2 a_TexCoord;

//out vec4 v_Color;
//out vec2 v_TexCoord;

uniform mat4 vw_pr_matrix;
uniform mat4 ml_matrix;
//uniform vec4 un_color = vec4(1.0f);

void main()
{
	gl_Position = vw_pr_matrix * ml_matrix * vec4(a_Position, 1.0f);
	//v_Color = a_Color * un_color; // since now with the current vertex buffer, no colors are specified
	//v_TexCoord = a_TexCoord;
}


#shader fragment
#version 330 core

layout(location = 0) out vec4 color;

//in vec4 v_Color;
//in vec2 v_TexCoord;

uniform float ambientStrength;
uniform vec4 lightColor;

void main()
{
	color = lightColor * (ambientStrength + 0.5);
}