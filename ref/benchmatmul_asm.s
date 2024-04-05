.cpu cortex-a72
.arch armv8-a

// void pmod_mat_mul_asm(uint16_t *C, int C_r, int C_c, uint16_t *A, int A_r, int A_c, uint16_t *B, int B_r, int B_c)

.section .text

.global pmod_mat_mul_asm
pmod_mat_mul_asm:
    // Function prologue
    stp x29, x30, [sp, -16]!
    mov x29, sp

    // Do nothing in the body

    // Function epilogue
    ldp x29, x30, [sp], 16
    ret
