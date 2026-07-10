#version 460

#define SPR_GLOBAL_BINDINGS 1
#define SPR_FRAME_BINDINGS 1
#include "common_bindings.glsl"
#include "common_constants.glsl"

#define SPR_NORMALS 1
#define SPR_NORMALMAP_FLIP_Y 1
#include "common_util.glsl"

layout(location = 0) in vec4 pos;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 color;
layout(location = 3) in vec2 texCoord;
layout(location = 4) in flat uint drawId;
layout(location = 5) in vec4 tangent;

layout(location = 0) out vec4 FragColor;


void main() {
    DrawData draw = draws[drawId];
    MaterialData material = materials[draw.materialOffset];
    Transform transform = transforms[draw.transformOffset];
    
    vec3 mapNormal = texture(textures[material.normalTexIdx], texCoord).rgb;
    mapNormal.xy = mapNormal.xy * 2.0 - 1.0;
	mapNormal.z = sqrt(max(0.0, 1.0 - pow(mapNormal.x ,2.0) - pow(mapNormal.y ,2.0)));
    mapNormal.xy *= material.normalScale;
    mapNormal = normalize(mapNormal);
    
    vec3 n = normalize(normal);
    vec4 t = vec4(normalize(tangent.xyz), tangent.w);
    float sign = sign(determinant(mat3(transform.model)));
    vec3 b = cross(n, t.xyz) * t.w * sign;
    mat3 TBN = mat3(t.xyz, b, n);

    vec3 N = perturb_normal(TBN, mapNormal);
    N = N * 0.5 + 0.5;

    FragColor = vec4(N, 1.0);
}
