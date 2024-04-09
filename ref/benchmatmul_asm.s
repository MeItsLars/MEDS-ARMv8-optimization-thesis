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

// extern void pmod_mat_reduce_asm(uint16_t *C, int C_r, int C_c, uint16_t *tmp);

  //     C_tmp = vshrq_n_u32(C_red, GFq_bits);
  //     C_tmp = vmulq_n_u32(C_tmp, MEDS_p);
  //     C_red = vsubq_u32(C_red, C_tmp);
  //     C_tmp = vshrq_n_u32(C_red, GFq_bits);
  //     C_tmp = vmulq_n_u32(C_tmp, MEDS_p);
  //     C_red = vsubq_u32(C_red, C_tmp);
  //     C_red_u16 = vqmovn_u32(C_red);

.global pmod_mat_reduce_asm
pmod_mat_reduce_asm:
    // The four function parameters are stored in x0, x1, x2, x3

    // Store relevant values in NEON registers v17 and v18
    mov x4, 1153
    movk x4, 0x8018, lsl 16
    mov x5, 4093 // MEDS_p
    dup v17.2d, x4
    dup v18.2d, x5

    // Compute C_r*C_c
    mul x1, x1, x2
/*
    // Loop from 0 to C_r*C_c
    // Increment with 2, because we will use the NEON registers
    mov x4, 0
    loop:
        // Load the values from tmp
        ld1 {v1.2d}, [x3], #16   // v1 = tmp[i], tmp[i+1]
        ld1 {v5.2d}, [x3], #16   // v5 = tmp[i+2], tmp[i+3]

        // Multiply the tmp values with w4
        umull v2.2d, v1.2d, v17.2d
        umull v6.2d, v5.2d, v17.2d
        // Shift right by 43
        uqrshl v2.2d, v2.2d, #43
        uqrshl v6.2d, v6.2d, #43
        // Multiply the tmp values with w5 (MEDS_p)
        umull v2.2d, v2.2d, v18.2d
        umull v6.2d, v6.2d, v18.2d
        // Subtract the result from the original value
        sub v1.2d, v1.2d, v2.2d
        sub v5.2d, v5.2d, v6.2d

        // Store the results back to C
        st1 {v1.2d}, [x0], #8
        st1 {v5.2d}, [x0], #8

        // Increment i and check if we are done
        add x4, x4, 4
        cmp x4, x1
        b.lt loop
*/
    ret

/*
pmod_mat_reduce_asm:
        mul     w5, w1, w2
        cmp     w5, 0
        ble     .L1
        mov     w7, 1153
        sxtw    x5, w5
        mov     x2, 0
        movk    w7, 0x8018, lsl 16
        mov     w6, 4093
.L3:
        ldr     w4, [x3, x2, lsl 2]
        umull   x1, w4, w7
        lsr     x1, x1, 43
        msub    w1, w1, w6, w4
        strh    w1, [x0, x2, lsl 1]
        add     x2, x2, 1
        cmp     x2, x5
        bne     .L3
.L1:
        ret
*/
