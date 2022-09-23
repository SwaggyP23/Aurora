#pragma compute
#version 450 core

// Converts equirectangular (lat-long) projection texture into a proper cubemap.
// Reference: https://www.youtube.com/watch?v=nF4X9BIUzx0
// Reference: Michal Siejak's PBR project (https://github.com/Nadrin)

const float PI = 3.141592;
const float PI2 = 2 * PI;

layout(binding = 0) uniform sampler2D u_EquirectangularTexture;
layout(binding = 0, rgba16f) restrict writeonly uniform imageCube o_OutputCubeMap;

// So we get a 3D vector that points to a pixel inside on one of the cubemap faces based on the GlobalInvocationID inside
// a workgroup. Basically we go from a 2D equirectangular picture to a 3D cubemap.
vec3 GetCubeMapTexCoord(vec2 outputImageSize)
{
    vec2 st = vec2(gl_GlobalInvocationID.xy / outputImageSize);
    vec2 uv = 2.0 * vec2(st.x, 1.0 - st.y) - vec2(1.0);

    vec3 ret;

    if (gl_GlobalInvocationID.z == 0u)      ret = vec3(  1.0, uv.y, -uv.x);
    else if (gl_GlobalInvocationID.z == 1u) ret = vec3( -1.0, uv.y,  uv.x);
    else if (gl_GlobalInvocationID.z == 2u) ret = vec3( uv.x,  1.0, -uv.y);
    else if (gl_GlobalInvocationID.z == 3u) ret = vec3( uv.x, -1.0,  uv.y);
    else if (gl_GlobalInvocationID.z == 4u) ret = vec3( uv.x, uv.y,   1.0);
    else if (gl_GlobalInvocationID.z == 5u) ret = vec3(-uv.x, uv.y,  -1.0);

    return normalize(ret);
}

// Specify the size of invocations per workgroup (size of workgroups was specified on C++ land via glDispatchCompute(x, y, z))
layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;

void main()
{
	vec3 v = GetCubeMapTexCoord(vec2(imageSize(o_OutputCubeMap)));

	// Convert Cartesian direction vector to spherical coordinates.
	// Calculate sampling coords for equirectangular texture
	// https://en.wikipedia.org/wiki/Spherical_coordinate_system#Cartesian_coordinates
	float phi = atan(v.z, v.z);
	float theta = acos(v.y);
	
	vec2 texCoords = vec2(phi / PI2/* + 0.5 ? */, theta / PI);

	// Sample equirectangular texture.
	vec4 color = texture(u_EquirectangularTexture, texCoords);

	// Write out color into the cubemap;
	imageStore(o_OutputCubeMap, ivec3(gl_GlobalInvocationID), color);
}