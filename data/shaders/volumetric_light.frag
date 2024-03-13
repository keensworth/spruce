#version 460

#define SPR_GLOBAL_BINDINGS
#define SPR_FRAME_BINDINGS
#include "common_bindings.glsl"
#include "common_constants.glsl"

#define SPR_NORMALS
#include "common_util.glsl"

#define SPR_SHADOW_CASCADE_MAPS 1
#define SPR_SHADOW_CASCADE_DATA 2
#include "common_shadow.glsl"

layout(set = 2, binding = 0) uniform sampler2D depthMap;

layout(location = 0) in vec2 texCoord;
layout(location = 1) in flat vec2 invScreenDim;
layout(location = 2) in flat float aspect;
layout(location = 3) in vec3 camRight;
layout(location = 4) in vec3 sunDir;
layout(location = 5) in vec3 sunColor;

layout(location = 0) out vec4 FragColor;


vec3 getCameraVec(vec2 uv, vec3 fwd, vec3 right, vec3 up, vec3 pos){
    vec3 center = fwd;
    // vec3 tl = center - right + up * aspect;
    // vec3 bl = -2.0 * up * aspect * uv.y;
    // vec3 tr = 2.0 * right * uv.x;

    float y = tan(camera.fov/2);

    vec3 tl = center + -right*(y*aspect) + up*y;
    vec3 bl = -2.0 * up * y * uv.y;
    vec3 tr = 2.0 * right*(y*aspect) * uv.x;
    vec3 point = tl + bl + tr;
	return point;
}

float calculateShadow(vec4 worldPos, float distanceFromCamera) {
	uint cascadeIndex = 0;

	for(uint i = 0; i < MAX_SHADOW_CASCADES - 1; ++i) {
		if(distanceFromCamera > shadowData.cascadeSplit[0][i]) {	
			cascadeIndex = i + 1;
		}
	}

    vec4 shadowCoord = (biasMat * shadowData.cascadeViewProj[cascadeIndex]) * worldPos;	

	// float mixedDist = shadowData.cascadeSplit[0][cascadeIndex];
	// if (cascadeIndex > 0){
	// 	float nearDist = shadowData.cascadeSplit[0][cascadeIndex-1];
	// 	float farDist = mixedDist;
	// 	mixedDist = mix(nearDist, farDist, (distanceFromCamera-nearDist)/(farDist - nearDist));
	// }

	float bias = 0.000025; //min(0.05 * max(1.0 - dot(normal, lightDir), 0.0), 0.005);
	//bias *= 1.0 / (mixedDist * 0.5);

	float shadow = filterPCF(shadowCoord, cascadeIndex, bias);
    
    //float shadow = textureProj(shadowCoord, vec2(0.0), cascadeIndex, 0.05);
	return shadow;
}

#define G_SCATTERING 0.6
// Henyey-Greenstein
float computeInScattering(float lightDotView){
    float result = 1.0 - G_SCATTERING * G_SCATTERING;
    result /= (4.0 * PI * pow(1.0 + G_SCATTERING * G_SCATTERING - (2.0 * G_SCATTERING) * lightDotView, 1.5));
    return result;
}

// Beer-Lambert
vec4 accumulateScattering(vec4 colorAndDensityFront, vec4 colorAndDensityBack){
    vec3 light = colorAndDensityFront.rgb + clamp(exp(-colorAndDensityFront.a), 0.0, 1.0) * colorAndDensityBack.rgb;
    return vec4(light.rgb, colorAndDensityFront.a + colorAndDensityBack.a);
}

#define VL_SAMPLES 128


void main(){
    vec2 tc_original = texCoord;

    // depth of scene
    float dhere = inverseDepth(textureLod(depthMap, tc_original, 0.0).x);

    // ray from camera to geometry surface (world space)
    vec3 camDir = camera.dir;
    vec3 camPos = camera.pos;
    vec3 camUp = camera.up;
    vec3 ray = getCameraVec(tc_original, camDir, camRight, camUp, camPos);
    vec3 tempRayCast = ray*dhere;
    vec3 tempAlso = camPos + tempRayCast;
    //ray.x = -ray.x;
    //ray.z = -ray.z;
    //ray.y = -ray.y;

    // single step
    vec3 stepVector = (ray*dhere) / VL_SAMPLES;

    float viewDotLight = dot(sunDir, camDir);
    //viewDotLight = min(viewDotLight, -viewDotLight);
    float scattering = computeInScattering(viewDotLight);

    // step from camera to back of scene
    vec4 colorAndDensity = vec4(0.0);
    for (uint z = 0; z < VL_SAMPLES; z++) {
        float stepDepth = z * (dhere / VL_SAMPLES);
        vec3 stepPos = camPos + z * stepVector;
        float shadow = calculateShadow(vec4(stepPos, 1.0), stepDepth);

        // in scattering at world pos
        // (just sun lightshafts, no fog)
        vec4 currColorAndDensity = vec4(0.0, 0.0, 0.0, 0.0);
        if (shadow == 1.0){
            currColorAndDensity = vec4(sunColor*scattering, scattering);
        }

        // out scattering between current and prev sample
        colorAndDensity = accumulateScattering(colorAndDensity, currColorAndDensity);
    }

    FragColor = vec4(colorAndDensity.rgb, exp(-colorAndDensity.a));
}

// TODO:
// vec4 scatteringInfo = ...
// vec3 inScattering = scatteringInfo.rgb;
// float transmittance = scatteringInfo.a;
// vec3 finalColor = pixelColorWithoutFog * transmittance.xxx + inScattering

// pbr_frag etc