#version 450 core
layout (location=0) in vec2 vertex;
layout(location=1) in vec2 texCoord;
layout(location=2) in uint tileId;
layout (location=3) in mat4 model;

layout (location=0) out vec2 uv;
layout(location=9) out uint tileIdFs;

//layout (location=0) uniform mat4 model;
layout (location=4) uniform mat4 view;
layout (location=8) uniform mat4 projection;

layout(location=10) uniform float layer;

//out vec4 color;

//layout (location=0) uniform sampler2D sprite;
//uniform vec3 spriteColor;

void main() {
    gl_Position = projection * view * model * vec4(vertex.xy, layer, 1.0);
    uv = texCoord;
    tileIdFs = tileId;
}