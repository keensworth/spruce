#version 460

layout(location = 0) in vec2 texCoord;

layout(location = 0) out vec4 FragColor;

layout(set = 2, binding = 0) uniform sampler2D tex;


void main() {
    FragColor = texture(tex, texCoord);
}