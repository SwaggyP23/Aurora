#shader vertex
#version 330 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normals;
layout(location = 2) in vec4 a_Color;
layout(location = 3) in vec2 a_TexCoord;

out vec4 v_Color;
out vec3 v_FragPos;
out vec3 v_Normals;
out vec2 v_TexCoord;

uniform mat4 ml_matrix = mat4(1.0f);
uniform mat4 vw_pr_matrix;
uniform mat3 normalMatrix;

void main()
{
	gl_Position = vw_pr_matrix * ml_matrix * vec4(a_Position, 1.0f);
	v_Color = a_Color; //* un_color; // since now with the current vertex buffer, no colors are specified
	v_FragPos = vec3(ml_matrix * vec4(a_Position, 1.0f));
	v_FragPos = a_Position;
	v_Normals = normalMatrix * a_Normals;
	v_TexCoord = a_TexCoord;
}

#shader fragment
#version 330 core

layout(location = 0) out vec4 color;

in vec4 v_Color; // Object color
in vec3 v_FragPos;
in vec3 v_Normals;
in vec2 v_TexCoord;

uniform float ambientStrength;
uniform sampler2D Groundtexture1;
uniform vec4 src_color; // Light color
uniform vec3 src_pos;
uniform vec3 view_pos;

void main()
{
	// Ambient
	vec4 ambientLight = src_color * (ambientStrength - 0.2f);

	// Diffuse
	vec3 norm = normalize(v_Normals);
	vec3 lightDirection = normalize(src_pos - v_FragPos);

	float diffuseImpact = max(dot(norm, lightDirection), 0.0f);
	vec4 diffuse = diffuseImpact * src_color;

	// Specular
	//float specularIntensity = 0.5f;
	//vec3 viewDir = normalize(view_pos - v_FragPos);
	//vec3 reflectionDir = reflect(-lightDirection, norm);

	//float spec = pow(max(dot(viewDir, reflectionDir), 0.0f), 32);
	//vec4 specular = specularIntensity * spec * src_color;


	vec4 result = v_Color * (ambientLight + diffuse /* + specular*/);
	color = texture(Groundtexture1, v_TexCoord * 20.0f) * result;
}