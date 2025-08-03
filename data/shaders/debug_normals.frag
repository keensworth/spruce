#version 460

#define SPR_GLOBAL_BINDINGS 1
#define SPR_FRAME_BINDINGS 1
#include "common_bindings.glsl"
#include "common_constants.glsl"

#define SPR_NORMALS 1
#include "common_util.glsl"

layout(location = 0) in vec4 pos;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 color;
layout(location = 3) in vec2 texCoord;
layout(location = 4) in flat uint drawId;
layout(location = 5) in mat3 TBN;

layout(location = 0) out vec4 FragColor;


void main() {
    DrawData draw = draws[drawId];
    MaterialData material = materials[draw.materialOffset];
    
    vec3 mapNormal = texture(textures[material.normalTexIdx], texCoord).rgb;
    mapNormal = mapNormal * 2.0 - 1.0;
    mapNormal *= normalize(vec3(material.normalScale, material.normalScale, 1.0));
    
    vec3 N = vec3(0.0);
    if (isnan(TBN[0][0])){
        N = perturb_normal(normal, camera.pos - pos.xyz, texCoord, mapNormal);
    } else {
        N = perturb_normal(TBN, mapNormal);
    }
    
    N = N * 0.5 + 0.5;

    FragColor = vec4(N, 1.0);
}