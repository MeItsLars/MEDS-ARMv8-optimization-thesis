.cpu cortex-a72
.arch armv8-a
.global pmod_mat_syst_test
pmod_mat_syst_test:
    mov x3, x1
    mov x5, #4093
    dup v1.4h, w5
    dup v2.4s, w5
    dup v0.4h, wzr
    mov x4, xzr
    mov x6, #0
elimination_loop:
    cmp x6, x3
    b.eq elimination_loop_end
    madd x9, x2, x6, x6
    madd x10, x2, x6, x6
    add x10, x0, x10, lsl #1
    add x7, x6, #1
elimination_row_zero_fix_outer_loop:
    cmp x7, x1
    b.eq elimination_row_zero_fix_outer_loop_end
    madd x11, x2, x7, x6
    add x11, x0, x11, lsl #1
    ldrh w12, [x0, x9, lsl #1]
    dup v3.4h, w12
    cmeq v6.4h, v3.4h, v0.4h
    mov x8, x6
elimination_row_zero_fix_inner_loop_neon:
    cmp x8, x2
    b.eq elimination_row_zero_fix_inner_loop_neon_end
    ld1 {v4.4h}, [x10]
    ld1 {v5.4h}, [x11], #8
    and v16.16b, v6.16b, v5.16b
    st1 {v5.4h}, [x10], #8
    add x8, x8, #1
    b elimination_row_zero_fix_inner_loop_neon
elimination_row_zero_fix_inner_loop_neon_end:
    add x7, x7, #1
    b elimination_row_zero_fix_outer_loop
elimination_row_zero_fix_outer_loop_end:
    add x6, x6, #1
    b elimination_loop
elimination_loop_end:
    mov x0, x4
    ret
