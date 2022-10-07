// Old shader and is currently useless
// Keeping it around so that i can have a bit of a small reference for phong lighting when i try to implement lighting

#pragma vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoords;
layout(location = 3) in int a_EntityID;

uniform mat4 u_ViewProjMatrix;
uniform vec3 u_CameraPosition;

struct VertexOutput
{
	vec3 position;
	vec3 normal;
	vec2 texCoords;
	vec3 cameraPos;
};

flat out int v_EntityID;
layout(location = 0) out VertexOutput Output;

void main()
{
	Output.position = a_Position;
	gl_Position = u_ViewProjMatrix * vec4(a_Position, 1.0f);

	Output.position = a_Position;
	Output.normal = a_Normal; // This should be a normal matrix which is from the inverse of the transpose of the transform which is done cpu side
	Output.texCoords = a_TexCoords;
	Output.cameraPos = u_CameraPosition;
}

#pragma fragment
#version 450 core

layout(location = 0) out vec4 o_Color;
layout(location = 1) out int o_EntityID;

//struct Light
//{
//	vec4 color;
//	vec3 position;
//	float p0;
//	vec3 direction;
//	float p1;
//	vec3 lightVector;
//	float intensity;
//};

struct Light // Each light should have different properties for each of its components and is not the same across all (Page: 129)
{
	vec3 Position;
	vec4 color;
	float intensity;
	vec3 lightVector;
};

struct Material
{
	vec3 diffuse;
	vec3 specular;
	float shininess;
};

struct VertexOutput
{
	vec3 position;
	vec3 normal;
	vec2 texCoords;
	vec3 cameraPos;
};

uniform vec3 u_LightPosition;
uniform vec4 u_LightColor;
uniform float u_LightIntensity;

// For attenuation
uniform	float u_Constant;
uniform	float u_Linear;
uniform	float u_Quadratic;

// Map inputs
uniform sampler2D u_DiffMap;
uniform sampler2D u_SpecMap;

// Static inputs
uniform vec3 u_DiffColor;
uniform vec3 u_SpecColor;
uniform float u_Shininess;

uniform vec3 u_CameraPos; // Camer pos

flat in int v_EntityID;

layout(location = 0) in VertexOutput Input;

//vec3 GetDiffMap(sampler2D tex, vec2 uv)
//{
//	return u_DiffColor + texture(u_DiffMap, uv);
//}

void main()
{
	vec3 norm = normalize(Input.normal);
//	vec3 viewDir = normalize(u_CameraPos - Input.position);
	vec3 viewDir = normalize(Input.position - u_CameraPos);

	Light light; // works for one light only
	light.Position = u_LightPosition;
	light.color = u_LightColor;
	light.intensity = u_LightIntensity;
	light.lightVector = normalize(light.Position - Input.position);

	Material material;
	material.diffuse = u_DiffColor;
	material.specular = u_SpecColor;
	material.shininess = u_Shininess;

	vec4 diffuse = vec4(0.0f);
	vec4 specular = vec4(0.0f);
	for(int i = 0; i < 1; i++)
	{
		float diffuseImpact = max(dot(norm, light.lightVector), 0.0f);
		diffuse = light.color * diffuseImpact * vec4(u_DiffColor, 1.0f);

		// Specular
		vec3 reflectionDir = reflect(-light.lightVector, norm);
		float spec = pow(max(dot(reflectionDir, viewDir), 0.0f), material.shininess);
		specular = light.color * spec * vec4(u_SpecColor, 1.0f);
	}

	o_Color = diffuse + specular;
	o_EntityID = v_EntityID;
}