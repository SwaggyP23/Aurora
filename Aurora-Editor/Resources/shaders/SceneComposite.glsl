#pragma vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;

layout(location = 0) out vec2 v_TexCoord;

void main()
{
	v_TexCoord = a_TexCoord;

	gl_Position = vec4(a_Position.xy, 0.0f, 1.0f);;
}

#pragma fragment
#version 450 core

layout(location = 0) out vec4 o_Color;

layout(location = 0) in vec2 v_TexCoord;

layout(binding = 0) uniform sampler2D u_Texture;

//layout(push_constant) uniform Uniforms
//{
//	float Exposure;
//} u_Uniforms;

// Based on http://www.oscars.org/science-technology/sci-tech-projects/aces
vec3 ACESTonemap(vec3 color)
{
	mat3 m1 = mat3(
		0.59719f, 0.07600f, 0.02840f,
		0.35458f, 0.90834f, 0.13383f,
		0.04823f, 0.01566f, 0.83777f
	);
	mat3 m2 = mat3(
		1.60475f, -0.10208f, -0.00327f,
		-0.53108f, 1.10813f, -0.07276f,
		-0.07367f, -0.00605f, 1.07602f
	);
	vec3 v = m1 * color;
	vec3 a = v * (v + 0.0245786f) - 0.000090537f;
	vec3 b = v * (0.983729f * v + 0.4329510f) + 0.238081f;
	return clamp(m2 * (a / b), 0.0f, 1.0f);
}

vec3 GammaCorrect(vec3 color, float gamma)
{
	return pow(color, vec3(1.0f / gamma));
}

void main()
{
	const float gamma = 2.2f;

	vec3 color = texture(u_Texture, v_TexCoord).rgb;
	
//	color *= u_Uniforms.Exposure; // TODO: Add exposure

	color = ACESTonemap(color);
	color = GammaCorrect(color, gamma);

	o_Color = vec4(color, 1.0f);
}