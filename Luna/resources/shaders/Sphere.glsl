#pragma vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;
layout(location = 2) in vec3 a_Normals;

uniform mat4 u_ViewProjmatrix;

struct VertexOutput
{
	vec3 Position;
	vec4 Color;
	vec3 Normals;
};

layout(location = 0) out VertexOutput Output;

void main()
{
	Output.Position = a_Position; // Since it is multiplied with model matrix on the cpu side
	Output.Color = a_Color;
	Output.Normals = a_Normals;

	gl_Position = u_ViewProjmatrix * vec4(a_Position, 1.0f);
}

#pragma fragment
#version 450 core

layout(location = 0) out vec4 o_Color;

uniform vec3 u_ViewPosition;

struct VertexOutput
{
	vec3 Position;
	vec4 Color;
	vec3 Normals;
};

struct Matrial
{
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

	float shininess;
};

struct Light // Each light should have different properties for each of its components and is not the same across all (Page: 129)
{
	vec3 Position;

	vec3 Ambient;
	vec3 Diffuse;
	vec3 Specular;
};

uniform Matrial material;
uniform Light light;


layout(location = 0) in VertexOutput Input;

void main()
{
	vec4 FragColor;

	// ambient
	vec3 ambientLight = light.Ambient * material.ambient;
	
	// Diffuse
	vec3 norm = normalize(Input.Normals);
	vec3 lightDirection = normalize(light.Position - Input.Position);
	
	float diffuseImpact = max(dot(norm, lightDirection), 0.0f);
	vec3 diffuse = light.Diffuse * (diffuseImpact * material.diffuse);
	
	// Specular
	vec3 viewDir = normalize(u_ViewPosition - Input.Position);
	vec3 reflectionDir = reflect(-lightDirection, norm);
	float spec = pow(max(dot(reflectionDir, viewDir), 0.0f), material.shininess);
	vec3 specular = light.Specular * (spec * material.specular);
	
	vec3 tempColor = vec3(Input.Color.rgb) * (ambientLight + diffuse + specular);
	FragColor = vec4(tempColor, Input.Color.w);

//	o_Color = FragColor;
	o_Color = vec4(1.0f);
}