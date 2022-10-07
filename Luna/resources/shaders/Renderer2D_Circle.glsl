#pragma vertex
#version 450 core

layout(location = 0) in vec3 a_WorldPosition;
layout(location = 1) in float a_Thickness;
layout(location = 2) in vec2 a_LocalPosition;
layout(location = 3) in vec4 a_Color;

layout(std140, binding = 2) uniform Camera
{
	mat4 u_ViewProjMatrix;
};

struct VertexOutput
{
	vec2 LocalPosition;
	vec4 Color;
	float Thickness;
};

layout(location = 0) out VertexOutput Output;

invariant gl_Position;

void main()
{
	Output.LocalPosition = a_LocalPosition;
	Output.Color = a_Color;
	Output.Thickness = a_Thickness;

	gl_Position = u_ViewProjMatrix * vec4(a_WorldPosition, 1.0f);
}

#pragma fragment
#version 450 core

layout(location = 0) out vec4 o_Color;

struct VertexOutput
{
	vec2 LocalPosition;
	vec4 Color;
	float Thickness;
};

layout(location = 0) in VertexOutput Input;

void main()
{
	const float fade = 0.01f;

	// Calculate distance and fill circle with white
    float distance = 1.0f - sqrt(dot(Input.LocalPosition, Input.LocalPosition));
    float circle = smoothstep(0.0f, fade, distance);
    circle *= smoothstep(Input.Thickness + fade, Input.Thickness, distance);

	if (circle == 0.0f)
		discard;

    // Set output color
    o_Color = Input.Color;
	o_Color.a *= circle;

	// Since this implementation allows for transperancy in the output i should do this to avoid depth write in case alpha == 0;
	if(o_Color.a == 0.0f)
		discard;
}