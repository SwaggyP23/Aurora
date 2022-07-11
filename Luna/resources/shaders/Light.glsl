#pragma vertex
#version 450 core

layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec4 a_Color;
//layout (location = 2) in vec2 a_TexCoord;

out vec4 v_Color;
//out vec2 v_TexCoord;

uniform mat4 vw_pr_matrix;

void main()
{
	gl_Position = vw_pr_matrix * vec4(a_Position, 1.0f);
	v_Color = a_Color;
	//v_TexCoord = a_TexCoord;
}


#pragma fragment
#version 450 core

layout(location = 0) out vec4 o_Color;

in vec4 v_Color;
//in vec2 v_TexCoord;

uniform float ambientStrength = 0.3f;

void main()
{
	o_Color = v_Color * (ambientStrength + 0.5);
}