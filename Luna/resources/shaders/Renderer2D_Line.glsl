#pragma vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;

layout(std140, binding = 2) uniform Camera
{
	mat4 u_ViewProjMatrix;
};	

layout(location = 0) out vec4 v_Color;

invariant gl_Position;

void main()
{
	v_Color = a_Color;
	gl_Position = u_ViewProjMatrix * vec4(a_Position, 1.0f);
}

#pragma fragment
#version 450 core

layout(location = 0) out vec4 o_Color;

layout(early_fragment_tests) in;

layout(location = 0) in vec4 v_Color;

void main()
{
	o_Color = v_Color;

	// Discard to avoid depth write
	if(o_Color.a == 0.0f)
		discard;
}