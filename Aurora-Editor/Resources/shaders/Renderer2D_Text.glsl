#pragma vertex
#version 450 core

layout(location = 0) in vec3 a_Position; // Already in world space
layout(location = 1) in vec4 a_Color;
//layout(location = 2) in vec4 a_OutLineColor;
layout(location = 2) in vec2 a_TexCoord;
layout(location = 3) in float a_TexIndex;
//layout(location = 5) in float a_OutLineWidth;

layout(std140, binding = 3) uniform Camera
{
	mat4 u_ViewProjMatrix;
};	

struct VertexOutput
{
	vec4 Color;
//	vec4 OutLineColor;
	vec2 TexCoord;
//	float OutLineWidth;
};

layout(location = 0) out VertexOutput Output;
layout(location = 3) out flat float TexIndex;

invariant gl_Position;

void main()
{
	Output.Color = a_Color;
	Output.TexCoord = a_TexCoord;
//	Output.OutLineColor = a_OutLineColor;
//	Output.OutLineWidth = a_OutLineWidth;
	TexIndex = a_TexIndex;

	gl_Position = u_ViewProjMatrix * vec4(a_Position, 1.0f);
}

#pragma fragment
#version 450 core

layout(location = 0) out vec4 o_Color;

struct VertexOutput
{
	vec4 Color;
//	vec4 OutLineColor;
	vec2 TexCoord;
//	float OutLineWidth;
};

layout(location = 0) in VertexOutput Input;
layout(location = 3) in flat float TexIndex;

layout(binding = 0) uniform sampler2D u_FontAtlases[16]; // Consecutive textures will take the next consecutive binding point

float median(float r, float g, float b)
{
	return max(min(r, g), min(max(r, g), b));
}

// For 2d
//float ScreenPxRange()
//{
//	float pixRange = 2.0f;
//	float geoSize = 72.0f;

//	return geoSize / 32.0f * pixRange;
//}

float ScreenPxRange()
{
	float pxRange = 2.0f;
	vec2 unitRange = vec2(pxRange) / vec2(textureSize(u_FontAtlases[int(TexIndex)], 0));
	vec2 screenTexSize = vec2(1.0f) / fwidth(Input.TexCoord);
	
	return max(0.5f * dot(unitRange, screenTexSize), 1.0f);
}

// TODO: Rework the outlines
void main()
{
	vec4 bgColor = vec4(Input.Color.rgb, 0.0f);
	vec4 fgColor = Input.Color;		
	
	vec4 textureData = texture(u_FontAtlases[int(TexIndex)], Input.TexCoord);
	vec3 msd = textureData.rgb; // This is the pure msd data

	float sd = median(msd.r, msd.g, msd.b); // This include some extra blurrinness and gives a kind of blurry outline.
	float screenPxDistance = ScreenPxRange() * (sd - 0.5f); // This is 1.0f for pixels inside the letter otherwise 0.0f
	float opacity = clamp(screenPxDistance + 0.5f, 0.0f, 1.0f);

	o_Color = mix(bgColor, fgColor, opacity);

	// A very hacky way to get outlines
//	if(sd + Input.OutLineWidth * 2.0f > screenPxDistance)
//		o_Color = vec4(Input.OutLineColor.rgb, opacity);
//	o_Color = vec4(vec3(sd), 1.0f);

	// Discard to avoid depth write
	if(o_Color.a == 0.0f)
		discard;
}