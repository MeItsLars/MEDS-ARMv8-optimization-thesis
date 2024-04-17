.cpu cortex-a72
.arch armv8-a
.global pmod_mat_mul_asm_29_103_65
pmod_mat_mul_asm_29_103_65:
    mov w10, #4093
    dup v16.4s, w10
    mov x3, #206
    mov x4, #130
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
    cmp x7, #100
    blt k_loop_2
k_loop_pk:
    add x8, x8, #8
    add x9, x9, x4, lsl #2
    ld1 {v0.4h}, [x8]
    add x10, x8, x3
    ld1 {v1.4h}, [x10], x3
    ld1 {v2.4h}, [x10], x3
    dup v3.4h, wzr
    ldrh w11, [x10, #0]
    ins v3.h[0], w11
    ldrh w11, [x10, #2]
    ins v3.h[1], w11
    ldrh w11, [x10, #4]
    ins v3.h[2], w11
    ld1 {v4.4h}, [x9]
    add x10, x9, x4
    ld1 {v5.4h}, [x10], x4
    ld1 {v6.4h}, [x10]
    umlal v8.4s, v4.4h, v0.4h[0]
    umlal v8.4s, v5.4h, v0.4h[1]
    umlal v8.4s, v6.4h, v0.4h[2]
    umlal v9.4s, v4.4h, v1.4h[0]
    umlal v9.4s, v5.4h, v1.4h[1]
    umlal v9.4s, v6.4h, v1.4h[2]
    umlal v10.4s, v4.4h, v2.4h[0]
    umlal v10.4s, v5.4h, v2.4h[1]
    umlal v10.4s, v6.4h, v2.4h[2]
    umlal v11.4s, v4.4h, v3.4h[0]
    umlal v11.4s, v5.4h, v3.4h[1]
    umlal v11.4s, v6.4h, v3.4h[2]
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
    cmp x6, #64
    blt c_loop
c_loop_pc:
    mov x7, #0
k_loop_pc_1:
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
    dup v7.4h, wzr
    ldrh w11, [x10, #0]
    ins v7.h[0], w11
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
    b k_loop_pc_end
k_loop_pc_2:
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
    dup v7.4h, wzr
    ldrh w11, [x10, #0]
    ins v7.h[0], w11
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
k_loop_pc_end:
    add x7, x7, #4
    cmp x7, #100
    blt k_loop_pc_2
k_loop_pc_pk:
    add x8, x8, #8
    add x9, x9, x4, lsl #2
    ld1 {v0.4h}, [x8]
    add x10, x8, x3
    ld1 {v1.4h}, [x10], x3
    ld1 {v2.4h}, [x10], x3
    dup v3.4h, wzr
    ldrh w11, [x10, #0]
    ins v3.h[0], w11
    ldrh w11, [x10, #2]
    ins v3.h[1], w11
    ldrh w11, [x10, #4]
    ins v3.h[2], w11
    ld1 {v4.4h}, [x9]
    add x10, x9, x4
    ld1 {v5.4h}, [x10], x4
    dup v6.4h, wzr
    ldrh w11, [x10, #0]
    ins v6.h[0], w11
    umlal v8.4s, v4.4h, v0.4h[0]
    umlal v8.4s, v5.4h, v0.4h[1]
    umlal v8.4s, v6.4h, v0.4h[2]
    umlal v9.4s, v4.4h, v1.4h[0]
    umlal v9.4s, v5.4h, v1.4h[1]
    umlal v9.4s, v6.4h, v1.4h[2]
    umlal v10.4s, v4.4h, v2.4h[0]
    umlal v10.4s, v5.4h, v2.4h[1]
    umlal v10.4s, v6.4h, v2.4h[2]
    umlal v11.4s, v4.4h, v3.4h[0]
    umlal v11.4s, v5.4h, v3.4h[1]
    umlal v11.4s, v6.4h, v3.4h[2]
    ushr v12.4s, v8.4s, #12
    mul v12.4s, v12.4s, v16.4s
    sub v8.4s, v8.4s, v12.4s
    ushr v12.4s, v8.4s, #12
    mul v12.4s, v12.4s, v16.4s
    sub v8.4s, v8.4s, v12.4s
    cmhs v12.4s, v8.4s, v16.4s
    and v12.16b, v12.16b, v16.16b
    sub v8.4s, v8.4s, v12.4s
    ushr v13.4s, v9.4s, #12
    mul v13.4s, v13.4s, v16.4s
    sub v9.4s, v9.4s, v13.4s
    ushr v13.4s, v9.4s, #12
    mul v13.4s, v13.4s, v16.4s
    sub v9.4s, v9.4s, v13.4s
    cmhs v13.4s, v9.4s, v16.4s
    and v13.16b, v13.16b, v16.16b
    sub v9.4s, v9.4s, v13.4s
    ushr v14.4s, v10.4s, #12
    mul v14.4s, v14.4s, v16.4s
    sub v10.4s, v10.4s, v14.4s
    ushr v14.4s, v10.4s, #12
    mul v14.4s, v14.4s, v16.4s
    sub v10.4s, v10.4s, v14.4s
    cmhs v14.4s, v10.4s, v16.4s
    and v14.16b, v14.16b, v16.16b
    sub v10.4s, v10.4s, v14.4s
    ushr v15.4s, v11.4s, #12
    mul v15.4s, v15.4s, v16.4s
    sub v11.4s, v11.4s, v15.4s
    ushr v15.4s, v11.4s, #12
    mul v15.4s, v15.4s, v16.4s
    sub v11.4s, v11.4s, v15.4s
    cmhs v15.4s, v11.4s, v16.4s
    and v15.16b, v15.16b, v16.16b
    sub v11.4s, v11.4s, v15.4s
    add x10, x0, x6, lsl #1
    mov w11, v8.s[0]
    strh w11, [x10, #0]
    add x10, x10, x4
    mov w11, v9.s[0]
    strh w11, [x10, #0]
    add x10, x10, x4
    mov w11, v10.s[0]
    strh w11, [x10, #0]
    add x10, x10, x4
    mov w11, v11.s[0]
    strh w11, [x10, #0]
r_loop_end:
    add x5, x5, #4
    add x0, x0, x4, lsl #2
    cmp x5, #28
    blt r_loop
r_loop_pr:
    mov x6, #0
c_loop_pr:
    mov x7, #0
k_loop_pr_1:
    madd x8, x3, x5, x1
    add x9, x2, x6, lsl #1
    ld1 {v0.4h}, [x8]
    ld1 {v4.4h}, [x9]
    add x10, x9, x4
    ld1 {v5.4h}, [x10], x4
    ld1 {v6.4h}, [x10], x4
    ld1 {v7.4h}, [x10]
    umull v8.4s, v4.4h, v0.4h[0]
    umlal v8.4s, v5.4h, v0.4h[1]
    umlal v8.4s, v6.4h, v0.4h[2]
    umlal v8.4s, v7.4h, v0.4h[3]
    b k_loop_pr_end
k_loop_pr_2:
    add x8, x8, #8
    add x9, x9, x4, lsl #2
    ld1 {v0.4h}, [x8]
    ld1 {v4.4h}, [x9]
    add x10, x9, x4
    ld1 {v5.4h}, [x10], x4
    ld1 {v6.4h}, [x10], x4
    ld1 {v7.4h}, [x10]
    umlal v8.4s, v4.4h, v0.4h[0]
    umlal v8.4s, v5.4h, v0.4h[1]
    umlal v8.4s, v6.4h, v0.4h[2]
    umlal v8.4s, v7.4h, v0.4h[3]
k_loop_pr_end:
    add x7, x7, #4
    cmp x7, #100
    blt k_loop_pr_2
k_loop_pr_pk:
    add x8, x8, #8
    add x9, x9, x4, lsl #2
    dup v0.4h, wzr
    ldrh w11, [x8, #0]
    ins v0.h[0], w11
    ldrh w11, [x8, #2]
    ins v0.h[1], w11
    ldrh w11, [x8, #4]
    ins v0.h[2], w11
    ld1 {v4.4h}, [x9]
    add x10, x9, x4
    ld1 {v5.4h}, [x10], x4
    ld1 {v6.4h}, [x10]
    umlal v8.4s, v4.4h, v0.4h[0]
    umlal v8.4s, v5.4h, v0.4h[1]
    umlal v8.4s, v6.4h, v0.4h[2]
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
    add x10, x0, x6, lsl #1
    st1 {v8.4h}, [x10]
c_loop_pr_end:
    add x6, x6, #4
    cmp x6, #64
    blt c_loop_pr
c_loop_pr_pc:
    mov x7, #0
k_loop_pr_pc_1:
    madd x8, x3, x5, x1
    add x9, x2, x6, lsl #1
    ld1 {v0.4h}, [x8]
    ld1 {v4.4h}, [x9]
    add x10, x9, x4
    ld1 {v5.4h}, [x10], x4
    ld1 {v6.4h}, [x10], x4
    dup v7.4h, wzr
    ldrh w11, [x10, #0]
    ins v7.h[0], w11
    umull v8.4s, v4.4h, v0.4h[0]
    umlal v8.4s, v5.4h, v0.4h[1]
    umlal v8.4s, v6.4h, v0.4h[2]
    umlal v8.4s, v7.4h, v0.4h[3]
    b k_loop_pr_pc_end
k_loop_pr_pc_2:
    add x8, x8, #8
    add x9, x9, x4, lsl #2
    ld1 {v0.4h}, [x8]
    ld1 {v4.4h}, [x9]
    add x10, x9, x4
    ld1 {v5.4h}, [x10], x4
    ld1 {v6.4h}, [x10], x4
    dup v7.4h, wzr
    ldrh w11, [x10, #0]
    ins v7.h[0], w11
    umlal v8.4s, v4.4h, v0.4h[0]
    umlal v8.4s, v5.4h, v0.4h[1]
    umlal v8.4s, v6.4h, v0.4h[2]
    umlal v8.4s, v7.4h, v0.4h[3]
k_loop_pr_pc_end:
    add x7, x7, #4
    cmp x7, #100
    blt k_loop_pr_pc_2
k_loop_pr_pc_pk:
    add x8, x8, #8
    add x9, x9, x4, lsl #2
    dup v0.4h, wzr
    ldrh w11, [x8, #0]
    ins v0.h[0], w11
    ldrh w11, [x8, #2]
    ins v0.h[1], w11
    ldrh w11, [x8, #4]
    ins v0.h[2], w11
    ld1 {v4.4h}, [x9]
    add x10, x9, x4
    ld1 {v5.4h}, [x10], x4
    dup v6.4h, wzr
    ldrh w11, [x10, #0]
    ins v6.h[0], w11
    umlal v8.4s, v4.4h, v0.4h[0]
    umlal v8.4s, v5.4h, v0.4h[1]
    umlal v8.4s, v6.4h, v0.4h[2]
    ushr v12.4s, v8.4s, #12
    mul v12.4s, v12.4s, v16.4s
    sub v8.4s, v8.4s, v12.4s
    ushr v12.4s, v8.4s, #12
    mul v12.4s, v12.4s, v16.4s
    sub v8.4s, v8.4s, v12.4s
    cmhs v12.4s, v8.4s, v16.4s
    and v12.16b, v12.16b, v16.16b
    sub v8.4s, v8.4s, v12.4s
    add x10, x0, x6, lsl #1
    mov w11, v8.s[0]
    strh w11, [x10, #0]
r_loop_pr_end:
    ret
