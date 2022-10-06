#pragma vertex
#version 450 core
layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec2 a_TexCoords;

layout(location = 0) out vec3 v_TexCoords;

layout(std140, binding = 0) uniform Camera
{
	mat4 ViewProjectionMatrix;
	mat4 InverseViewProjectionMatrix;
	mat4 ProjectionMatrix;
	mat4 InverseProjectionMatrix;
	mat4 ViewMatrix;
	mat4 InverseViewMatrix;
	float NearClip;
	float FarClip;
	float FOV; // Radians
	float Padding0;
} u_Camera;

void main()
{
//    v_TexCoords = a_Position;
//    vec4 pos = u_Camera.ProjectionMatrix * mat4(mat3(u_Camera.ViewMatrix)) * vec4(a_Position, 1.0f);
//    gl_Position = pos.xyww;

	vec4 position = vec4(a_Position.xy, 1.0f, 1.0f);
	gl_Position = position;

	v_TexCoords = (u_Camera.InverseViewProjectionMatrix * position).xyz;
}  

#pragma fragment
#version 450 core
layout(location = 0) out vec4 o_Color;

layout(location = 0) in vec3 v_TexCoords;

layout(binding = 0) uniform samplerCube u_RadianceMap;

layout(push_constant) uniform Uniforms
{
    float TextureLOD;
    float Intensity;
} u_Uniforms;

void main()
{	
	// TODO: FIXME! The intensitiy multiplication kind of works however for now it does not register in renderdoc which i need
	// for debugging, So when im happy with the stuff and dont need renderdoc as much ill uncomment it!
    o_Color = textureLod(u_RadianceMap, v_TexCoords, u_Uniforms.TextureLOD) * u_Uniforms.Intensity;
	o_Color.a = 1.0f; // Instead of disabling then enabling blending
}
