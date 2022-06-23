#shader vertex
#version 330 core

layout (location = 0) in vec3 a_Position; // since this is an attribute, takes the a_ Prefix.
//layout (location = 1) in vec4 a_Color;
layout (location = 1) in vec2 a_TexCoord;

//out vec4 v_Color;
out vec2 v_TexCoord;

uniform mat4 ml_matrix;
uniform mat4 vw_matrix;
uniform mat4 pr_matrix;

void main()
{
	//gl_Position = translation * rotation * scale * vec4(a_Position, 1.0f);
	gl_Position = pr_matrix * vw_matrix * ml_matrix * vec4(a_Position, 1.0f);
	//gl_Position = transform * vec4(a_Position, 1.0f);
	//v_Color = a_Color;
	v_TexCoord = a_TexCoord;
}


#shader fragment 
#version 330 core

layout (location = 0) out vec4 color;

//in vec4 v_Color;
in vec2 v_TexCoord;

uniform float blend;
uniform sampler2D texture1;
uniform sampler2D texture2;

void main()
{
	color = mix(texture(texture1, v_TexCoord), texture(texture2, v_TexCoord), blend)/* * v_Color*/;
}