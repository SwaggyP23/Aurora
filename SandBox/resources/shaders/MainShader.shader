#shader vertex
#version 330 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;
layout(location = 2) in vec2 a_TexCoords;

out vec4 v_Color;
out vec2 v_TexCoords;

uniform mat4 u_ViewProjmatrix;
//uniform mat4 u_ModelMatrix;

void main()
{
	v_Color = a_Color;
	v_TexCoords = a_TexCoords;
	gl_Position = u_ViewProjmatrix * /*u_ModelMatrix **/ vec4(a_Position, 1.0f);
}

#shader fragment
#version 330 core

layout(location = 0) out vec4 FragColor;

in vec4 v_Color;
in vec2 v_TexCoords;

//uniform float u_TilingFactor;
//uniform vec4 u_Color;
//uniform sampler2D u_Texture;

void main()
{
	// FragColor = texture(u_Texture, v_TexCoords * u_TilingFactor) * v_Color;
	FragColor = v_Color;
}