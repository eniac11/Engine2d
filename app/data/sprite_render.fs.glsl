#version 450 core
layout (location=0) in vec2 uv;
layout(location=9) in flat uint tileId;
layout (location=0) out vec4 FragColour;

layout(location=9) uniform sampler2DArray spriteTexture;

void main() {
    FragColour = texture(spriteTexture, vec3(uv, tileId));
}