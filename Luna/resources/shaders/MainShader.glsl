#pragma vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;
layout(location = 2) in vec3 a_Normals;
layout(location = 3) in vec2 a_TexCoords;
layout(location = 4) in float a_TexIndex;
layout(location = 5) in float a_TilingFactor;
layout(location = 6) in int a_Light;

uniform mat4 u_ViewProjmatrix;

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

layout(location = 0) out VertexOutput Output;

void main()
{
	Output.Position = a_Position; // Since it is multiplied with model matrix on the cpu side
	Output.Color = a_Color;
	Output.Normals = a_Normals;
	Output.TexCoords = a_TexCoords;
	TexIndex = a_TexIndex;
	Output.TilingFactor = a_TilingFactor;
	lightCube = a_Light;

	gl_Position = u_ViewProjmatrix * vec4(a_Position, 1.0f);
}

#pragma fragment
#version 450 core

layout(location = 0) out vec4 o_Color;

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
uniform Light light[2];

flat in float TexIndex;
flat in int lightCube;

layout(location = 0) in VertexOutput Input;

vec3 CalcPointLight(vec3 normals, vec3 viewDirection);

void main()
{
	vec4 FragColor;
	if(lightCube == 0){
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

	o_Color = FragColor;
}

vec3 CalcPointLight(vec3 normals, vec3 viewDirection)
{
	vec3 Total = vec3(0.0f);
	vec3 ambient = vec3(0.0f), diffuse = vec3(0.0f), specular = vec3(0.0f);
	vec3 lightDirection = vec3(0.0f), reflectionDir = vec3(0.0f); 
	float Distance, attenuation, diffuseImpact, spec;

	for(int i = 0; i < 2; i++)
	{
		Distance = length(light[i].Position - Input.Position);
		attenuation = 1.0f / (light[i].Constant + light[i].Linear * Distance + light[i].Quadratic * (Distance * Distance));

		// Ambient
		ambient = light[i].Ambient * texture(u_Textures[int(TexIndex)], Input.TexCoords).rgb;
		ambient *= attenuation;
		Total += ambient;

		// Diffuse
		lightDirection = normalize(light[i].Position - Input.Position);
		diffuseImpact = max(dot(normals, lightDirection), 0.0f);
		diffuse = light[i].Diffuse * diffuseImpact * texture(u_Textures[int(TexIndex)], Input.TexCoords).rgb;
		diffuse *= attenuation;
		Total += diffuse;

		// Specular
		reflectionDir = reflect(-lightDirection, normals);
		spec = pow(max(dot(reflectionDir, viewDirection), 0.0f), material.shininess);
		specular = light[i].Specular * spec * texture(u_Textures[int(TexIndex)], Input.TexCoords).rgb;
		specular *= attenuation;
		Total += specular;
	}

	return Total;
}