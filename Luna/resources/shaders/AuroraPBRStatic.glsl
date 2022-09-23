#pragma vertex
#version 450 core
layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec2 a_TexCoords;

layout(std140, binding = 0) uniform Camera
{
    mat4 u_ViewProjMatrix;
    mat4 u_SkyVP;
};

struct VertexOutput
{
    vec2 TexCoords;
};

layout(location = 0) out VertexOutput v_Output;

layout(push_constant) uniform Transform
{
    mat4 transform;
} u_Renderer;

invariant gl_Position;

void main()
{
    mat4 model = u_Renderer.transform;
    v_Output.TexCoords = a_TexCoords;
    gl_Position = u_ViewProjMatrix * model * vec4(a_Position, 1.0);
}  

#pragma fragment
#version 450 core
layout(location = 0) out vec4 o_Color;
layout(location = 1) out int o_EntityID;

// PBR texture inputs
layout(binding = 0) uniform sampler2D u_AlbedoTexture;
layout(binding = 1) uniform sampler2D u_NormalTexture;
layout(binding = 2) uniform sampler2D u_MetalnessTexture;
layout(binding = 3) uniform sampler2D u_RoughnessTexture;

struct VertexOutput
{
    vec2 TexCoords;
};

layout(location = 0) in VertexOutput v_Input;

layout(push_constant) uniform Materials
{
    vec4 AlbedoColor;
    float Metalness;
    float Roughness;

    bool UseNormalMap;
} u_MaterialUniforms;

void main()
{    
    o_Color = texture(u_AlbedoTexture, v_Input.TexCoords) * u_MaterialUniforms.AlbedoColor;
    o_EntityID = -1;
}