#version 460

#define SPR_GLOBAL_BINDINGS 1
#define SPR_FRAME_BINDINGS 1
#include "common_bindings.glsl"

layout(set = 2, binding = 0) uniform sampler2D depthMap;

layout(location = 0) in vec3 texCoord;

layout(location = 0) out vec4 FragColor;

void main() {
    Scene scene = sceneData;
    vec2 fragUV = vec2(gl_FragCoord.xy / textureSize(depthMap, 0));

    // verify no geometry was drawn here
    float depth = texture(depthMap, fragUV).r;
    if (depth != 0.0){
		discard;
	}

    const float gamma = 2.2;
    vec3 hdrColor = texture(cubemaps[0], texCoord).rgb;
    
    vec3 mapped = vec3(1.0) - exp(-hdrColor * scene.exposure);
    mapped = pow(mapped, vec3(1.0 / gamma));
  
    FragColor = vec4(mapped, 1.0);
}
