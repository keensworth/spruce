#version 460

#define SPR_GLOBAL_BINDINGS 1
#define SPR_FRAME_BINDINGS 1
#include "common_bindings.glsl"
#include "common_constants.glsl"

#define SPR_NORMALS 1
#include "common_util.glsl"

layout(location = 0) out vec4 FragColor;

layout(location = 0) in vec4 pos;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 color;
layout(location = 3) in vec2 texCoord;
layout(location = 4) in flat uint drawId;


void main(){
    DrawData draw = draws[drawId];
    MaterialData material = materials[draw.materialOffset];
    Scene scene = sceneData;

    vec3 mapNormal = texture(textures[material.normalTexIdx], texCoord).rgb;
    mapNormal = normalize(mapNormal * 2.0 - 1.0);
    mapNormal *= vec3(material.normalScale, material.normalScale, 1.0);
    
    vec3 N = perturb_normal(normal, camera.pos - pos.xyz, texCoord, mapNormal);
    N = N * 0.5 + 0.5;
    // N = normal * 0.5 + 0.5;

    FragColor = vec4(N, 1.0);
}