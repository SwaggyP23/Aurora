#shader vertex
#version 330 core
layout(location = 0) in vec3 a_Pos;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoords;
layout(location = 3) in vec4 a_Color;

out vec2 v_TexCoords;
//out vec3 v_WorldPos;
out vec3 v_Normal;
out vec3 v_FragPos;
out vec4 v_Color;

uniform mat4 vw_pr_matrix;
uniform mat4 ml_matrix;
uniform mat3 normalMatrix;

void main()
{
    v_TexCoords = a_TexCoords;
    //v_WorldPos = vec3(ml_matrix * vec4(a_Pos, 1.0));
    v_Normal = normalMatrix * a_Normal;
    v_FragPos = vec3(ml_matrix * vec4(a_Pos, 1.0f));
    v_Color = a_Color;

    gl_Position = vw_pr_matrix * ml_matrix * vec4(a_Pos, 1.0);
}

#shader fragment
#version 330 core

layout(location = 0) out vec4 FragColor;

in vec4 v_Color; // Object color
in vec3 v_FragPos;
in vec3 v_Normal;
in vec2 v_TexCoords;

uniform sampler2D SphereTexture1;
uniform float ambientStrength;
uniform vec4 src_color; // Light color
uniform vec3 src_pos;
uniform vec3 view_pos;

void main()
{
    // Ambient..
    vec4 ambientLight = src_color * ambientStrength;

    // Diffuse..
    vec3 norm = normalize(v_Normal);
    vec3 lightDir = normalize(src_pos - v_FragPos);

    float diffuseImpact = dot(lightDir, norm);
    vec4 diffuse = diffuseImpact * src_color;

    // Specular..
    vec3 viewDir = normalize(view_pos - v_FragPos);
    vec3 reflectionDir = reflect(-lightDir, norm);
    float specularIntensity = dot(viewDir, reflectionDir);

    float spec = pow(specularIntensity, 32);
    vec4 specular = specularIntensity * spec * src_color;

    FragColor = texture(SphereTexture1, v_TexCoords) * (v_Color * (ambientLight + diffuse + specular));
    //FragColor = v_Color;
}