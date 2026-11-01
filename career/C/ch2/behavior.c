#include <stdio.h>
#include <limits.h>
#include <stdint.h>
#include <inttypes.h>

int main(void)
{
    printf("char  %zu byte,  %d ~ %d\n", sizeof(char), CHAR_MIN, CHAR_MAX);
    printf("int   %zu bytes, %d ~ %d\n", sizeof(int), INT_MIN, INT_MAX);
    printf("long  %zu bytes, %ld ~ %ld\n", sizeof(long), LONG_MIN, LONG_MAX);
    printf("unsigned %zu bytes, %u ~ %u\n", sizeof(unsigned int), 0, UINT_MAX);
    printf("signed %zu bytes, %d ~ %d\n", sizeof(signed int), INT_MIN, INT_MAX);

    unsigned int u = UINT_MAX;
    u = u + 1;                                // 無號溢位：保證繞回 0
    printf("UINT_MAX + 1 = %u\n", u);

    uint32_t reg = 0xDEADBEEF;                // 固定 32 bit，到哪個平台都一樣
    printf("reg = 0x%08X (%zu bytes)\n", (unsigned int)reg, sizeof reg);
    printf("reg = 0x%08" PRIX32 " (%zu bytes)\n", reg, sizeof reg);

    return 0;
}