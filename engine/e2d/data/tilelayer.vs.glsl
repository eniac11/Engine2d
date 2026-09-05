#version 450 core
struct TileIds {
    uint packed_tileid[1024];
};

layout (location=0) in vec2 vertex;
layout(location=1) in vec2 texCoord;
//layout(location=2) in TileIds tileIds;

layout (location=0) out vec2 uv;
//layout(location=9) out TileIds tileIdFs;
layout(location=10) out flat uint tileid_offset;

layout (location=0) uniform mat4 model;
layout (location=4) uniform mat4 view;
layout (location=8) uniform mat4 projection;
layout(location=10) uniform uint layer;
layout(location=11) uniform uint layer_count;
layout(location=12) uniform ivec2 chunk_id;
layout(location=13) uniform uint stack_count;
layout(location=14) uniform uint stack;

layout(std430, binding = 0) readonly buffer TileIDBuffer
{
    uint packed_tileids[];
};

//out vec4 color;

//layout (location=0) uniform sampler2D sprite;
//uniform vec3 spriteColor;

void main() {
    // gl_InstanceID * stride * uint_floordiv(gl_VertexID, 4u)
    tileid_offset = (uint(gl_InstanceID) * 1024u) + uint(gl_VertexID);
    uint tileId = packed_tileids[tileid_offset];
    // Protect against divide by 0
    uint stack_count_checked = stack_count <= 0 ? 1 : stack_count+1;
    // Clamp the layer between 0 and 1
    // FIXME: the divide might be a performance problem
    float stackz = (float(gl_InstanceID) / float(stack_count_checked));

    // Protect against divide by 0
    uint layer_count_checked = layer_count <= 0 ? 1 : layer_count+1;
    // Clamp the layer between 0 and 1
    // FIXME: the divide might be a performance problem
    float layerz = (float(layer) + (stackz)) / float(layer_count_checked);



//    float actual_layer = (layerz + stackz );
    // Clamp layerz so that it never be exactly on the clipping planes.
    // Also fixes a problem where a chunk does not render at either 0 or 1 when using depth test.
    // FIXME: May introduce z-fighting in extreme cases.
    layerz = clamp(layerz, 0.01, 0.99);
    // FIXME: This is a hack: using 1000 as a sentinel value for a chunk grid coord that has no tile.
    if (tileId == 1000) {
        // FIXME: This is a hack to put the vertex outside of the view frustum and effectively cull the vertex.
        layerz = 1000;
    }
    gl_Position = projection * view * model * vec4(vertex.xy + (chunk_id*16), layerz, 1.0);
    uv = texCoord;
//    tileIdFs = tileIds;
}