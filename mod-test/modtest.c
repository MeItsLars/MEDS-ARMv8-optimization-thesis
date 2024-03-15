#include <stdio.h>
#include <stdint.h>

// #define MODULO 4093
// #define MODULO_BITS 12
#define MODULO 2039
#define MODULO_BITS 11

#define MAX_UINT32_T_VALUE 0xFFFFFFFF

// For 2039, we need 3 iterations. For 4093, we need 2 iterations (?).
uint16_t reduce_new(uint32_t a)
{
    a = a - MODULO * (a >> MODULO_BITS);
    a = a - MODULO * (a >> MODULO_BITS);
    a = a - MODULO * (a >> MODULO_BITS);
    int32_t diff = a - MODULO;
    int32_t mask = (diff >> 31) & 0x1;
    return mask * a + (1 - mask) * diff;
}

void reduce_with_prints(uint32_t a)
{
    for (int i = 0; i < 3; i++)
    {
        printf("Iteration %d\n", i + 1);
        printf("a = %u\n", a);
        printf("1) a >> MODULO_BITS = %u >> %u = %u\n", a, MODULO_BITS, a >> MODULO_BITS);
        printf("2) MODULO * (a >> MODULO_BITS) = %u * %u = %u\n", MODULO, a >> MODULO_BITS, MODULO * (a >> MODULO_BITS));
        printf("3) a - MODULO * (a >> MODULO_BITS) = %u - %u = %u\n", a, MODULO * (a >> MODULO_BITS), a - MODULO * (a >> MODULO_BITS));
        a = a - MODULO * (a >> MODULO_BITS);
    }
    printf("Final result: %u\n", a);
}

uint16_t reduce_classic(uint32_t a)
{
    return a % MODULO;
}

uint16_t mod_mul_new(uint16_t a, uint16_t b)
{
    uint32_t res = (uint32_t)a * (uint32_t)b;
    return reduce_new(res);
}

int main()
{
    for (uint32_t a = 0; a < MAX_UINT32_T_VALUE; a++)
    {
        uint16_t a_reduced = reduce_classic(a);
        uint16_t a_reduced_new = reduce_new(a);
        if (a_reduced != a_reduced_new)
        {
            printf("Error: reduce_classic(%u) = %u, reduce_new(%u) = %u\n", a, a_reduced, a, a_reduced_new);
            return 1;
        }
    }
    printf("All tests passed\n");

    // reduce_with_prints(0xfffffffe);
    return 0;
}