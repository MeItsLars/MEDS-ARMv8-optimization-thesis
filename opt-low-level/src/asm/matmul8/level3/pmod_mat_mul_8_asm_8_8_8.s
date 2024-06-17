.cpu cortex-a72
.arch armv8-a
.global pmod_mat_mul_8_asm_8_8_8
pmod_mat_mul_8_asm_8_8_8:
    mov w10, 0x0481
    movk w10, 0x8018, lsl #16
    dup v29.4s, w10
    mov w10, #4093
    dup v28.4s, w10
    mov x3, #16
    mov x4, #16
    mov x5, #0
r_loop:
    mov x6, #0
c_loop:
    mov x7, #0
k_loop_1:
    madd x8, x3, x5, x1
    add x9, x2, x6, lsl #1
    ld1 {v0.4h}, [x8]
    add x10, x8, x3
    ld1 {v1.4h}, [x10], x3
    ld1 {v2.4h}, [x10], x3
    ld1 {v3.4h}, [x10]
    ld1 {v4.4h}, [x9]
    add x10, x9, x4
    ld1 {v5.4h}, [x10], x4
    ld1 {v6.4h}, [x10], x4
    ld1 {v7.4h}, [x10]
    umull v16.4s, v4.4h, v0.4h[0]
    umlal v16.4s, v5.4h, v0.4h[1]
    umlal v16.4s, v6.4h, v0.4h[2]
    umlal v16.4s, v7.4h, v0.4h[3]
    umull v17.4s, v4.4h, v1.4h[0]
    umlal v17.4s, v5.4h, v1.4h[1]
    umlal v17.4s, v6.4h, v1.4h[2]
    umlal v17.4s, v7.4h, v1.4h[3]
    umull v18.4s, v4.4h, v2.4h[0]
    umlal v18.4s, v5.4h, v2.4h[1]
    umlal v18.4s, v6.4h, v2.4h[2]
    umlal v18.4s, v7.4h, v2.4h[3]
    umull v19.4s, v4.4h, v3.4h[0]
    umlal v19.4s, v5.4h, v3.4h[1]
    umlal v19.4s, v6.4h, v3.4h[2]
    umlal v19.4s, v7.4h, v3.4h[3]
    b k_loop_end
k_loop_2:
    add x8, x8, #8
    add x9, x9, x4, lsl #2
    ld1 {v0.4h}, [x8]
    add x10, x8, x3
    ld1 {v1.4h}, [x10], x3
    ld1 {v2.4h}, [x10], x3
    ld1 {v3.4h}, [x10]
    ld1 {v4.4h}, [x9]
    add x10, x9, x4
    ld1 {v5.4h}, [x10], x4
    ld1 {v6.4h}, [x10], x4
    ld1 {v7.4h}, [x10]
    umlal v16.4s, v4.4h, v0.4h[0]
    umlal v16.4s, v5.4h, v0.4h[1]
    umlal v16.4s, v6.4h, v0.4h[2]
    umlal v16.4s, v7.4h, v0.4h[3]
    umlal v17.4s, v4.4h, v1.4h[0]
    umlal v17.4s, v5.4h, v1.4h[1]
    umlal v17.4s, v6.4h, v1.4h[2]
    umlal v17.4s, v7.4h, v1.4h[3]
    umlal v18.4s, v4.4h, v2.4h[0]
    umlal v18.4s, v5.4h, v2.4h[1]
    umlal v18.4s, v6.4h, v2.4h[2]
    umlal v18.4s, v7.4h, v2.4h[3]
    umlal v19.4s, v4.4h, v3.4h[0]
    umlal v19.4s, v5.4h, v3.4h[1]
    umlal v19.4s, v6.4h, v3.4h[2]
    umlal v19.4s, v7.4h, v3.4h[3]
k_loop_end:
    add x7, x7, #4
    cmp x7, #8
    blt k_loop_2
    umull v30.2d, v16.2s, v29.2s
    umull2 v31.2d, v16.4s, v29.4s
    uzp2 v30.4s, v30.4s, v31.4s
    ushr v30.4s, v30.4s, 11
    mls v16.4s, v30.4s, v28.4s
    xtn v16.4h, v16.4s
    umull v30.2d, v17.2s, v29.2s
    umull2 v31.2d, v17.4s, v29.4s
    uzp2 v30.4s, v30.4s, v31.4s
    ushr v30.4s, v30.4s, 11
    mls v17.4s, v30.4s, v28.4s
    xtn v17.4h, v17.4s
    umull v30.2d, v18.2s, v29.2s
    umull2 v31.2d, v18.4s, v29.4s
    uzp2 v30.4s, v30.4s, v31.4s
    ushr v30.4s, v30.4s, 11
    mls v18.4s, v30.4s, v28.4s
    xtn v18.4h, v18.4s
    umull v30.2d, v19.2s, v29.2s
    umull2 v31.2d, v19.4s, v29.4s
    uzp2 v30.4s, v30.4s, v31.4s
    ushr v30.4s, v30.4s, 11
    mls v19.4s, v30.4s, v28.4s
    xtn v19.4h, v19.4s
    add x10, x0, x6, lsl #1
    st1 {v16.4h}, [x10], x4
    st1 {v17.4h}, [x10], x4
    st1 {v18.4h}, [x10], x4
    st1 {v19.4h}, [x10]
c_loop_end:
    add x6, x6, #4
    cmp x6, #8
    blt c_loop
r_loop_end:
    add x5, x5, #4
    add x0, x0, x4, lsl #2
    cmp x5, #8
    blt r_loop
    ret
