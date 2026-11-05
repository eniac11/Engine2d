bool is_bit_set(uint packed_data, int n) {
    return bool((packed_data >> n) & 1u);
}
uint chop_nth_hi_bits(uint bits, uint n) {
    bits = bits << n;
    bits = bits >> n;
    return bits;
}