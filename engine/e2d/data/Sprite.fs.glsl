#version 460

layout(location=0) in vec2 uv;

layout (location=0) out vec4 FragColour;

layout(binding=0) uniform UBO1 {
    mat4 model;
    vec4 colour;
    uint tileId;
} GPUSprite;
layout(location=15) uniform sampler2DArray spriteTexture;

void main() {
    vec4 tex = GPUSprite.colour /*** texture(spriteTexture, vec3(uv, GPUSprite.tileId))*/;
    FragColour = tex;

}