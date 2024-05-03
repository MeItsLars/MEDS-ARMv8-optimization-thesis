.cpu cortex-a72
.arch armv8-a
.global pmod_mat_syst_test
pmod_mat_syst_test:
    mov x3, x1
    mov w11, #4093
    dup v1.4h, w11
    dup v13.4s, w11
    dup v2.4h, wzr
    mov w11, #-1
    dup v3.4h, w11
    dup v0.4h, wzr
    mov x4, #0
elimination_loop:
    cmp x4, x3
    b.eq elimination_loop_end
    madd x11, x2, x4, x4
    ldr d4, [x0, x11, lsl #3]
    madd x7, x2, x4, x4
    add x7, x0, x7, lsl #3
    cmeq v5.4h, v4.4h, v2.4h
    add x5, x4, #1
elimination_row_zero_fix_outer_loop:
    cmp x5, x1
    b.eq elimination_row_zero_fix_outer_loop_end
    madd x8, x2, x5, x4
    add x8, x0, x8, lsl #3
    mov x6, x4
elimination_row_zero_fix_inner_loop:
    cmp x6, x2
    b.eq elimination_row_zero_fix_inner_loop_end
    ldr d6, [x7]
    ldr d7, [x8], #8
    and v16.16b, v5.16b, v7.16b
    uaddl v17.4s, v16.4h, v6.4h
    ushr v16.4s, v17.4s, #12
    mul v16.4s, v16.4s, v13.4s
    sub v17.4s, v17.4s, v16.4s
    ushr v16.4s, v17.4s, #12
    mul v16.4s, v16.4s, v13.4s
    sub v17.4s, v17.4s, v16.4s
    sqxtn v17.4h, v17.4s
    cmhs v16.4h, v17.4h, v1.4h
    and v16.16b, v16.16b, v1.16b
    sub v17.4h, v17.4h, v16.4h
    str d17, [x7], #8
    add x6, x6, #1
    b elimination_row_zero_fix_inner_loop
elimination_row_zero_fix_inner_loop_end:
    add x5, x5, #1
    b elimination_row_zero_fix_outer_loop
elimination_row_zero_fix_outer_loop_end:
    cmeq v16.4h, v4.4h, #0
    and v17.16b, v16.16b, v3.16b
    mvn v16.16b, v16.16b
    and v16.16b, v16.16b, v0.16b
    orr v0.16b, v16.16b, v17.16b
    umull v19.4s, v4.4h, v4.4h
    ushr v20.4s, v19.4s, #12
    mul v20.4s, v20.4s, v13.4s
    sub v19.4s, v19.4s, v20.4s
    ushr v20.4s, v19.4s, #12
    mul v20.4s, v20.4s, v13.4s
    sub v19.4s, v19.4s, v20.4s
    sqxtn v16.4h, v19.4s
    umull v19.4s, v16.4h, v16.4h
    ushr v20.4s, v19.4s, #12
    mul v20.4s, v20.4s, v13.4s
    sub v19.4s, v19.4s, v20.4s
    ushr v20.4s, v19.4s, #12
    mul v20.4s, v20.4s, v13.4s
    sub v19.4s, v19.4s, v20.4s
    sqxtn v16.4h, v19.4s
    umull v19.4s, v16.4h, v4.4h
    ushr v20.4s, v19.4s, #12
    mul v20.4s, v20.4s, v13.4s
    sub v19.4s, v19.4s, v20.4s
    ushr v20.4s, v19.4s, #12
    mul v20.4s, v20.4s, v13.4s
    sub v19.4s, v19.4s, v20.4s
    sqxtn v17.4h, v19.4s
    umull v19.4s, v17.4h, v17.4h
    ushr v20.4s, v19.4s, #12
    mul v20.4s, v20.4s, v13.4s
    sub v19.4s, v19.4s, v20.4s
    ushr v20.4s, v19.4s, #12
    mul v20.4s, v20.4s, v13.4s
    sub v19.4s, v19.4s, v20.4s
    sqxtn v16.4h, v19.4s
    umull v19.4s, v16.4h, v17.4h
    ushr v20.4s, v19.4s, #12
    mul v20.4s, v20.4s, v13.4s
    sub v19.4s, v19.4s, v20.4s
    ushr v20.4s, v19.4s, #12
    mul v20.4s, v20.4s, v13.4s
    sub v19.4s, v19.4s, v20.4s
    sqxtn v18.4h, v19.4s
    umull v19.4s, v18.4h, v18.4h
    ushr v20.4s, v19.4s, #12
    mul v20.4s, v20.4s, v13.4s
    sub v19.4s, v19.4s, v20.4s
    ushr v20.4s, v19.4s, #12
    mul v20.4s, v20.4s, v13.4s
    sub v19.4s, v19.4s, v20.4s
    sqxtn v16.4h, v19.4s
    umull v19.4s, v16.4h, v16.4h
    ushr v20.4s, v19.4s, #12
    mul v20.4s, v20.4s, v13.4s
    sub v19.4s, v19.4s, v20.4s
    ushr v20.4s, v19.4s, #12
    mul v20.4s, v20.4s, v13.4s
    sub v19.4s, v19.4s, v20.4s
    sqxtn v16.4h, v19.4s
    umull v19.4s, v16.4h, v16.4h
    ushr v20.4s, v19.4s, #12
    mul v20.4s, v20.4s, v13.4s
    sub v19.4s, v19.4s, v20.4s
    ushr v20.4s, v19.4s, #12
    mul v20.4s, v20.4s, v13.4s
    sub v19.4s, v19.4s, v20.4s
    sqxtn v16.4h, v19.4s
    umull v19.4s, v16.4h, v16.4h
    ushr v20.4s, v19.4s, #12
    mul v20.4s, v20.4s, v13.4s
    sub v19.4s, v19.4s, v20.4s
    ushr v20.4s, v19.4s, #12
    mul v20.4s, v20.4s, v13.4s
    sub v19.4s, v19.4s, v20.4s
    sqxtn v16.4h, v19.4s
    umull v19.4s, v16.4h, v18.4h
    ushr v20.4s, v19.4s, #12
    mul v20.4s, v20.4s, v13.4s
    sub v19.4s, v19.4s, v20.4s
    ushr v20.4s, v19.4s, #12
    mul v20.4s, v20.4s, v13.4s
    sub v19.4s, v19.4s, v20.4s
    sqxtn v16.4h, v19.4s
    umull v19.4s, v16.4h, v16.4h
    ushr v20.4s, v19.4s, #12
    mul v20.4s, v20.4s, v13.4s
    sub v19.4s, v19.4s, v20.4s
    ushr v20.4s, v19.4s, #12
    mul v20.4s, v20.4s, v13.4s
    sub v19.4s, v19.4s, v20.4s
    sqxtn v16.4h, v19.4s
    umull v19.4s, v16.4h, v16.4h
    ushr v20.4s, v19.4s, #12
    mul v20.4s, v20.4s, v13.4s
    sub v19.4s, v19.4s, v20.4s
    ushr v20.4s, v19.4s, #12
    mul v20.4s, v20.4s, v13.4s
    sub v19.4s, v19.4s, v20.4s
    sqxtn v16.4h, v19.4s
    umull v19.4s, v16.4h, v16.4h
    ushr v20.4s, v19.4s, #12
    mul v20.4s, v20.4s, v13.4s
    sub v19.4s, v19.4s, v20.4s
    ushr v20.4s, v19.4s, #12
    mul v20.4s, v20.4s, v13.4s
    sub v19.4s, v19.4s, v20.4s
    sqxtn v16.4h, v19.4s
    umull v19.4s, v16.4h, v17.4h
    ushr v20.4s, v19.4s, #12
    mul v20.4s, v20.4s, v13.4s
    sub v19.4s, v19.4s, v20.4s
    ushr v20.4s, v19.4s, #12
    mul v20.4s, v20.4s, v13.4s
    sub v19.4s, v19.4s, v20.4s
    sqxtn v16.4h, v19.4s
    umull v19.4s, v16.4h, v16.4h
    ushr v20.4s, v19.4s, #12
    mul v20.4s, v20.4s, v13.4s
    sub v19.4s, v19.4s, v20.4s
    ushr v20.4s, v19.4s, #12
    mul v20.4s, v20.4s, v13.4s
    sub v19.4s, v19.4s, v20.4s
    sqxtn v16.4h, v19.4s
    umull v19.4s, v16.4h, v4.4h
    ushr v20.4s, v19.4s, #12
    mul v20.4s, v20.4s, v13.4s
    sub v19.4s, v19.4s, v20.4s
    ushr v20.4s, v19.4s, #12
    mul v20.4s, v20.4s, v13.4s
    sub v19.4s, v19.4s, v20.4s
    sqxtn v16.4h, v19.4s
    cmhs v19.4h, v16.4h, v1.4h
    and v19.16b, v19.16b, v1.16b
    sub v4.4h, v16.4h, v19.4h
    madd x7, x2, x4, x4
    add x7, x0, x7, lsl #3
    mov x5, x4
elimination_normalize_row_loop:
    cmp x5, x2
    b.eq elimination_normalize_row_loop_end
    ldr d6, [x7]
    umull v16.4s, v6.4h, v4.4h
    ushr v17.4s, v16.4s, #12
    mul v17.4s, v17.4s, v13.4s
    sub v16.4s, v16.4s, v17.4s
    ushr v17.4s, v16.4s, #12
    mul v17.4s, v17.4s, v13.4s
    sub v16.4s, v16.4s, v17.4s
    sqxtn v6.4h, v16.4s
    cmhs v17.4h, v6.4h, v1.4h
    and v17.16b, v17.16b, v1.16b
    sub v6.4h, v6.4h, v17.4h
    str d6, [x7], #8
    add x5, x5, #1
    b elimination_normalize_row_loop
elimination_normalize_row_loop_end:
    add x5, x4, #1
elimination_eliminate_rows_loop:
    cmp x5, x1
    b.eq elimination_eliminate_rows_loop_end
    add x5, x5, #1
    b elimination_eliminate_rows_loop
elimination_eliminate_rows_loop_end:
    add x4, x4, #1
    b elimination_loop
elimination_loop_end:
    ret
