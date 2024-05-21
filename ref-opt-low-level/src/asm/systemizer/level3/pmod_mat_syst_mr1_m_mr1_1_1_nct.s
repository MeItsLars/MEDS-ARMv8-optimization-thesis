.cpu cortex-a72
.arch armv8-a
.global pmod_mat_syst_mr1_m_mr1_1_1_nct
pmod_mat_syst_mr1_m_mr1_1_1_nct:
    mov x2, #34
    mov x4, #4093
    dup v16.4h, w4
    dup v17.8h, w4
    dup v18.4s, w4
    mov x3, #33
    mov x5, #0
elimination_loop:
    cmp x5, #33
    b.eq elimination_loop_end
    madd x8, x2, x5, x5
    ldrh w11, [x0, x8, lsl #1]
    cmp x11, #0
    b.ne elimination_loop_post_zero_fix
    mov x11, #68
    add x6, x0, x8, lsl #1
    mov x15, #0
    mov x7, x5
elimination_swap_or_loop:
    cmp x7, #33
    b.ge elimination_swap_or_loop_end
    ldrh w14, [x6], #68
    orr x15, x15, x14
    add x7, x7, #1
    b elimination_swap_or_loop
elimination_swap_or_loop_end:
    cmp x15, #0
    csel x3, x3, x5, ne
    add x9, x0, x5, lsl #1
    add x10, x0, 66
    mov x6, #0
elimination_swap_loop:
    cmp x6, #33
    b.ge elimination_swap_loop_end
    ldrh w12, [x9]
    ldrh w13, [x10]
    cmp x15, #0
    csel x16, x12, x13, ne
    csel w13, w13, w12, ne
    mov x12, x16
    strh w12, [x9], #68
    strh w13, [x10], #68
    add x6, x6, #1
    b elimination_swap_loop
elimination_swap_loop_end:
    add x6, x5, #1
elimination_row_zero_fix_outer_loop:
    cmp x6, 33
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
    ushr v20.4s, v19.4s, #12
    mul v20.4s, v20.4s, v18.4s
    sub v19.4s, v19.4s, v20.4s
    ushr v20.4s, v19.4s, #12
    mul v20.4s, v20.4s, v18.4s
    sub v19.4s, v19.4s, v20.4s
    sqxtn v3.4h, v19.4s
    cmhs v20.4h, v3.4h, v16.4h
    and v20.16b, v20.16b, v16.16b
    sub v3.4h, v3.4h, v20.4h
    st1 {v3.4h}, [x9], #8
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
    cmp x6, 33
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
    ushr v20.4s, v19.4s, #12
    mul v20.4s, v20.4s, v18.4s
    sub v19.4s, v19.4s, v20.4s
    ushr v20.4s, v19.4s, #12
    mul v20.4s, v20.4s, v18.4s
    sub v19.4s, v19.4s, v20.4s
    sqxtn v19.4h, v19.4s
    cmhs v20.4h, v19.4h, v16.4h
    and v20.16b, v20.16b, v16.16b
    sub v19.4h, v19.4h, v20.4h
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
    mov x5, #32
backsub_outer_loop:
    cmp x5, #0
    b.lt backsub_outer_loop_end
    mov x6, #0
backsub_inner_loop:
    cmp x6, x5
    b.eq backsub_inner_loop_end
    madd x15, x2, x6, x5
    ldrh w14, [x0, x15, lsl #1]
    dup v7.4h, w14
    madd x15, x2, x5, x5
    ldrh w12, [x0, x15, lsl #1]
    madd x10, x2, x6, x5
    ldrh w13, [x0, x10, lsl #1]
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
    strh w13, [x0, x10, lsl #1]
    mov x7, #33
    madd x9, x2, x5, x7
    add x9, x0, x9, lsl #1
    madd x10, x2, x6, x7
    add x10, x0, x10, lsl #1
backsub_column_loop_neon_16x4:
    sub x15, x2, x7
    cmp x15, #4
    b.lt backsub_column_loop_scalar
    ld1 {v3.4h}, [x9], #8
    ld1 {v4.4h}, [x10]
    umull v19.4s, v3.4h, v7.4h
    ushr v20.4s, v19.4s, #12
    mul v20.4s, v20.4s, v18.4s
    sub v19.4s, v19.4s, v20.4s
    ushr v20.4s, v19.4s, #12
    mul v20.4s, v20.4s, v18.4s
    sub v19.4s, v19.4s, v20.4s
    sqxtn v19.4h, v19.4s
    cmhs v20.4h, v19.4h, v16.4h
    and v20.16b, v20.16b, v16.16b
    sub v19.4h, v19.4h, v20.4h
    add v20.4h, v4.4h, v16.4h
    sub v20.4h, v20.4h, v19.4h
    cmhs v19.4h, v20.4h, v16.4h
    and v19.16b, v19.16b, v16.16b
    sub v4.4h, v20.4h, v19.4h
    st1 {v4.4h}, [x10], #8
    add x7, x7, #4
    b backsub_column_loop_neon_16x4
backsub_column_loop_scalar:
    cmp x7, x2
    b.ge backsub_column_loop_end
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
    b backsub_column_loop_scalar
backsub_column_loop_end:
    add x6, x6, #1
    b backsub_inner_loop
backsub_inner_loop_end:
    sub x5, x5, #1
    b backsub_outer_loop
backsub_outer_loop_end:
ret_success:
    mov x0, x3
    ret
ret_fail:
    mov x0, #-1
    ret
