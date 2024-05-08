.cpu cortex-a72
.arch armv8-a
.global pmod_mat_syst_m_m_m_0_1
pmod_mat_syst_m_m_m_0_1:
    mov x1, #34
    mov x3, #4093
    dup v16.4h, w3
    dup v17.8h, w3
    dup v18.4s, w3
    mov x2, xzr
    mov x4, #0
elimination_loop:
    cmp x4, #34
    b.eq elimination_loop_end
    madd x7, x1, x4, x4
    add x5, x4, #1
elimination_row_zero_fix_outer_loop:
    cmp x5, 34
    b.eq elimination_row_zero_fix_outer_loop_end
    madd x9, x1, x5, x4
    add x9, x0, x9, lsl #1
    ldrh w10, [x0, x7, lsl #1]
    add x8, x0, x7, lsl #1
    dup v1.4h, w10
    dup v2.8h, w10
    cmeq v5.4h, v1.4h, #0
    cmeq v6.8h, v2.8h, #0
    mov x6, x4
elimination_row_zero_fix_inner_loop_neon_16x8:
    sub x14, x1, x6
    cmp x14, #8
    b.lt elimination_row_zero_fix_inner_loop_neon_16x4
    ld1 {v3.8h}, [x8]
    ld1 {v4.8h}, [x9], #16
    and v19.16b, v6.16b, v4.16b
    add v20.8h, v19.8h, v3.8h
    cmhs v19.8h, v20.8h, v17.8h
    and v19.16b, v19.16b, v17.16b
    sub v20.8h, v20.8h, v19.8h
    st1 {v20.8h}, [x8], #16
    add x6, x6, #8
    b elimination_row_zero_fix_inner_loop_neon_16x8
elimination_row_zero_fix_inner_loop_neon_16x4:
    sub x14, x1, x6
    cmp x14, #4
    b.lt elimination_row_zero_fix_inner_loop_scalar
    ld1 {v3.4h}, [x8]
    ld1 {v4.4h}, [x9], #8
    and v19.16b, v5.16b, v4.16b
    add v20.4h, v19.4h, v3.4h
    cmhs v19.4h, v20.4h, v16.4h
    and v19.16b, v19.16b, v16.16b
    sub v20.4h, v20.4h, v19.4h
    st1 {v20.4h}, [x8], #8
    add x6, x6, #4
elimination_row_zero_fix_inner_loop_scalar:
    cmp x6, x1
    b.ge elimination_row_zero_fix_inner_loop_end
    ldrh w11, [x8]
    ldrh w12, [x9], #2
    cmp x10, #0
    csel x14, x12, xzr, eq
    add x14, x14, x11
    cmp x14, #4093
    csel x11, x3, xzr, ge
    sub x14, x14, x11
    strh w14, [x8], #2
    add x6, x6, #1
    b elimination_row_zero_fix_inner_loop_scalar
elimination_row_zero_fix_inner_loop_end:
    add x5, x5, #1
    b elimination_row_zero_fix_outer_loop
elimination_row_zero_fix_outer_loop_end:
    ldrh w10, [x0, x7, lsl #1]
    cmp x10, #0
    b.eq ret_fail
    mul x14, x10, x10
    lsr x6, x14, #12
    mul x6, x6, x3
    sub x14, x14, x6
    lsr x6, x14, #12
    mul x6, x6, x3
    sub x14, x14, x6
    mul x14, x14, x14
    lsr x6, x14, #12
    mul x6, x6, x3
    sub x14, x14, x6
    lsr x6, x14, #12
    mul x6, x6, x3
    sub x14, x14, x6
    mul x15, x14, x10
    lsr x6, x15, #12
    mul x6, x6, x3
    sub x15, x15, x6
    lsr x6, x15, #12
    mul x6, x6, x3
    sub x15, x15, x6
    mul x14, x15, x15
    lsr x6, x14, #12
    mul x6, x6, x3
    sub x14, x14, x6
    lsr x6, x14, #12
    mul x6, x6, x3
    sub x14, x14, x6
    mul x5, x14, x15
    lsr x6, x5, #12
    mul x6, x6, x3
    sub x5, x5, x6
    lsr x6, x5, #12
    mul x6, x6, x3
    sub x5, x5, x6
    mul x14, x5, x5
    lsr x6, x14, #12
    mul x6, x6, x3
    sub x14, x14, x6
    lsr x6, x14, #12
    mul x6, x6, x3
    sub x14, x14, x6
    mul x14, x14, x14
    lsr x6, x14, #12
    mul x6, x6, x3
    sub x14, x14, x6
    lsr x6, x14, #12
    mul x6, x6, x3
    sub x14, x14, x6
    mul x14, x14, x14
    lsr x6, x14, #12
    mul x6, x6, x3
    sub x14, x14, x6
    lsr x6, x14, #12
    mul x6, x6, x3
    sub x14, x14, x6
    mul x14, x14, x14
    lsr x6, x14, #12
    mul x6, x6, x3
    sub x14, x14, x6
    lsr x6, x14, #12
    mul x6, x6, x3
    sub x14, x14, x6
    mul x14, x14, x5
    lsr x6, x14, #12
    mul x6, x6, x3
    sub x14, x14, x6
    lsr x6, x14, #12
    mul x6, x6, x3
    sub x14, x14, x6
    mul x14, x14, x14
    lsr x6, x14, #12
    mul x6, x6, x3
    sub x14, x14, x6
    lsr x6, x14, #12
    mul x6, x6, x3
    sub x14, x14, x6
    mul x14, x14, x14
    lsr x6, x14, #12
    mul x6, x6, x3
    sub x14, x14, x6
    lsr x6, x14, #12
    mul x6, x6, x3
    sub x14, x14, x6
    mul x14, x14, x14
    lsr x6, x14, #12
    mul x6, x6, x3
    sub x14, x14, x6
    lsr x6, x14, #12
    mul x6, x6, x3
    sub x14, x14, x6
    mul x14, x14, x15
    lsr x6, x14, #12
    mul x6, x6, x3
    sub x14, x14, x6
    lsr x6, x14, #12
    mul x6, x6, x3
    sub x14, x14, x6
    mul x14, x14, x14
    lsr x6, x14, #12
    mul x6, x6, x3
    sub x14, x14, x6
    lsr x6, x14, #12
    mul x6, x6, x3
    sub x14, x14, x6
    mul x14, x14, x10
    lsr x6, x14, #12
    mul x6, x6, x3
    sub x14, x14, x6
    lsr x6, x14, #12
    mul x6, x6, x3
    sub x14, x14, x6
    cmp x14, #4093
    csel x6, x3, xzr, ge
    sub x10, x14, x6
    dup v1.4h, w10
    add x8, x0, x7, lsl #1
    mov x6, x4
elimination_normalize_row_loop_neon_16x4:
    sub x14, x1, x6
    cmp x14, #4
    b.lt elimination_normalize_row_loop_scalar
    ld1 {v3.4h}, [x8]
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
    st1 {v3.4h}, [x8], #8
    add x6, x6, #4
    b elimination_normalize_row_loop_neon_16x4
elimination_normalize_row_loop_scalar:
    cmp x6, x1
    b.ge elimination_normalize_row_loop_end
    ldrh w11, [x8]
    mul x14, x11, x10
    lsr x15, x14, #12
    mul x15, x15, x3
    sub x14, x14, x15
    lsr x15, x14, #12
    mul x15, x15, x3
    sub x11, x14, x15
    cmp x11, #4093
    csel x15, x3, xzr, ge
    sub x11, x11, x15
    strh w11, [x8], #2
    add x6, x6, #1
    b elimination_normalize_row_loop_scalar
elimination_normalize_row_loop_end:
    add x5, x4, #1
elimination_eliminate_rows_loop:
    cmp x5, 34
    b.eq elimination_eliminate_rows_loop_end
    madd x9, x1, x5, x4
    add x9, x0, x9, lsl #1
    ldrh w13, [x9]
    add x8, x0, x7, lsl #1
    dup v1.4h, w10
    dup v7.4h, w13
    mov x6, x4
elimination_row_eliminate_inner_loop_neon_16x4:
    sub x14, x1, x6
    cmp x14, #4
    b.lt elimination_row_eliminate_inner_loop_scalar
    ld1 {v3.4h}, [x8], #8
    ld1 {v4.4h}, [x9]
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
    st1 {v4.4h}, [x9], #8
    add x6, x6, #4
    b elimination_row_eliminate_inner_loop_neon_16x4
elimination_row_eliminate_inner_loop_scalar:
    cmp x6, x1
    b.ge elimination_row_eliminate_inner_loop_end
    ldrh w11, [x8], #2
    ldrh w12, [x9]
    mul x14, x11, x13
    lsr x15, x14, #12
    mul x15, x15, x3
    sub x14, x14, x15
    lsr x15, x14, #12
    mul x15, x15, x3
    sub x14, x14, x15
    cmp x14, #4093
    csel x15, x3, xzr, ge
    sub x14, x14, x15
    add x15, x12, #4093
    sub x15, x15, x14
    cmp x15, #4093
    csel x14, x3, xzr, ge
    sub x12, x15, x14
    strh w12, [x9], #2
    add x6, x6, #1
    b elimination_row_eliminate_inner_loop_scalar
elimination_row_eliminate_inner_loop_end:
    add x5, x5, #1
    b elimination_eliminate_rows_loop
elimination_eliminate_rows_loop_end:
    add x4, x4, #1
    b elimination_loop
elimination_loop_end:
    mov x4, #33
backsub_outer_loop:
    cmp x4, #0
    b.lt backsub_outer_loop_end
    mov x5, #0
backsub_inner_loop:
    cmp x5, x4
    b.eq backsub_inner_loop_end
    madd x14, x1, x5, x4
    ldrh w13, [x0, x14, lsl #1]
    dup v7.4h, w13
    madd x14, x1, x4, x4
    ldrh w11, [x0, x14, lsl #1]
    madd x9, x1, x5, x4
    ldrh w12, [x0, x9, lsl #1]
    mul x14, x11, x13
    lsr x15, x14, #12
    mul x15, x15, x3
    sub x14, x14, x15
    lsr x15, x14, #12
    mul x15, x15, x3
    sub x14, x14, x15
    cmp x14, #4093
    csel x15, x3, xzr, ge
    sub x14, x14, x15
    add x15, x12, #4093
    sub x15, x15, x14
    cmp x15, #4093
    csel x14, x3, xzr, ge
    sub x12, x15, x14
    strh w12, [x0, x9, lsl #1]
    mov x6, #34
    madd x8, x1, x4, x6
    add x8, x0, x8, lsl #1
    madd x9, x1, x5, x6
    add x9, x0, x9, lsl #1
backsub_column_loop_neon_16x4:
    sub x14, x1, x6
    cmp x14, #4
    b.lt backsub_column_loop_scalar
    ld1 {v3.4h}, [x8], #8
    ld1 {v4.4h}, [x9]
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
    st1 {v4.4h}, [x9], #8
    add x6, x6, #4
    b backsub_column_loop_neon_16x4
backsub_column_loop_scalar:
    cmp x6, x1
    b.ge backsub_column_loop_end
    ldrh w11, [x8], #2
    ldrh w12, [x9]
    mul x14, x11, x13
    lsr x15, x14, #12
    mul x15, x15, x3
    sub x14, x14, x15
    lsr x15, x14, #12
    mul x15, x15, x3
    sub x14, x14, x15
    cmp x14, #4093
    csel x15, x3, xzr, ge
    sub x14, x14, x15
    add x15, x12, #4093
    sub x15, x15, x14
    cmp x15, #4093
    csel x14, x3, xzr, ge
    sub x12, x15, x14
    strh w12, [x9], #2
    add x6, x6, #1
    b backsub_column_loop_scalar
backsub_column_loop_end:
    add x5, x5, #1
    b backsub_inner_loop
backsub_inner_loop_end:
    sub x4, x4, #1
    b backsub_outer_loop
backsub_outer_loop_end:
ret_success:
    mov x0, x2
    ret
ret_fail:
    mov x0, #-1
    ret
