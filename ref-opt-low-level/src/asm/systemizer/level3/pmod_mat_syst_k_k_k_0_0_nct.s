.cpu cortex-a72
.arch armv8-a
.global pmod_mat_syst_k_k_k_0_0_nct
pmod_mat_syst_k_k_k_0_0_nct:
    mov x2, #34
    mov w4, 0x0481
    movk w4, 0x8018, lsl #16
    dup v25.4s, w4
    mov x4, #4093
    dup v16.4h, w4
    dup v17.8h, w4
    dup v18.4s, w4
    mov x3, xzr
    mov x5, #0
elimination_loop:
    cmp x5, #34
    b.eq elimination_loop_end
    madd x8, x2, x5, x5
    ldrh w11, [x0, x8, lsl #1]
    cmp x11, #0
    b.ne elimination_loop_post_zero_fix
    add x6, x5, #1
elimination_row_zero_fix_outer_loop:
    cmp x6, 34
    b.eq elimination_row_zero_fix_outer_loop_end
    madd x10, x2, x6, x5
    add x10, x0, x10, lsl #1
    ldrh w11, [x0, x8, lsl #1]
    add x9, x0, x8, lsl #1
    dup v1.4h, w11
    dup v2.8h, w11
    cmeq v5.4h, v1.4h, #0
    cmeq v6.8h, v2.8h, #0
    mov x7, x5
elimination_row_zero_fix_inner_loop_neon_16x8:
    sub x15, x2, x7
    cmp x15, #8
    b.lt elimination_row_zero_fix_inner_loop_neon_16x4
    ld1 {v3.8h}, [x9]
    ld1 {v4.8h}, [x10], #16
    and v19.16b, v6.16b, v4.16b
    add v20.8h, v19.8h, v3.8h
    cmhs v19.8h, v20.8h, v17.8h
    and v19.16b, v19.16b, v17.16b
    sub v20.8h, v20.8h, v19.8h
    st1 {v20.8h}, [x9], #16
    add x7, x7, #8
    b elimination_row_zero_fix_inner_loop_neon_16x8
elimination_row_zero_fix_inner_loop_neon_16x4:
    sub x15, x2, x7
    cmp x15, #4
    b.lt elimination_row_zero_fix_inner_loop_scalar
    ld1 {v3.4h}, [x9]
    ld1 {v4.4h}, [x10], #8
    and v19.16b, v5.16b, v4.16b
    add v20.4h, v19.4h, v3.4h
    cmhs v19.4h, v20.4h, v16.4h
    and v19.16b, v19.16b, v16.16b
    sub v20.4h, v20.4h, v19.4h
    st1 {v20.4h}, [x9], #8
    add x7, x7, #4
elimination_row_zero_fix_inner_loop_scalar:
    cmp x7, x2
    b.ge elimination_row_zero_fix_inner_loop_end
    ldrh w12, [x9]
    ldrh w13, [x10], #2
    cmp x11, #0
    csel x15, x13, xzr, eq
    add x15, x15, x12
    cmp x15, #4093
    csel x12, x4, xzr, ge
    sub x15, x15, x12
    strh w15, [x9], #2
    add x7, x7, #1
    b elimination_row_zero_fix_inner_loop_scalar
elimination_row_zero_fix_inner_loop_end:
    add x6, x6, #1
    b elimination_row_zero_fix_outer_loop
elimination_row_zero_fix_outer_loop_end:
    ldrh w11, [x0, x8, lsl #1]
    cmp x11, #0
    b.eq ret_fail
    elimination_loop_post_zero_fix:
    ldrh w11, [x1, x11, lsl #1]
    dup v1.4h, w11
    add x9, x0, x8, lsl #1
    mov x7, x5
elimination_normalize_row_loop_neon_16x4:
    sub x15, x2, x7
    cmp x15, #4
    b.lt elimination_normalize_row_loop_scalar
    ld1 {v3.4h}, [x9]
    umull v19.4s, v3.4h, v1.4h
    umull v23.2d, v19.2s, v25.2s
    umull2 v24.2d, v19.4s, v25.4s
    uzp2 v23.4s, v23.4s, v24.4s
    ushr v23.4s, v23.4s, 11
    mls v19.4s, v23.4s, v18.4s
    xtn v19.4h, v19.4s
    st1 {v19.4h}, [x9], #8
    add x7, x7, #4
    b elimination_normalize_row_loop_neon_16x4
elimination_normalize_row_loop_scalar:
    cmp x7, x2
    b.ge elimination_normalize_row_loop_end
    ldrh w12, [x9]
    mul x15, x12, x11
    lsr x16, x15, #12
    mul x16, x16, x4
    sub x15, x15, x16
    lsr x16, x15, #12
    mul x16, x16, x4
    sub x12, x15, x16
    cmp x12, #4093
    csel x16, x4, xzr, ge
    sub x12, x12, x16
    strh w12, [x9], #2
    add x7, x7, #1
    b elimination_normalize_row_loop_scalar
elimination_normalize_row_loop_end:
    add x6, x5, #1
elimination_eliminate_rows_loop:
    cmp x6, 34
    b.eq elimination_eliminate_rows_loop_end
    madd x10, x2, x6, x5
    add x10, x0, x10, lsl #1
    ldrh w14, [x10]
    add x9, x0, x8, lsl #1
    dup v1.4h, w11
    dup v7.4h, w14
    mov x7, x5
elimination_row_eliminate_inner_loop_neon_16x4:
    sub x15, x2, x7
    cmp x15, #4
    b.lt elimination_row_eliminate_inner_loop_scalar
    ld1 {v3.4h}, [x9], #8
    ld1 {v4.4h}, [x10]
    umull v19.4s, v3.4h, v7.4h
    umull v20.2d, v19.2s, v25.2s
    umull2 v21.2d, v19.4s, v25.4s
    uzp2 v20.4s, v20.4s, v21.4s
    ushr v20.4s, v20.4s, 11
    mls v19.4s, v20.4s, v18.4s
    xtn v19.4h, v19.4s
    add v20.4h, v4.4h, v16.4h
    sub v20.4h, v20.4h, v19.4h
    cmhs v19.4h, v20.4h, v16.4h
    and v19.16b, v19.16b, v16.16b
    sub v4.4h, v20.4h, v19.4h
    st1 {v4.4h}, [x10], #8
    add x7, x7, #4
    b elimination_row_eliminate_inner_loop_neon_16x4
elimination_row_eliminate_inner_loop_scalar:
    cmp x7, x2
    b.ge elimination_row_eliminate_inner_loop_end
    ldrh w12, [x9], #2
    ldrh w13, [x10]
    mul x15, x12, x14
    lsr x16, x15, #12
    mul x16, x16, x4
    sub x15, x15, x16
    lsr x16, x15, #12
    mul x16, x16, x4
    sub x15, x15, x16
    cmp x15, #4093
    csel x16, x4, xzr, ge
    sub x15, x15, x16
    add x16, x13, #4093
    sub x16, x16, x15
    cmp x16, #4093
    csel x15, x4, xzr, ge
    sub x13, x16, x15
    strh w13, [x10], #2
    add x7, x7, #1
    b elimination_row_eliminate_inner_loop_scalar
elimination_row_eliminate_inner_loop_end:
    add x6, x6, #1
    b elimination_eliminate_rows_loop
elimination_eliminate_rows_loop_end:
    add x5, x5, #1
    b elimination_loop
elimination_loop_end:
ret_success:
    mov x0, x3
    ret
ret_fail:
    mov x0, #-1
    ret
