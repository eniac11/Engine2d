#version 450 core
layout (location=0) in vec2 vertex;
layout(location=1) in vec2 texCoord;
layout(location=2) in uint tileId;

layout (location=0) out vec2 uv;
layout(location=9) out uint tileIdFs;

layout (location=0) uniform mat4 model;
layout (location=4) uniform mat4 view;
layout (location=8) uniform mat4 projection;
layout(location=10) uniform uint layer;
layout(location=11) uniform uint layer_count;

//out vec4 color;

//layout (location=0) uniform sampler2D sprite;
//uniform vec3 spriteColor;

void main() {
    // Protect against divide by 0
    uint layer_count_checked = layer_count <= 0 ? 1 : layer_count;
    // Clamp the layer between 0 and 1
    // FIXME: the divide might be a performance problem
    float layerz = float(layer) / float(layer_count_checked);
    // Clamp layerz so that it never be exactly on the clipping planes.
    // Also fixes a problem where a chunk does not render at either 0 or 1 when using depth test.
    // FIXME: May introduce z-fighting in extreme cases.
    layerz = clamp(layerz, 0.01, 0.99);
    gl_Position = projection * view * model * vec4(vertex.xy, layerz, 1.0);
    uv = texCoord;
    tileIdFs = tileId;
}