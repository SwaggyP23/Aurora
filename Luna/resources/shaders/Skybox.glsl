#pragma vertex
#version 450 core
layout (location = 0) in vec3 a_Position;

out vec3 v_TexCoords;

uniform mat4 u_ViewProjMatrix;

void main()
{
    v_TexCoords = a_Position;
    vec4 pos = u_ViewProjMatrix * vec4(a_Position, 1.0);
    gl_Position = pos.xyww;
}  

#pragma fragment
#version 450 core
layout(location = 0) out vec4 o_Color;
layout(location = 1) out int o_EntityID;

in vec3 v_TexCoords;

uniform samplerCube skybox;

void main()
{    
    o_Color = texture(skybox, v_TexCoords);
    o_EntityID = -1;
}


//#praga vertex
//#version 450 core
//
//layout(location = 0) in vec3 a_Position;
////layout(location = 1) in vec2 a_TexCoords;
//
//uniform mat4 u_InverseViewProjMatrix;
//
//out DATA
//{
//	vec4 Pos;
//	vec2 texCoords;
//} vs_Out;
//
//void main()
//{
//	
//}
//
//#praga fragment
//#version 450 core
//