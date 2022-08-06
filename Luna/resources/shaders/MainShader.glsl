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

//layout(std140, binding = 0) uniform Camera // This is the uniform buffer and this is written in vulkan type and vulkan does not support plain uniforms
//{
//	mat4 u_ViewProjMatrix;
//};

uniform mat4 u_ViewProjMatrix; // Temp untill i learn about UBOs

struct VertexOutput
{
	vec3 Position;
	vec4 Color;
	vec3 Normals;
	vec2 TexCoords;
	float TilingFactor;
};

flat out float TexIndex; // Needs to be float and flat so that it does not get interpolated
flat out int lightCube;
flat out int v_EntityID; // Temporary untill i reach uniform buffer objects in learn opengl

layout(location = 0) out VertexOutput Output;
//layout(location = 5) out flat int v_EntityID; // 5 since VertexOutput contains 5 attributes

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

layout(location = 0) out vec4 o_Color;
layout(location = 1) out int o_EntityID;

uniform sampler2D u_Textures[32];
uniform vec3 u_ViewPosition;

struct VertexOutput
{
	vec3 Position;
	vec4 Color;
	vec3 Normals;
	vec2 TexCoords;
	float TilingFactor;
};

struct Material
{
//	sampler2D diffuse; // This is not needed here since it is in the textures array
	sampler2D specular;

	float shininess;
};

struct Light // Each light should have different properties for each of its components and is not the same across all (Page: 129)
{
	vec3 Position;
	vec3 Direction; // For directional light

	vec3 Ambient;
	vec3 Diffuse;
	vec3 Specular;

	float Constant;
	float Linear;
	float Quadratic;
};

uniform Material material;
uniform Light light[1];

flat in float TexIndex;
flat in int lightCube;
flat in int v_EntityID; // Temporary untill i reach uniform buffer objects in learn opengl

layout(location = 0) in VertexOutput Input;
//layout(location = 5) in flat int v_EntityID;

vec3 CalcPointLight(vec3 normals, vec3 viewDirection);

void main()
{
	vec4 FragColor;
	if(lightCube == 0)
	{
		// Diffuse
		vec3 norm = normalize(Input.Normals);
		// Specular
		vec3 viewDir = normalize(u_ViewPosition - Input.Position);
		vec3 Result = CalcPointLight(norm, viewDir);
	
		vec3 tempColor = vec3(Input.Color.rgb) * Result;
		FragColor = vec4(vec3(texture(u_Textures[int(TexIndex)], Input.TexCoords * Input.TilingFactor)) * tempColor, Input.Color.w);
	}
	else
	{
		FragColor = Input.Color;// This is for light source cubes
	}
	
//	FragColor = vec4(0.1f, 0.1f, 0.1f, 1.0f);
	o_Color = FragColor;
	//o_Color = Input.Color;
	o_EntityID = v_EntityID;
}

vec3 CalcPointLight(vec3 normals, vec3 viewDirection)
{
	vec3 Total = vec3(0.0f);
	vec3 ambient = vec3(0.0f), diffuse = vec3(0.0f), specular = vec3(0.0f);
	vec3 lightDirection = vec3(0.0f), reflectionDir = vec3(0.0f); 
	float Distance, attenuation, diffuseImpact, spec;

	for(int i = 0; i < 1; i++)
	{
		Distance = length(light[i].Position - Input.Position);
		attenuation = 1.0f / (light[i].Constant + light[i].Linear * Distance + light[i].Quadratic * (Distance * Distance));

		// Ambient
		ambient = light[i].Ambient * texture(u_Textures[int(TexIndex)], Input.TexCoords).rgb;
		ambient *= attenuation / attenuation;
		Total += ambient;

		// Diffuse
		// lightDirection = normalize(light[i].Position - Input.Position); // This is for if i dont have directional lighting
		lightDirection = normalize(light[0].Direction); // This is for directional lighting
		diffuseImpact = max(dot(normals, lightDirection), 0.0f);
		diffuse = light[i].Diffuse * diffuseImpact * texture(u_Textures[int(TexIndex)], Input.TexCoords).rgb;
		diffuse *= attenuation / attenuation;
		Total += diffuse;

		// Specular
//		reflectionDir = reflect(-lightDirection, normals);
//		spec = pow(max(dot(reflectionDir, viewDirection), 0.0f), material.shininess);
//		specular = light[i].Specular * spec * texture(u_Textures[int(TexIndex)], Input.TexCoords).rgb;
//		specular *= attenuation;
//		Total += specular;
	}

	return Total;
}