#version 450 core
//#include "Chunk.glsl"
struct TileIds {
    uint packed_tileids[1024];
};

layout (location=0) in vec2 uv;
//layout(location=9) in TileIds tileIds;
layout(location=10) in flat uint tileid_offset;
layout (location=0) out vec4 FragColour;

layout(location=9) uniform sampler2DArray spriteTexture;
//buffer BufferChunk {
//
//};

layout(std430, binding = 0) readonly buffer TileIDBuffer
{
    uint packed_tileids[];
};

bool is_bit_set(uint packed_data, int n) {
    return bool((packed_data >> n) & 1u);
}
uint chop_nth_hi_bits(uint bits, uint n) {
    bits = bits << n;
    bits = bits >> n;
    return bits;
}

void main() {
    uint packed_tileid = packed_tileids[tileid_offset];
    vec2 uv_copy = uv;
    bool flipX = is_bit_set(packed_tileid, 31);
    if (flipX) {
        uv_copy.x = 1.0 - uv_copy.x;
    }
    bool flipY = is_bit_set(packed_tileid, 30);
    if (flipY) {
        uv_copy.y = 1.0 - uv_copy.y;
    }
    uint tileId = chop_nth_hi_bits(packed_tileid, 2);
    vec4 tex = texture(spriteTexture, vec3(uv_copy, tileId));
    if (tex.a < 0.1) {
        discard;
    }
    FragColour = tex;
}