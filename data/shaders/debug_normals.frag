#version 460

#define SPR_GLOBAL_BINDINGS 1
#define SPR_FRAME_BINDINGS 1
#include "common_bindings.glsl"
#include "common_constants.glsl"

layout(location = 0) out vec4 FragColor;

layout(location = 0) in vec4 pos;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 color;
layout(location = 3) in vec2 texCoord;
layout(location = 4) in flat uint drawId;

vec3 worldNormal(vec3 geomNormal, vec3 mapNormal){
    mapNormal = mapNormal * 2.0 - 1.0;
    mapNormal.x = -mapNormal.x;
    vec3 up = normalize(vec3(0.0001, 1, 0.0001));
    vec3 surfaceTangent = normalize(cross(geomNormal, up));
    vec3 surfaceBinormal = normalize(cross(geomNormal, surfaceTangent));
    return normalize(mapNormal.y * surfaceTangent + mapNormal.x * surfaceBinormal + mapNormal.z * geomNormal);
}

void main(){
    DrawData draw = draws[drawId];
    MaterialData material = materials[draw.materialOffset];
    Scene scene = sceneData;

    vec3 mapNormal = texture(textures[material.normalTexIdx], texCoord).rgb;
    mapNormal *= material.normalScale;
    
	vec3 N = worldNormal(normal, mapNormal);
    N = N * 0.5 + 0.5;

    FragColor = vec4(N, 1.0);
}