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
uniform Light light;

flat in float TexIndex;
flat in int lightCube;

layout(location = 0) in VertexOutput Input;

void main()
{
	vec4 FragColor;
	if(lightCube == 0){
		float Distance = length(light.Position - Input.Position);
		float attenuation = 1.0f / (light.Constant + light.Linear * Distance + light.Quadratic * (Distance * Distance));

		// ambient
		vec3 ambientLight = light.Ambient * texture(u_Textures[int(TexIndex)], Input.TexCoords).rgb; // Since ambient and diffuse should be fairly the same (Page: 132).
		ambientLight *= attenuation;

		// Diffuse
		vec3 norm = normalize(Input.Normals);
		vec3 lightDirection = normalize(light.Position - Input.Position);
//		vec3 lightDirection = normalize(-light.Direction);
	
		float diffuseImpact = max(dot(norm, lightDirection), 0.0f);
		vec3 diffuse = light.Diffuse * diffuseImpact * texture(u_Textures[int(TexIndex)], Input.TexCoords).rgb;
		diffuse *= attenuation;
	
		// Specular
		vec3 viewDir = normalize(u_ViewPosition - Input.Position);
		vec3 reflectionDir = reflect(-lightDirection, norm);
		float spec = pow(max(dot(reflectionDir, viewDir), 0.0f), material.shininess);
		vec3 specular = light.Specular * spec * texture(u_Textures[int(TexIndex)], Input.TexCoords).rgb;
		specular *= attenuation;
	
		vec3 tempColor = vec3(Input.Color.rgb) * (ambientLight + diffuse + specular);
		FragColor = vec4(vec3(texture(u_Textures[int(TexIndex)], Input.TexCoords * Input.TilingFactor)) * tempColor, Input.Color.w);
	}
	else
	{
		FragColor = Input.Color;// This is for light source cubes
	}

	o_Color = FragColor;
}