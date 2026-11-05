#version 460

layout (location=0) in vec2 vertex;
layout(location=1) in vec2 texCoord;

layout(location=0) out vec2 uv;

layout(location=0) uniform mat4 projection;
layout(location=4) uniform mat4 view;
layout(binding=0) uniform UBO1 {
    mat4 model;
    vec4 colour;
    uint tileId;
} GPUSprite;


void main() {
    gl_Position = projection * view * GPUSprite.model * vec4(vertex.xy, 0, 1.0);
    uv = texCoord;
}