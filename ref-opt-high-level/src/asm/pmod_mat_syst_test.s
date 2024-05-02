.cpu cortex-a72
.arch armv8-a
.global pmod_mat_syst_test
pmod_mat_syst_test:
    mov x3, x1
    mov w11, #4093
    dup v1.4h, w11
    dup v2.4h, wzr
    dup v0.4h, wzr
    mov x4, #0
elimination_loop:
    cmp x4, x3
    b.eq elimination_loop_end
    madd x11, x2, x4, x4
    ldr d3, [x0, x11, lsl #3]
    madd x7, x2, x4, x4
    mov x7, x7, lsl #3
    cmeq v4.4h, v3.4h, v2.4h
    add x5, x4, #1
elimination_row_zero_fix_outer_loop:
    cmp x5, x1
    b.eq elimination_row_zero_fix_outer_loop_end
    madd x8, x2, x5, x4
    mov x8, x8, lsl #3
    mov x6, x4
elimination_row_zero_fix_inner_loop:
    cmp x6, x2
    b.eq elimination_row_zero_fix_inner_loop_end
    ldr d5, [x7], #8
    ldr d6, [x8], #8
    and v7.16b, v4.16b, v6.16b
    uaddl v8.4s, v7.4h, v5.4h
    ushr v7.4s, v8.4s, #12
    mul v7.4s, v7.4s, v1.4s
    sub v8.4s, v8.4s, v7.4s
    ushr v7.4s, v8.4s, #12
    mul v7.4s, v7.4s, v1.4s
    sub v8.4s, v8.4s, v7.4s
    cmhs v7.4s, v8.4s, v1.4s
    and v7.16b, v7.16b, v1.16b
    sub v8.4s, v8.4s, v7.4s
    sqxtn v8.4h, v8.4s
    str d8, [x7], #8
    add x6, x6, #1
    b elimination_row_zero_fix_inner_loop
elimination_row_zero_fix_inner_loop_end:
    add x5, x5, #1
    b elimination_row_zero_fix_outer_loop
elimination_row_zero_fix_outer_loop_end:
    mov x5, x4
elimination_normalize_row_loop:
    cmp x5, x2
    b.eq elimination_normalize_row_loop_end
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
