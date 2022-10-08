#pragma vertex
#version 450 core

// Reference: https://www.geeks3d.com/hacklab/20180611/demo-simple-2d-grid-in-glsl/

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoords;

layout(std140, binding = 0) uniform Camera
{
	mat4 ViewProjectionMatrix;
	mat4 InverseViewProjectionMatrix;
	mat4 ProjectionMatrix;
	mat4 InverseProjectionMatrix;
	mat4 ViewMatrix;
	mat4 InverseViewMatrix;
	float NearClip;
	float FarClip;
	float FOV; // Radians
	float Padding0;
} u_Camera;

layout(push_constant) uniform Transform
{
	mat4 Transform;
} u_Renderer;

layout(location = 0) out vec2 v_TexCoords;

void main()
{
	v_TexCoords = a_TexCoords;

	gl_Position = u_Camera.ViewProjectionMatrix * u_Renderer.Transform * vec4(a_Position, 1.0f);
}

#pragma fragment
#version 450 core

layout(location = 0) out vec4 o_Color;

layout(location = 0) in vec2 v_TexCoords;

layout(push_constant) uniform Settings
{
	float Scale;
	float Size;
} u_Settings;

float grid(vec2 uv, float res)
{
	vec2 grid = fract(uv);
	return step(res, grid.x) * step(res, grid.y);
}

void main()
{
	float x = grid(v_TexCoords * u_Settings.Scale, u_Settings.Size);
	
	o_Color = vec4(vec3(0.2f), 0.5f) * (1.0f - x);

	// Avoid Depth write
	if(o_Color.a == 0.0f)
		discard;
}