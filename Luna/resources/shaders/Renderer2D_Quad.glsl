#pragma vertex
#version 450 core

layout(location = 0) in vec3 a_Position; // Already in world space
layout(location = 1) in vec4 a_Color;
layout(location = 2) in vec2 a_TexCoords;
layout(location = 3) in float a_TexIndex;
layout(location = 4) in float a_TilingFactor;

layout(std140, binding = 3) uniform Camera
{
	mat4 u_ViewProjMatrix;
};	

struct VertexOutput
{
	vec4 Color;
	vec2 TexCoords;
	float TilingFactor;
};

layout(location = 0) out VertexOutput Output;
layout(location = 3) out flat float TexIndex;

invariant gl_Position;

void main()
{
	Output.Color = a_Color;
	Output.TexCoords = a_TexCoords;
	TexIndex = a_TexIndex;
	Output.TilingFactor = a_TilingFactor;

	gl_Position = u_ViewProjMatrix * vec4(a_Position, 1.0f);
}

#pragma fragment
#version 450 core

layout(early_fragment_tests) in;

layout(location = 0) out vec4 o_Color;

struct VertexOutput
{
	vec4 Color;
	vec2 TexCoords;
	float TilingFactor;
};

layout(location = 0) in VertexOutput Input;
layout(location = 3) in flat float TexIndex;

layout(binding = 0) uniform sampler2D u_Textures[16]; // Consecutive textures will take the next consecutive binding point

void main()
{
	o_Color = texture(u_Textures[int(TexIndex)], Input.TexCoords * Input.TilingFactor) * Input.Color;

	// Discard to avoid depth write
	if(o_Color.a == 0.0f)
		discard;
}