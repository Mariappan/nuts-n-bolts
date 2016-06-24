#include <stdio.h>
#include <stdint.h>

void typeconversion()
{
    uint16_t u16 = 24;
    uint64_t u64 = 24;
    uint16_t c16 = (uint16_t)u64;
    uint64_t c64 = (uint64_t)u16;

    printf("%u %lu %u %lu\n", u16, (uint64_t)u16, (uint16_t)u64, u64);
    printf("%u %lu %u %lu\n", u16, (uint64_t)c16, (uint16_t)c64, c64);
}

void combine_two_uint32_to_uint64()
{
    uint32_t a32 = 876758123;
    uint32_t b32 = 876231453;
    uint64_t z64 = 0;

    z64 = a32;
    z64 = (z64 << 32) | b32;

    printf("%lu %u %u\n", z64, a32, b32);

    uint32_t aa32 = (uint32_t)(z64 >> 32);
    uint32_t bb32 = (uint32_t)z64;
    printf("%lu %u %u\n", z64, aa32, bb32);
}

int main()
{
    typeconversion();
    combine_two_uint32_to_uint64();
    return 0;
}
