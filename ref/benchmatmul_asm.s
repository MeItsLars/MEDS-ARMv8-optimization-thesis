.cpu cortex-a72
.arch armv8-a

// void pmod_mat_mul_asm(uint16_t *C, uint16_t *A, uint16_t *B, int m, int n, int o);

// https://wiki.cdot.senecapolytechnic.ca/wiki/AArch64_Register_and_Instruction_Quick_Start

// A0 stored in v0.4s
// A1 stored in v1.4s
// A2 stored in v2.4s
// A3 stored in v3.4s
// B0 stored in v4.4h
// B1 stored in v5.4h
// B2 stored in v6.4h
// B3 stored in v7.4h
// C0 stored in v8.4h
// C1 stored in v9.4h
// C2 stored in v10.4h
// C3 stored in v11.4h
// C0_tmp stored in v12.4s
// C1_tmp stored in v13.4s
// C2_tmp stored in v14.4s
// C3_tmp stored in v15.4s
// MEDS_p stored in v16.4s

// x0:  C
// x1:  A
// x2:  B
// x3:  m = C_r = A_r
// x4:  n = A_c = B_r
// x5:  o = C_c = B_c
// x6:  2m = 2C_r = 2A_r
// x7:  2n = 2A_c = 2B_r
// x8:  2o = 2C_c = 2B_c
// x9:  r
// x10: c
// x11: k
// x12: Ai
// x13: Bi

// x9-x15: temporary
// x19-x28: callee-saved (meaning we need to save them if we use them)

// Load Bc into x8 but only lower half to fix segmentation fault?
// ldrh w8, [sp, 0]

.section .text

.global pmod_mat_mul_asm
pmod_mat_mul_asm:
    // Initialize MEDS_p into v16.4s
    mov w9, #4093
    dup v16.4s, w9

    // Widen m, n and o so that we can use them as offsets
    lsl x6, x3, #1
    lsl x7, x4, #1
    lsl x8, x5, #1

    mov x9, #0  // Initialize r = 0
r_loop:
    mov x10, #0 // Initialize c = 0
c_loop:
    mov x11, #0 // Initialize k = 0
k_loop:
    // Initialize Ai and Bi to become:
    // Ai = idx of A[r * A_c + k] = A + 2 * (r * A_c + k)
    madd x12, x9, x4, x11     // x12 = r * A_c + k
    add x12, x1, x12, lsl #1 // x12 = A + 2 * (r * A_c + k)
    // Bi = idx of B[k * B_c + c] = B + 2 * (k * B_c + c)
    madd x13, x11, x5, x10    // x13 = k * B_c + c
    add x13, x2, x13, lsl #1 // x13 = B + 2 * (k * B_c + c)

    // Initialize A0-A3
    ld1 {v0.4h, v1.4h, v2.4h, v3.4h}, [x12], x4

    // Load B0-B3
    ld1 {v4.4h, v5.4h, v6.4h, v7.4h}, [x13], x5
    
    // Compute C0
    umull v8.4s, v4.4h, v0.4h[0] // C0 = B0 * A[A0 + 0]
    umlal v8.4s, v5.4h, v0.4h[1] // C0 = C0 + B1 * A[A0 + 1]
    umlal v8.4s, v6.4h, v0.4h[2] // C0 = C0 + B2 * A[A0 + 2]
    umlal v8.4s, v7.4h, v0.4h[3] // C0 = C0 + B3 * A[A0 + 3]

    // Compute C1
    umull v9.4s, v4.4h, v1.4h[0] // C1 = B0 * A[A1 + 0]
    umlal v9.4s, v5.4h, v1.4h[1] // C1 = C1 + B1 * A[A1 + 1]
    umlal v9.4s, v6.4h, v1.4h[2] // C1 = C1 + B2 * A[A1 + 2]
    umlal v9.4s, v7.4h, v1.4h[3] // C1 = C1 + B3 * A[A1 + 3]

    // Compute C2
    umull v10.4s, v4.4h, v2.4h[0] // C2 = B0 * A[A2 + 0]
    umlal v10.4s, v5.4h, v2.4h[1] // C2 = C2 + B1 * A[A2 + 1]
    umlal v10.4s, v6.4h, v2.4h[2] // C2 = C2 + B2 * A[A2 + 2]
    umlal v10.4s, v7.4h, v2.4h[3] // C2 = C2 + B3 * A[A2 + 3]

    // Compute C3
    umull v11.4s, v4.4h, v3.4h[0] // C3 = B0 * A[A3 + 0]
    umlal v11.4s, v5.4h, v3.4h[1] // C3 = C3 + B1 * A[A3 + 1]
    umlal v11.4s, v6.4h, v3.4h[2] // C3 = C3 + B2 * A[A3 + 2]
    umlal v11.4s, v7.4h, v3.4h[3] // C3 = C3 + B3 * A[A3 + 3]

    // Reduce C0-C3 modulo 4093
    // C0 = C0 % 4093
    ushr v12.4s, v8.4s, #12
    mul v12.4s, v12.4s, v16.4s
    sub v8.4s, v8.4s, v12.4s
    ushr v12.4s, v8.4s, #12
    mul v12.4s, v12.4s, v16.4s
    sub v8.4s, v8.4s, v12.4s
    // C1 = C1 % 4093
    ushr v13.4s, v9.4s, #12
    mul v13.4s, v13.4s, v16.4s
    sub v9.4s, v9.4s, v13.4s
    ushr v13.4s, v9.4s, #12
    mul v13.4s, v13.4s, v16.4s
    sub v9.4s, v9.4s, v13.4s
    // C2 = C2 % 4093
    ushr v14.4s, v10.4s, #12
    mul v14.4s, v14.4s, v16.4s
    sub v10.4s, v10.4s, v14.4s
    ushr v14.4s, v10.4s, #12
    mul v14.4s, v14.4s, v16.4s
    sub v10.4s, v10.4s, v14.4s
    // C3 = C3 % 4093
    ushr v15.4s, v11.4s, #12
    mul v15.4s, v15.4s, v16.4s
    sub v11.4s, v11.4s, v15.4s
    ushr v15.4s, v11.4s, #12
    mul v15.4s, v15.4s, v16.4s
    sub v11.4s, v11.4s, v15.4s

    // Shrink C0-C3 to 16-bit
    sqxtn v8.4h, v8.4s
    sqxtn v9.4h, v9.4s
    sqxtn v10.4h, v10.4s
    sqxtn v11.4h, v11.4s
k_loop_end:
    // Store C0-C3
    st1 {v8.4h, v9.4h, v10.4h, v11.4h}, [x0], x5

    // Increment k and branch if k < A_c
    add x11, x11, #4
    cmp x11, x4
    blt k_loop
c_loop_end:
    // Increment c and branch if c < C_c
    add x10, x10, #4
    cmp x10, x5
    blt c_loop
r_loop_end:
    // Increment r and branch if r < C_r
    add x9, x9, #4
    cmp x9, x3
    blt r_loop
end:
    ret
