#version 460

#define SPR_GLOBAL_BINDINGS 1
#define SPR_FRAME_BINDINGS 1
#include "common_bindings.glsl"

#define SPR_RANDOM 1
#include "common_util.glsl"

layout(location = 0) out vec2 texCoord;
layout(location = 1) out flat vec2 invScreenDim;
layout(location = 2) out flat float aspect;
layout(location = 3) out vec3 camRight;
layout(location = 4) out vec3 sunDir;
layout(location = 5) out vec3 sunColor;

vec2 positionsHard[4] = vec2[](
    vec2(-1.0,  1.0),
    vec2(-1.0, -1.0),
    vec2( 1.0, -1.0),
    vec2( 1.0,  1.0)
);

void main() {
    Scene scene = sceneData;

    invScreenDim = vec2(1.0 / float(scene.screenDimX), 1.0 / float(scene.screenDimY));
    aspect = float(scene.screenDimX) / float(scene.screenDimY);

    vec4 pos = vec4(positionsHard[gl_VertexIndex], 0.0, 1.0);

    float x = pos.x;
    float y = -pos.y;
    texCoord.x = x * 0.5 + 0.5;
    texCoord.y = y * 0.5 + 0.5;

    camRight = cross(camera.dir, camera.up);

    Light sun = lights[scene.sunOffset];
    sunDir = sun.dir;
    sunColor = sun.color;
    
    gl_Position = pos;
}
