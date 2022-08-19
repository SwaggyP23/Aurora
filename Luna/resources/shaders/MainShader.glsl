#pragma vertex
#version 450 core

// TODO: Change whatever this shit is with all these layouts
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;
layout(location = 2) in vec3 a_Normals;
layout(location = 3) in vec2 a_TexCoords;
layout(location = 4) in float a_TexIndex;
layout(location = 5) in float a_TilingFactor;
layout(location = 6) in int a_Light;
layout(location = 7) in int a_EntityID;

layout(std140, binding = 0) uniform Camera // This is the uniform buffer and this is written in vulkan type and vulkan does not support plain uniforms
{
	mat4 u_ViewProjMatrix;
	mat4 u_SkyVP;
};

struct VertexOutput
{
	vec3 Position;
	vec4 Color;
	vec3 Normals;
	vec2 TexCoords;
	float TilingFactor;
};

layout(location = 0) out VertexOutput Output;
layout(location = 5) out flat int v_EntityID; // 5 since VertexOutput contains 5 attributes
layout(location = 6) out flat float TexIndex;
layout(location = 7) out flat int lightCube;

void main()
{
	Output.Position = a_Position; // Since it is multiplied with model matrix on the cpu side
	Output.Color = a_Color;
	Output.Normals = a_Normals;
	Output.TexCoords = a_TexCoords;
	TexIndex = a_TexIndex;
	Output.TilingFactor = a_TilingFactor;
	lightCube = a_Light;
	v_EntityID = a_EntityID;

	gl_Position = u_ViewProjMatrix * vec4(a_Position, 1.0f);
}

#pragma fragment
#version 450 core

layout(early_fragment_tests) in;

layout(location = 0) out vec4 o_Color;
layout(location = 1) out int o_EntityID;

layout(binding = 0) uniform sampler2D u_Textures[16];

struct VertexOutput
{
	vec3 Position;
	vec4 Color;
	vec3 Normals;
	vec2 TexCoords;
	float TilingFactor;
};

layout(location = 0) in VertexOutput Input;
layout(location = 5) in flat int v_EntityID;
layout(location = 6) in flat float TexIndex;
layout(location = 7) in flat int lightCube;

void main()
{
	vec4 FragColor;
//	if(lightCube == 0)
//	{
//		vec3 tempColor = vec3(Input.Color.rgb);
//		FragColor = vec4(vec3(texture(u_Textures[int(TexIndex)], Input.TexCoords * Input.TilingFactor)) * tempColor, Input.Color.w);
//	}
//	else
//	{
//		FragColor = Input.Color;// This is for light source cubes
//	}
	
	vec3 tempColor = vec3(Input.Color.rgb);
	FragColor = vec4(vec3(texture(u_Textures[int(TexIndex)], Input.TexCoords * Input.TilingFactor)) * tempColor, Input.Color.a);
	o_Color = FragColor;
	o_EntityID = v_EntityID;
}