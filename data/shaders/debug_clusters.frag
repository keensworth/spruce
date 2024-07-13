#version 460

#define SPR_GLOBAL_BINDINGS
#define SPR_FRAME_BINDINGS
#include "common_bindings.glsl"
#include "common_constants.glsl"

#define SPR_NORMALS
#include "common_util.glsl"

#define SPR_SHADOW_CASCADE_MAPS 2
#define SPR_SHADOW_CASCADE_DATA 3
#include "common_shadow.glsl"

layout(set = 2, binding = 0) uniform sampler2D depthMap;
layout(set = 2, binding = 1) uniform sampler2D occlusionMap;
// binding 2 & 3 defined in common_shadow.glsl
layout(set = 2, binding = 4) uniform sampler2D scatteringMap;

struct Cluster {
    uint offset;
    uint count;
};
layout (std430, set = 3, binding = 0) buffer ClusterList {
    Cluster clusters[];
};
layout (std430, set = 3, binding = 1) buffer LightList {
    uint lightIndices[];
};
layout (std430, set = 3, binding = 2) buffer GlobalIndexCount {
    uint globalIndexCount;
};

layout(location = 0) in vec4 pos;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 color;
layout(location = 3) in vec2 texCoord;
layout(location = 4) in flat uint drawId;
layout(location = 5) in vec4 viewPos;

layout(location = 0) out vec4 FragColor;

const uvec3 clusterCounts = {16, 8, 24};

// heatmap color pallette
vec3 debugColorPalette[6] = vec3[](
    vec3( 53.0, 25.0, 62.0)/255.0,
    vec3(112.0, 31.0, 87.0)/255.0,
    vec3(173.0, 23.0, 89.0)/255.0,
    vec3(225.0, 51.0, 66.0)/255.0,
    vec3(243.0,118.0, 81.0)/255.0,
    vec3(246.0,180.0,143.0)/255.0
);

Cluster getCluster(){
	float zFar = camera.far;
	float zNear = camera.near;

	float sliceCount = clusterCounts.z;

	vec2 tileSizePx = vec2(sceneData.screenDimX / clusterCounts.x, sceneData.screenDimY / clusterCounts.y);

	float z = -viewPos.z;

	uvec3 clusterPos;
	clusterPos.xy = uvec2(gl_FragCoord.xy / tileSizePx);
	clusterPos.z = uint((log(abs(z) / zNear) * sliceCount) / log(zFar / zNear));

	uint clusterIndex = clusterPos.x + 
						clusterPos.y * clusterCounts.x +
						clusterPos.z * clusterCounts.x * clusterCounts.y;

	return clusters[clusterIndex];
}

void main() {
	// get relevant light cluster
	Cluster cluster = getCluster();
	uint offset = cluster.offset;
	uint count = cluster.count;

	uint debugColorBase = count / (256/6);
	float interp = float(count % (256/6)) / float(256/6);

	vec3 mixedColor = debugColorPalette[debugColorBase] * (1.0 - interp) + debugColorPalette[debugColorBase + 1] * interp;

    FragColor = vec4(mixedColor, 1.0);


	// DrawData draw = draws[drawId];
    // MaterialData material = materials[draw.materialOffset];
    // Scene scene = sceneData;

    // vec4 color = vec4(texture(textures[material.baseColorTexIdx], texCoord).rgb, 0.7);

	// float zFar = camera.far;
	// float zNear = camera.near;
	// float sliceCount = clusterCounts.z;
	// float z = -viewPos.z;
	// vec2 tileSizePx = vec2(sceneData.screenDimX / clusterCounts.x, sceneData.screenDimY / clusterCounts.y);
	// uint slice = uint((log(abs(z) / zNear) * sliceCount) / log(zFar / zNear));

	// vec4 cascadeColor = vec4(0.0);
    // if (slice%5 == 0){
    //     cascadeColor = vec4(1.0, 0.0, 0.0, 0.3);
    // } else if (slice%5 == 1){
    //     cascadeColor = vec4(0.0, 1.0, 0.0, 0.3);
    // } else if (slice%5 == 2){
    //     cascadeColor = vec4(0.0, 0.0, 1.0, 0.3);
    // } else if (slice%5 == 3){
    //     cascadeColor = vec4(1.0, 1.0, 0.0, 0.3);
    // } else {
    //     cascadeColor = vec4(1.0, 1.0, 1.0, 0.3);
    // }

    // FragColor = color + cascadeColor;
}