#version 460

layout(location = 0) out vec4 FragColor;

void main(){
    float r, g, b;
    r = (gl_PrimitiveID % 256) / 255.0f;
    g = ((gl_PrimitiveID / 256) % 256) / 255.0f;
    b = ((gl_PrimitiveID / (256 * 256)) % 256) / 255.0f;
    FragColor = vec4(r, 1.0, b, 1.0);
}