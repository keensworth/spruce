#version 460

#define SPR_GLOBAL_BINDINGS 1
#define SPR_FRAME_BINDINGS 1
#include "common_bindings.glsl"
#include "common_constants.glsl"

layout(location = 0) in vec2 texCoord;
layout(location = 1) in flat vec2 invScreenDim;
layout(location = 2) in flat float aspect;
layout(location = 3) in float spatialOffset;
layout(location = 4) in float angleOffset;

layout(location = 0) out vec4 FragColor;

layout(set = 2, binding = 0) uniform sampler2D depthMap;

// Ground truth-based ambient occlusion
// Implementation based on:
// Practical Realtime Strategies for Accurate Indirect Occlusion, Siggraph 2016
// Jorge Jimenez, Xianchun Wu, Angelo Pesce, Adrian Jarabo

// Implementation by /u/Kvaleya
// 2018-08-11

#define PI 3.1415926535897932384626433832795
#define PI_HALF 1.5707963267948966192313216916398

// [Eberly2014] GPGPU Programming for Games and Science
float GTAOFastAcos(float x)
{
	float res = -0.156583 * abs(x) + PI_HALF;
	res *= sqrt(1.0 - abs(x));
	return x >= 0 ? res : PI - res;
}

float IntegrateArc(float h1, float h2, float n)
{
	float cosN = cos(n);
	float sinN = sin(n);
	return 0.25 * (-cos(2.0 * h1 - n) + cosN + 2.0 * h1 * sinN - cos(2.0 * h2 - n) + cosN + 2.0 * h2 * sinN);
}


// These are offsets that change every frame, results are accumulated using temporal filtering in a separate shader
//uniform float angleOffset;
//uniform float spacialOffset;

vec3 Visualize_0_3(float x)
{
	const vec3 color0 = vec3(1.0, 0.0, 0.0);
	const vec3 color1 = vec3(1.0, 1.0, 0.0);
	const vec3 color2 = vec3(0.0, 1.0, 0.0);
	const vec3 color3 = vec3(0.0, 1.0, 1.0);
	vec3 color = mix(color0, color1, clamp(x - 0.0, 0.0, 1.0));
	color = mix(color, color2, clamp(x - 1.0, 0.0, 1.0));
	color = mix(color, color3, clamp(x - 2.0, 0.0, 1.0));
	return color;
}

vec3 GetCameraVec(vec2 uv)
{	
	// Returns the vector from camera to the specified position on the camera plane (uv argument), located one unit away from the camera
	// This vector is not normalized.
	// The nice thing about this setup is that the returned vector from this function can be simply multiplied with the linear depth to get pixel's position relative to camera position.
	// This particular function does not account for camera rotation or position or FOV at all (since we don't need it for AO)
	// TODO: AO is dependent on FOV, this function is not!
	// The outcome of using this simplified function is that the effective AO range is larger when using larger FOV
	// Use something more accurate to get proper FOV-independent world-space range, however you will likely also have to adjust the SSAO constants below
	return vec3(uv.x * -2.0 + 1.0, uv.y * 2.0 * aspect - aspect, 1.0);
}

#define SSAO_LIMIT 10
#define SSAO_SAMPLES 16
#define SSAO_RADIUS 0.9
#define SSAO_FALLOFF 1.5
#define SSAO_THICKNESSMIX 0.2
#define SSAO_MAX_STRIDE 4

// #define SSAO_LIMIT 100
// #define SSAO_SAMPLES 4
// #define SSAO_RADIUS 2.5
// #define SSAO_FALLOFF 1.5
// #define SSAO_THICKNESSMIX 0.2
// #define SSAO_MAX_STRIDE 32

float linearizeDepth(float d,float zNear,float zFar){
    return zNear * zFar / (zFar + d * (zNear - zFar));
}


float inverseDepth(float sampledDepth){
	float d = 1.0 - sampledDepth;
	return linearizeDepth(d, camera.near, camera.far);
}

void SliceSample(vec2 tc_base, vec2 aoDir, int i, float targetMip, vec3 ray, vec3 v, inout float closest)
{
	vec2 uv = tc_base + aoDir * i;
	float depth = inverseDepth(textureLod(depthMap, uv, targetMip).x);
	// Vector from current pixel to current slice sample
	vec3 p = GetCameraVec(uv) * depth - ray;
	// Cosine of the horizon angle of the current sample
	float current = dot(v, normalize(p));
	// Linear falloff for samples that are too far away from current pixel
	float falloff = clamp((SSAO_RADIUS - length(p)) / SSAO_FALLOFF, 0.0, 1.0);
	if(current > closest)
		closest = mix(closest, current, falloff);
	// Helps avoid overdarkening from thin objects
	closest = mix(closest, current, SSAO_THICKNESSMIX * falloff);
}

void main()
{	
	vec2 tc_original = texCoord;
	
	// Depth of the current pixel
	float dhere = inverseDepth(textureLod(depthMap, tc_original, 0.0).x);
	// Vector from camera to the current pixel's position
	vec3 ray = GetCameraVec(tc_original) * dhere;
	
	const float normalSampleDist = 1.0;
	
	// Calculate normal from the 4 neighbourhood pixels
	vec2 uv = tc_original + vec2(invScreenDim.x * normalSampleDist, 0.0);
	vec3 p1 = ray - GetCameraVec(uv) * inverseDepth(textureLod(depthMap, uv, 0.0).x);
	
	uv = tc_original + vec2(0.0, invScreenDim.y * normalSampleDist);
	vec3 p2 = ray - GetCameraVec(uv) * inverseDepth(textureLod(depthMap, uv, 0.0).x);
	
	uv = tc_original + vec2(-invScreenDim.x * normalSampleDist, 0.0);
	vec3 p3 = ray - GetCameraVec(uv) * inverseDepth(textureLod(depthMap, uv, 0.0).x);
	
	uv = tc_original + vec2(0.0, -invScreenDim.y * normalSampleDist);
	vec3 p4 = ray - GetCameraVec(uv) * inverseDepth(textureLod(depthMap, uv, 0.0).x);
	
	vec3 normal1 = normalize(cross(p1, p2));
	vec3 normal2 = normalize(cross(p3, p4));
	
	vec3 normal = normalize(normal1 + normal2);
	
	// Calculate the distance between samples (direction vector scale) so that the world space AO radius remains constant but also clamp to avoid cache trashing
	// invScreenDim = vec2(1.0 / sreenWidth, 1.0 / screenHeight)
	float stride = min((1.0 / length(ray)) * SSAO_LIMIT, SSAO_MAX_STRIDE);
	vec2 dirMult = invScreenDim.xy * stride;
	// Get the view vector (normalized vector from pixel to camera)
	vec3 v = normalize(-ray);
	
	// Calculate slice direction from pixel's position
	float dirAngle = (PI / 16.0) * (((int(gl_FragCoord.x) + int(gl_FragCoord.y) & 3) << 2) + (int(gl_FragCoord.x) & 3)) + angleOffset;
	vec2 aoDir = dirMult * vec2(sin(dirAngle), cos(dirAngle));
	
	// Project world space normal to the slice plane
	vec3 toDir = GetCameraVec(tc_original + aoDir);
	vec3 planeNormal = normalize(cross(v, -toDir));
	vec3 projectedNormal = normal - planeNormal * dot(normal, planeNormal);
	
	// Calculate angle n between view vector and projected normal vector
	vec3 projectedDir = normalize(normalize(toDir) + v);
	float n = GTAOFastAcos(dot(-projectedDir, normalize(projectedNormal))) - PI_HALF;
	
	// Init variables
	float c1 = -1.0;
	float c2 = -1.0;
	
	vec2 tc_base = tc_original + aoDir * (0.25 * ((int(gl_FragCoord.y) - int(gl_FragCoord.x)) & 3) - 0.375) + spatialOffset;
	
	const float minMip = 0.0;
	const float maxMip = 3.0;
	const float mipScale = 1.0 / 12.0;
	
	float targetMip = floor(clamp(pow(stride, 1.3) * mipScale, minMip, maxMip));
	
	// Find horizons of the slice
	for(int i = -1; i >= -SSAO_SAMPLES; i--)
	{
		SliceSample(tc_base, aoDir, i, targetMip, ray, v, c1);
	}
	for(int i = 1; i <= SSAO_SAMPLES; i++)
	{
		SliceSample(tc_base, aoDir, i, targetMip, ray, v, c2);
	}
	
	// Finalize
	float h1a = -GTAOFastAcos(c1);
	float h2a = GTAOFastAcos(c2);
	
	// Clamp horizons to the normal hemisphere
	float h1 = n + max(h1a - n, -PI_HALF);
	float h2 = n + min(h2a - n, PI_HALF);
	
	float visibility = mix(1.0, IntegrateArc(h1, h2, n), length(projectedNormal));
	
	FragColor = vec4(vec3(visibility*visibility), 1.0);
}