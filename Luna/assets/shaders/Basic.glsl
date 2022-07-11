#pragma vertex
#version 330 core

layout (location = 0) in vec3 a_Position; // since this is an attribute, takes the a_ Prefix.
layout (location = 1) in vec3 a_Normals;
layout (location = 2) in vec4 a_Color;
layout (location = 3) in vec2 a_TexCoord;

out vec4 v_Color;
out vec3 v_FragPos;
out vec3 v_Normals;
out vec2 v_TexCoord;

uniform mat4 ml_matrix;
uniform mat4 vw_pr_matrix;
uniform mat3 normalMatrix;
uniform vec4 un_color = vec4(1.0f);

void main()
{
	gl_Position = vw_pr_matrix * ml_matrix * vec4(a_Position, 1.0f);
	v_Color = a_Color * un_color; // since now with the current vertex buffer, no colors are specified
	v_FragPos = vec3(ml_matrix * vec4(a_Position, 1.0f));
	//v_Normals = a_Normals; // This would not react with the uniform trasformation like rotations and translate.
	v_Normals = normalMatrix * a_Normals;
	v_TexCoord = a_TexCoord;
}


#pragma fragment
#version 330 core

layout (location = 0) out vec4 color;

in vec4 v_Color; // Object color
in vec3 v_FragPos;
in vec3 v_Normals;
in vec2 v_TexCoord;

uniform float blend;
uniform float ambientStrength = -0.3f;
uniform sampler2D texture1;
uniform sampler2D texture2;
uniform vec4 src_color = vec4(1.0f); // Light color
uniform vec3 src_pos;
uniform vec3 view_pos;

void main()
{
	// Ambient
	vec4 ambientLight = src_color * ambientStrength;

	// Diffuse
	vec3 norm = normalize(v_Normals);
	vec3 lightDirection = normalize(src_pos - v_FragPos);

	float diffuseImpact = max(dot(norm, lightDirection), 0.0f);
	vec4 diffuse = diffuseImpact * src_color;

	// Specular
	//float specularIntensity = 0.5f;
	vec3 viewDir = normalize(view_pos - v_FragPos);
	vec3 reflectionDir = reflect(-lightDirection, norm);
	float specularIntensity = max(dot(reflectionDir, viewDir), 0.0f);

	float spec = pow(specularIntensity, 32);
	vec4 specular = specularIntensity * spec * src_color;

	color = mix(texture(texture1, v_TexCoord), texture(texture2, v_TexCoord), blend) * (v_Color * (ambientLight + diffuse + specular));
}