#version 460

layout(location = 0) out vec4 FragColor;

layout(set = 2, binding = 0) uniform sampler2D tex;


void main() {
    vec2 texCoords = vec2(gl_FragCoord.xy / textureSize(tex, 0));
    vec4 color = texture(tex, texCoords);
    FragColor = pow(color, vec4(2.2));
}