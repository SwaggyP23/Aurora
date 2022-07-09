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
flat out int light;

layout(location = 0) out VertexOutput Output;

void main()
{
	Output.Position = a_Position; // Since it is multiplied with model matrix on the cpu side
	Output.Color = a_Color;
	Output.Normals = a_Normals;
	Output.TexCoords = a_TexCoords;
	TexIndex = a_TexIndex;
	Output.TilingFactor = a_TilingFactor;
	light = a_Light;

	gl_Position = u_ViewProjmatrix * vec4(a_Position, 1.0f);
}

#pragma fragment
#version 450 core

layout(location = 0) out vec4 o_Color;

uniform sampler2D u_Textures[32];
uniform vec3 u_SourcePos;
uniform vec3 u_ViewPosition;

struct VertexOutput
{
	vec3 Position;
	vec4 Color;
	vec3 Normals;
	vec2 TexCoords;
	float TilingFactor;
};

flat in float TexIndex;
flat in int light;

layout(location = 0) in VertexOutput Input;

void main()
{
	vec4 FragColor;
	if(light == 0){
		vec4 src_color = vec4(1.0f);
		const float ambientStrength = 0.3f;
		// ambient
		vec4 ambientLight = src_color * (ambientStrength - 0.2f);
	
		// Diffuse
		vec3 norm = normalize(Input.Normals);
		vec3 lightDirection = normalize(u_SourcePos - Input.Position);
	
		float diffuseImpact = max(dot(norm, lightDirection), 0.0f);
		vec4 diffuse = diffuseImpact * src_color;
	
		// Specular
		vec3 viewDir = normalize(u_ViewPosition - Input.Position);
		vec3 reflectionDir = reflect(-lightDirection, norm);
		float specularIntensity = max(dot(reflectionDir, viewDir), 0.0f);
	
		float spec = pow(specularIntensity, 32);
		vec4 specular = specularIntensity * spec * src_color;
	
		vec4 tempColor = Input.Color * (ambientLight + diffuse + specular);
		FragColor = texture(u_Textures[int(TexIndex)], Input.TexCoords * Input.TilingFactor) * tempColor;
	}
	else
	{
		FragColor = vec4(1.0f); // This is for light source cubes
	}

	o_Color = FragColor;
}