.cpu cortex-a72
.arch armv8-a
.global pmod_mat_mul_asm_4_4_4
pmod_mat_mul_asm_4_4_4:
    mov w10, #4093
    dup v16.4s, w10
    mov x3, #8
    mov x4, #8
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
    umull v8.4s, v4.4h, v0.4h[0]
    umlal v8.4s, v5.4h, v0.4h[1]
    umlal v8.4s, v6.4h, v0.4h[2]
    umlal v8.4s, v7.4h, v0.4h[3]
    umull v9.4s, v4.4h, v1.4h[0]
    umlal v9.4s, v5.4h, v1.4h[1]
    umlal v9.4s, v6.4h, v1.4h[2]
    umlal v9.4s, v7.4h, v1.4h[3]
    umull v10.4s, v4.4h, v2.4h[0]
    umlal v10.4s, v5.4h, v2.4h[1]
    umlal v10.4s, v6.4h, v2.4h[2]
    umlal v10.4s, v7.4h, v2.4h[3]
    umull v11.4s, v4.4h, v3.4h[0]
    umlal v11.4s, v5.4h, v3.4h[1]
    umlal v11.4s, v6.4h, v3.4h[2]
    umlal v11.4s, v7.4h, v3.4h[3]
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
    umlal v8.4s, v4.4h, v0.4h[0]
    umlal v8.4s, v5.4h, v0.4h[1]
    umlal v8.4s, v6.4h, v0.4h[2]
    umlal v8.4s, v7.4h, v0.4h[3]
    umlal v9.4s, v4.4h, v1.4h[0]
    umlal v9.4s, v5.4h, v1.4h[1]
    umlal v9.4s, v6.4h, v1.4h[2]
    umlal v9.4s, v7.4h, v1.4h[3]
    umlal v10.4s, v4.4h, v2.4h[0]
    umlal v10.4s, v5.4h, v2.4h[1]
    umlal v10.4s, v6.4h, v2.4h[2]
    umlal v10.4s, v7.4h, v2.4h[3]
    umlal v11.4s, v4.4h, v3.4h[0]
    umlal v11.4s, v5.4h, v3.4h[1]
    umlal v11.4s, v6.4h, v3.4h[2]
    umlal v11.4s, v7.4h, v3.4h[3]
k_loop_end:
    add x7, x7, #4
    cmp x7, #4
    blt k_loop_2
    ushr v12.4s, v8.4s, #12
    mul v12.4s, v12.4s, v16.4s
    sub v8.4s, v8.4s, v12.4s
    ushr v12.4s, v8.4s, #12
    mul v12.4s, v12.4s, v16.4s
    sub v8.4s, v8.4s, v12.4s
    cmhs v12.4s, v8.4s, v16.4s
    and v12.16b, v12.16b, v16.16b
    sub v8.4s, v8.4s, v12.4s
    sqxtn v8.4h, v8.4s
    ushr v13.4s, v9.4s, #12
    mul v13.4s, v13.4s, v16.4s
    sub v9.4s, v9.4s, v13.4s
    ushr v13.4s, v9.4s, #12
    mul v13.4s, v13.4s, v16.4s
    sub v9.4s, v9.4s, v13.4s
    cmhs v13.4s, v9.4s, v16.4s
    and v13.16b, v13.16b, v16.16b
    sub v9.4s, v9.4s, v13.4s
    sqxtn v9.4h, v9.4s
    ushr v14.4s, v10.4s, #12
    mul v14.4s, v14.4s, v16.4s
    sub v10.4s, v10.4s, v14.4s
    ushr v14.4s, v10.4s, #12
    mul v14.4s, v14.4s, v16.4s
    sub v10.4s, v10.4s, v14.4s
    cmhs v14.4s, v10.4s, v16.4s
    and v14.16b, v14.16b, v16.16b
    sub v10.4s, v10.4s, v14.4s
    sqxtn v10.4h, v10.4s
    ushr v15.4s, v11.4s, #12
    mul v15.4s, v15.4s, v16.4s
    sub v11.4s, v11.4s, v15.4s
    ushr v15.4s, v11.4s, #12
    mul v15.4s, v15.4s, v16.4s
    sub v11.4s, v11.4s, v15.4s
    cmhs v15.4s, v11.4s, v16.4s
    and v15.16b, v15.16b, v16.16b
    sub v11.4s, v11.4s, v15.4s
    sqxtn v11.4h, v11.4s
    add x10, x0, x6, lsl #1
    st1 {v8.4h}, [x10], x4
    st1 {v9.4h}, [x10], x4
    st1 {v10.4h}, [x10], x4
    st1 {v11.4h}, [x10]
c_loop_end:
    add x6, x6, #4
    cmp x6, #4
    blt c_loop
r_loop_end:
    add x5, x5, #4
    add x0, x0, x4, lsl #2
    cmp x5, #4
    blt r_loop
    ret
