.cpu cortex-a72
.arch armv8-a

// void pmod_mat_mul_asm(uint16_t *C, uint16_t *A, uint16_t *B, int m, int o, int n);

// https://wiki.cdot.senecapolytechnic.ca/wiki/AArch64_Register_and_Instruction_Quick_Start

// A0 stored in v0.4h
// A1 stored in v1.4h
// A2 stored in v2.4h
// A3 stored in v3.4h
// B0 stored in v4.4h
// B1 stored in v5.4h
// B2 stored in v6.4h
// B3 stored in v7.4h
// C0 stored in v8.4s
// C1 stored in v9.4s
// C2 stored in v10.4s
// C3 stored in v11.4s
// C0_tmp stored in v12.4s
// C1_tmp stored in v13.4s
// C2_tmp stored in v14.4s
// C3_tmp stored in v15.4s
// MEDS_p stored in v16.4s

// x0:  C > contains the running value for C; it is incremented by 2 * C_c at the end of each loop_r
// x1:  A
// x2:  B
// x3:  m = C_r = A_r > becomes 2m
// x4:  o = A_c = B_r > becomes 2o
// x5:  n = C_c = B_c > becomes 2n
// x6:  -
// x7:  -
// x8:  -
// x9:  r
// x10: c
// x11: k
// x12: Ai
// x13: Bi
// x14: Ci
// x15: temp register for loading values from A and B

// x9-x15: temporary
// x19-x28: callee-saved (meaning we need to save them if we use them)

.global pmod_mat_mul_asm
pmod_mat_mul_asm:
    // Initialize MEDS_p into v16.4s
    mov w9, #4093
    dup v16.4s, w9

    // Widen m, o and n so that we can use them as offsets
    lsl x3, x3, #1
    lsl x4, x4, #1
    lsl x5, x5, #1

    mov x9, #0  // Initialize r = 0
r_loop:
    mov x10, #0 // Initialize c = 0
c_loop:
    mov x11, #0 // Initialize k = 0
k_loop_1:
    // Ai = idx of A[r * A_c + k] = A + 2r * A_c + 2k
    madd x12, x4, x9, x1 // Ai = A + 2rA_c (since k = 0)
    // Bi = idx of B[k * B_c + c] = B + 2k * B_c + 2c
    add x13, x2, x10, lsl #1 // Bi = B + 2c (since k = 0)

    // Initialize A0-A3
    // ld1 {v0.4h, v1.4h, v2.4h, v3.4h}, [x12], x4
    ld1 {v0.4h}, [x12]
    add x15, x12, x4
    ld1 {v1.4h}, [x15]
    add x15, x15, x4
    ld1 {v2.4h}, [x15]
    add x15, x15, x4
    ld1 {v3.4h}, [x15]

    // Load B0-B3
    // ld1 {v4.4h, v5.4h, v6.4h, v7.4h}, [x13], x5
    ld1 {v4.4h}, [x13]
    add x15, x13, x5
    ld1 {v5.4h}, [x15]
    add x15, x15, x5
    ld1 {v6.4h}, [x15]
    add x15, x15, x5
    ld1 {v7.4h}, [x15]

    // Use umull for the first instruction    
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

    // Jump to end of the k-loop for incrementing later. (NOT NEEDED IN PYTHON!)
    b k_loop_end
k_loop_2:
    // Ai = idx of A[r * A_c + k] = A + 2r * A_c + 2k
    add x12, x12, #8         // Ai += 8 (k=4; 2k=8)
    // Bi = idx of B[k * B_c + c] = B + 2k * B_c + 2c
    add x13, x13, x5, lsl #2 // Bi += 2B_c << 2 (k=4; 2k=8)

    // Initialize A0-A3
    // ld1 {v0.4h, v1.4h, v2.4h, v3.4h}, [x12], x4
    ld1 {v0.4h}, [x12]
    add x15, x12, x4
    ld1 {v1.4h}, [x15]
    add x15, x15, x4
    ld1 {v2.4h}, [x15]
    add x15, x15, x4
    ld1 {v3.4h}, [x15]

    // Load B0-B3
    // ld1 {v4.4h, v5.4h, v6.4h, v7.4h}, [x13], x5
    ld1 {v4.4h}, [x13]
    add x15, x13, x5
    ld1 {v5.4h}, [x15]
    add x15, x15, x5
    ld1 {v6.4h}, [x15]
    add x15, x15, x5
    ld1 {v7.4h}, [x15]
    
    // Compute C0
    umlal v8.4s, v4.4h, v0.4h[0] // C0 = C0 + B0 * A[A0 + 0]
    umlal v8.4s, v5.4h, v0.4h[1] // C0 = C0 + B1 * A[A0 + 1]
    umlal v8.4s, v6.4h, v0.4h[2] // C0 = C0 + B2 * A[A0 + 2]
    umlal v8.4s, v7.4h, v0.4h[3] // C0 = C0 + B3 * A[A0 + 3]

    // Compute C1
    umlal v9.4s, v4.4h, v1.4h[0] // C1 = C1 + B0 * A[A1 + 0]
    umlal v9.4s, v5.4h, v1.4h[1] // C1 = C1 + B1 * A[A1 + 1]
    umlal v9.4s, v6.4h, v1.4h[2] // C1 = C1 + B2 * A[A1 + 2]
    umlal v9.4s, v7.4h, v1.4h[3] // C1 = C1 + B3 * A[A1 + 3]

    // Compute C2
    umlal v10.4s, v4.4h, v2.4h[0] // C2 = C2 + B0 * A[A2 + 0]
    umlal v10.4s, v5.4h, v2.4h[1] // C2 = C2 + B1 * A[A2 + 1]
    umlal v10.4s, v6.4h, v2.4h[2] // C2 = C2 + B2 * A[A2 + 2]
    umlal v10.4s, v7.4h, v2.4h[3] // C2 = C2 + B3 * A[A2 + 3]

    // Compute C3
    umlal v11.4s, v4.4h, v3.4h[0] // C3 = C3 + B0 * A[A3 + 0]
    umlal v11.4s, v5.4h, v3.4h[1] // C3 = C3 + B1 * A[A3 + 1]
    umlal v11.4s, v6.4h, v3.4h[2] // C3 = C3 + B2 * A[A3 + 2]
    umlal v11.4s, v7.4h, v3.4h[3] // C3 = C3 + B3 * A[A3 + 3]
k_loop_end:
    // Increment k and branch if k < A_c
    add x11, x11, #4
    cmp x11, x4, lsr #1
    blt k_loop_2
    
    // Once we are done with the k-loop, we need to reduce C0-C3 modulo 4093 and store the result in C

    // C0 = C0 % 4093
    ushr v12.4s, v8.4s, #12
    mul v12.4s, v12.4s, v16.4s
    sub v8.4s, v8.4s, v12.4s
    ushr v12.4s, v8.4s, #12
    mul v12.4s, v12.4s, v16.4s
    sub v8.4s, v8.4s, v12.4s
    sqxtn v8.4h, v8.4s
    // C1 = C1 % 4093
    ushr v13.4s, v9.4s, #12
    mul v13.4s, v13.4s, v16.4s
    sub v9.4s, v9.4s, v13.4s
    ushr v13.4s, v9.4s, #12
    mul v13.4s, v13.4s, v16.4s
    sub v9.4s, v9.4s, v13.4s
    sqxtn v9.4h, v9.4s
    // C2 = C2 % 4093
    ushr v14.4s, v10.4s, #12
    mul v14.4s, v14.4s, v16.4s
    sub v10.4s, v10.4s, v14.4s
    ushr v14.4s, v10.4s, #12
    mul v14.4s, v14.4s, v16.4s
    sub v10.4s, v10.4s, v14.4s
    sqxtn v10.4h, v10.4s
    // C3 = C3 % 4093
    ushr v15.4s, v11.4s, #12
    mul v15.4s, v15.4s, v16.4s
    sub v11.4s, v11.4s, v15.4s
    ushr v15.4s, v11.4s, #12
    mul v15.4s, v15.4s, v16.4s
    sub v11.4s, v11.4s, v15.4s
    sqxtn v11.4h, v11.4s

    // Ci = idx of C[C_c * r + c] = C + 2 * (r * C_c + c) = C + 2rC_c + 2c
    add x14, x0, x10, lsl #1 // Ci = C + 2c

    // Store C0-C3
    // st1 {v8.4h, v9.4h, v10.4h, v11.4h}, [x14], x5
    st1 {v8.4h}, [x14], x5
    st1 {v9.4h}, [x14], x5
    st1 {v10.4h}, [x14], x5
    st1 {v11.4h}, [x14]
c_loop_end:
    // Increment c and branch if c < C_c
    add x10, x10, #4
    cmp x10, x5, lsr #1
    blt c_loop
r_loop_end:
    // Increment r and branch if r < C_r
    add x9, x9, #4
    // Set Ci = C + 2rC_c; add 2rC_c to C
    add x0, x0, x5, lsl #2 

    cmp x9, x3, lsr #1
    blt r_loop
end:
    ret
