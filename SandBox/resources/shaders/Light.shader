#shader vertex
#version 330 core

layout(location = 0) in vec3 a_Position; // since this is an attribute, takes the a_ Prefix.
layout(location = 1) in vec4 a_Color;
layout(location = 2) in vec2 a_TexCoord;

//out vec4 v_Color;
//out vec2 v_TexCoord;

uniform mat4 ml_matrix = mat4(1.0f);
uniform mat4 vw_matrix = mat4(1.0f);
uniform mat4 pr_matrix = mat4(1.0f);
//uniform vec4 un_color = vec4(1.0f);

void main()
{
	gl_Position = pr_matrix * vw_matrix * ml_matrix * vec4(a_Position, 1.0f);
	//v_Color = a_Color * un_color; // since now with the current vertex buffer, no colors are specified
	//v_TexCoord = a_TexCoord;
}


#shader fragment
#version 330 core

layout(location = 0) out vec4 color;

//in vec4 v_Color;
//in vec2 v_TexCoord;

//uniform float blend;
uniform float ambientStrength;
uniform vec4 lightColor;
//uniform sampler2D texture1;
//uniform sampler2D texture2;

void main()
{
	color = lightColor * ambientStrength;
}