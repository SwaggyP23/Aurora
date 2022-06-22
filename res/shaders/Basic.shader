#shader vertex
#version 330 core

layout (location = 0) in vec3 a_Position; // since this is an attribute, takes the a_ Prefix.
layout (location = 1) in vec4 a_color;

out vec4 v_Color;

void main()
{
	gl_Position = vec4(a_Position, 1.0f);
	v_Color = a_color;
}


#shader fragment 
#version 330 core

layout (location = 0) out vec4 a_Color;

in vec4 v_Color;

void main()
{
	a_Color = v_Color;
}