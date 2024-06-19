.cpu cortex-a72
.arch armv8-a
.global pmod_mat_mul_8_asm_2_mn_k
pmod_mat_mul_8_asm_2_mn_k:
    mov w10, 0x0481
    movk w10, 0x8018, lsl #16
    dup v0.4s, w10
    mov w10, #4093
    dup v1.4s, w10
    mov x3, #68
    mov x4, #2380
    mov x5, #0
r_loop_pr:
    mov x6, #0
c_loop_pr:
    mov x7, #0
k_loop_pr_1:
    madd x8, x3, x5, x1
    add x9, x2, x6, lsl #1
    ld1 {v19.8h}, [x9]
    add x11, x9, x4
    ld1 {v20.8h}, [x11], x4
    ld1 {v21.8h}, [x11], x4
    ld1 {v22.8h}, [x11], x4
    ld1 {v23.8h}, [x11], x4
    ld1 {v24.8h}, [x11], x4
    ld1 {v25.8h}, [x11], x4
    ld1 {v26.8h}, [x11]
    ld1 {v2.8h}, [x8]
    add x10, x8, x3
    umull v3.4s, v19.4h, v2.h[0]
    umull2 v11.4s, v19.8h, v2.h[0]
    umlal v3.4s, v20.4h, v2.h[1]
    umlal2 v11.4s, v20.8h, v2.h[1]
    umlal v3.4s, v21.4h, v2.h[2]
    umlal2 v11.4s, v21.8h, v2.h[2]
    umlal v3.4s, v22.4h, v2.h[3]
    umlal2 v11.4s, v22.8h, v2.h[3]
    umlal v3.4s, v23.4h, v2.h[4]
    umlal2 v11.4s, v23.8h, v2.h[4]
    umlal v3.4s, v24.4h, v2.h[5]
    umlal2 v11.4s, v24.8h, v2.h[5]
    umlal v3.4s, v25.4h, v2.h[6]
    umlal2 v11.4s, v25.8h, v2.h[6]
    umlal v3.4s, v26.4h, v2.h[7]
    umlal2 v11.4s, v26.8h, v2.h[7]
    ld1 {v2.8h}, [x10]
    umull v4.4s, v19.4h, v2.h[0]
    umull2 v12.4s, v19.8h, v2.h[0]
    umlal v4.4s, v20.4h, v2.h[1]
    umlal2 v12.4s, v20.8h, v2.h[1]
    umlal v4.4s, v21.4h, v2.h[2]
    umlal2 v12.4s, v21.8h, v2.h[2]
    umlal v4.4s, v22.4h, v2.h[3]
    umlal2 v12.4s, v22.8h, v2.h[3]
    umlal v4.4s, v23.4h, v2.h[4]
    umlal2 v12.4s, v23.8h, v2.h[4]
    umlal v4.4s, v24.4h, v2.h[5]
    umlal2 v12.4s, v24.8h, v2.h[5]
    umlal v4.4s, v25.4h, v2.h[6]
    umlal2 v12.4s, v25.8h, v2.h[6]
    umlal v4.4s, v26.4h, v2.h[7]
    umlal2 v12.4s, v26.8h, v2.h[7]
    b k_loop_pr_end
k_loop_pr_2:
    add x8, x8, #16
    add x9, x9, x4, lsl #3
    ld1 {v19.8h}, [x9]
    add x11, x9, x4
    ld1 {v20.8h}, [x11], x4
    ld1 {v21.8h}, [x11], x4
    ld1 {v22.8h}, [x11], x4
    ld1 {v23.8h}, [x11], x4
    ld1 {v24.8h}, [x11], x4
    ld1 {v25.8h}, [x11], x4
    ld1 {v26.8h}, [x11]
    ld1 {v2.8h}, [x8]
    add x10, x8, x3
    umlal v3.4s, v19.4h, v2.h[0]
    umlal2 v11.4s, v19.8h, v2.h[0]
    umlal v3.4s, v20.4h, v2.h[1]
    umlal2 v11.4s, v20.8h, v2.h[1]
    umlal v3.4s, v21.4h, v2.h[2]
    umlal2 v11.4s, v21.8h, v2.h[2]
    umlal v3.4s, v22.4h, v2.h[3]
    umlal2 v11.4s, v22.8h, v2.h[3]
    umlal v3.4s, v23.4h, v2.h[4]
    umlal2 v11.4s, v23.8h, v2.h[4]
    umlal v3.4s, v24.4h, v2.h[5]
    umlal2 v11.4s, v24.8h, v2.h[5]
    umlal v3.4s, v25.4h, v2.h[6]
    umlal2 v11.4s, v25.8h, v2.h[6]
    umlal v3.4s, v26.4h, v2.h[7]
    umlal2 v11.4s, v26.8h, v2.h[7]
    ld1 {v2.8h}, [x10]
    umlal v4.4s, v19.4h, v2.h[0]
    umlal2 v12.4s, v19.8h, v2.h[0]
    umlal v4.4s, v20.4h, v2.h[1]
    umlal2 v12.4s, v20.8h, v2.h[1]
    umlal v4.4s, v21.4h, v2.h[2]
    umlal2 v12.4s, v21.8h, v2.h[2]
    umlal v4.4s, v22.4h, v2.h[3]
    umlal2 v12.4s, v22.8h, v2.h[3]
    umlal v4.4s, v23.4h, v2.h[4]
    umlal2 v12.4s, v23.8h, v2.h[4]
    umlal v4.4s, v24.4h, v2.h[5]
    umlal2 v12.4s, v24.8h, v2.h[5]
    umlal v4.4s, v25.4h, v2.h[6]
    umlal2 v12.4s, v25.8h, v2.h[6]
    umlal v4.4s, v26.4h, v2.h[7]
    umlal2 v12.4s, v26.8h, v2.h[7]
k_loop_pr_end:
    add x7, x7, #8
    cmp x7, #32
    blt k_loop_pr_2
k_loop_pr_pk:
    add x8, x8, #16
    add x9, x9, x4, lsl #3
    ld1 {v19.8h}, [x9]
    add x11, x9, x4
    ld1 {v20.8h}, [x11]
    ld1 {v2.8h}, [x8]
    add x10, x8, x3
    umlal v3.4s, v19.4h, v2.h[0]
    umlal2 v11.4s, v19.8h, v2.h[0]
    umlal v3.4s, v20.4h, v2.h[1]
    umlal2 v11.4s, v20.8h, v2.h[1]
    dup v2.8h, wzr
    ldrh w12, [x10, #0]
    ins v2.h[0], w12
    ldrh w12, [x10, #2]
    ins v2.h[1], w12
    umlal v4.4s, v19.4h, v2.h[0]
    umlal2 v12.4s, v19.8h, v2.h[0]
    umlal v4.4s, v20.4h, v2.h[1]
    umlal2 v12.4s, v20.8h, v2.h[1]
    umull v27.2d, v3.2s, v0.2s
    umull2 v28.2d, v3.4s, v0.4s
    uzp2 v27.4s, v27.4s, v28.4s
    ushr v27.4s, v27.4s, 11
    mls v3.4s, v27.4s, v1.4s
    umull v27.2d, v11.2s, v0.2s
    umull2 v28.2d, v11.4s, v0.4s
    uzp2 v27.4s, v27.4s, v28.4s
    ushr v27.4s, v27.4s, 11
    mls v11.4s, v27.4s, v1.4s
    uzp1 v19.8h, v3.8h, v11.8h
    umull v27.2d, v4.2s, v0.2s
    umull2 v28.2d, v4.4s, v0.4s
    uzp2 v27.4s, v27.4s, v28.4s
    ushr v27.4s, v27.4s, 11
    mls v4.4s, v27.4s, v1.4s
    umull v27.2d, v12.2s, v0.2s
    umull2 v28.2d, v12.4s, v0.4s
    uzp2 v27.4s, v27.4s, v28.4s
    ushr v27.4s, v27.4s, 11
    mls v12.4s, v27.4s, v1.4s
    uzp1 v20.8h, v4.8h, v12.8h
    add x10, x0, x6, lsl #1
    st1 {v19.8h}, [x10], x4
    st1 {v20.8h}, [x10]
c_loop_pr_end:
    add x6, x6, #8
    cmp x6, #1184
    blt c_loop_pr
c_loop_pr_pc:
    mov x7, #0
k_loop_pr_pc_1:
    madd x8, x3, x5, x1
    add x9, x2, x6, lsl #1
    ld1 {v19.8h}, [x9]
    add x11, x9, x4
    ld1 {v20.8h}, [x11], x4
    ld1 {v21.8h}, [x11], x4
    ld1 {v22.8h}, [x11], x4
    ld1 {v23.8h}, [x11], x4
    ld1 {v24.8h}, [x11], x4
    ld1 {v25.8h}, [x11], x4
    dup v26.8h, wzr
    ld1 {v26.4h}, [x11]
    ldrh w12, [x11, #8]
    ins v26.h[4], w12
    ldrh w12, [x11, #10]
    ins v26.h[5], w12
    ld1 {v2.8h}, [x8]
    add x10, x8, x3
    umull v3.4s, v19.4h, v2.h[0]
    umull2 v11.4s, v19.8h, v2.h[0]
    umlal v3.4s, v20.4h, v2.h[1]
    umlal2 v11.4s, v20.8h, v2.h[1]
    umlal v3.4s, v21.4h, v2.h[2]
    umlal2 v11.4s, v21.8h, v2.h[2]
    umlal v3.4s, v22.4h, v2.h[3]
    umlal2 v11.4s, v22.8h, v2.h[3]
    umlal v3.4s, v23.4h, v2.h[4]
    umlal2 v11.4s, v23.8h, v2.h[4]
    umlal v3.4s, v24.4h, v2.h[5]
    umlal2 v11.4s, v24.8h, v2.h[5]
    umlal v3.4s, v25.4h, v2.h[6]
    umlal2 v11.4s, v25.8h, v2.h[6]
    umlal v3.4s, v26.4h, v2.h[7]
    umlal2 v11.4s, v26.8h, v2.h[7]
    ld1 {v2.8h}, [x10]
    umull v4.4s, v19.4h, v2.h[0]
    umull2 v12.4s, v19.8h, v2.h[0]
    umlal v4.4s, v20.4h, v2.h[1]
    umlal2 v12.4s, v20.8h, v2.h[1]
    umlal v4.4s, v21.4h, v2.h[2]
    umlal2 v12.4s, v21.8h, v2.h[2]
    umlal v4.4s, v22.4h, v2.h[3]
    umlal2 v12.4s, v22.8h, v2.h[3]
    umlal v4.4s, v23.4h, v2.h[4]
    umlal2 v12.4s, v23.8h, v2.h[4]
    umlal v4.4s, v24.4h, v2.h[5]
    umlal2 v12.4s, v24.8h, v2.h[5]
    umlal v4.4s, v25.4h, v2.h[6]
    umlal2 v12.4s, v25.8h, v2.h[6]
    umlal v4.4s, v26.4h, v2.h[7]
    umlal2 v12.4s, v26.8h, v2.h[7]
    b k_loop_pr_pc_end
k_loop_pr_pc_2:
    add x8, x8, #16
    add x9, x9, x4, lsl #3
    ld1 {v19.8h}, [x9]
    add x11, x9, x4
    ld1 {v20.8h}, [x11], x4
    ld1 {v21.8h}, [x11], x4
    ld1 {v22.8h}, [x11], x4
    ld1 {v23.8h}, [x11], x4
    ld1 {v24.8h}, [x11], x4
    ld1 {v25.8h}, [x11], x4
    dup v26.8h, wzr
    ld1 {v26.4h}, [x11]
    ldrh w12, [x11, #8]
    ins v26.h[4], w12
    ldrh w12, [x11, #10]
    ins v26.h[5], w12
    ld1 {v2.8h}, [x8]
    add x10, x8, x3
    umlal v3.4s, v19.4h, v2.h[0]
    umlal2 v11.4s, v19.8h, v2.h[0]
    umlal v3.4s, v20.4h, v2.h[1]
    umlal2 v11.4s, v20.8h, v2.h[1]
    umlal v3.4s, v21.4h, v2.h[2]
    umlal2 v11.4s, v21.8h, v2.h[2]
    umlal v3.4s, v22.4h, v2.h[3]
    umlal2 v11.4s, v22.8h, v2.h[3]
    umlal v3.4s, v23.4h, v2.h[4]
    umlal2 v11.4s, v23.8h, v2.h[4]
    umlal v3.4s, v24.4h, v2.h[5]
    umlal2 v11.4s, v24.8h, v2.h[5]
    umlal v3.4s, v25.4h, v2.h[6]
    umlal2 v11.4s, v25.8h, v2.h[6]
    umlal v3.4s, v26.4h, v2.h[7]
    umlal2 v11.4s, v26.8h, v2.h[7]
    ld1 {v2.8h}, [x10]
    umlal v4.4s, v19.4h, v2.h[0]
    umlal2 v12.4s, v19.8h, v2.h[0]
    umlal v4.4s, v20.4h, v2.h[1]
    umlal2 v12.4s, v20.8h, v2.h[1]
    umlal v4.4s, v21.4h, v2.h[2]
    umlal2 v12.4s, v21.8h, v2.h[2]
    umlal v4.4s, v22.4h, v2.h[3]
    umlal2 v12.4s, v22.8h, v2.h[3]
    umlal v4.4s, v23.4h, v2.h[4]
    umlal2 v12.4s, v23.8h, v2.h[4]
    umlal v4.4s, v24.4h, v2.h[5]
    umlal2 v12.4s, v24.8h, v2.h[5]
    umlal v4.4s, v25.4h, v2.h[6]
    umlal2 v12.4s, v25.8h, v2.h[6]
    umlal v4.4s, v26.4h, v2.h[7]
    umlal2 v12.4s, v26.8h, v2.h[7]
k_loop_pr_pc_end:
    add x7, x7, #8
    cmp x7, #32
    blt k_loop_pr_pc_2
k_loop_pr_pc_pk:
    add x8, x8, #16
    add x9, x9, x4, lsl #3
    ld1 {v19.8h}, [x9]
    add x11, x9, x4
    dup v20.8h, wzr
    ld1 {v20.4h}, [x11]
    ldrh w12, [x11, #8]
    ins v20.h[4], w12
    ldrh w12, [x11, #10]
    ins v20.h[5], w12
    ld1 {v2.8h}, [x8]
    add x10, x8, x3
    umlal v3.4s, v19.4h, v2.h[0]
    umlal2 v11.4s, v19.8h, v2.h[0]
    umlal v3.4s, v20.4h, v2.h[1]
    umlal2 v11.4s, v20.8h, v2.h[1]
    dup v2.8h, wzr
    ldrh w12, [x10, #0]
    ins v2.h[0], w12
    ldrh w12, [x10, #2]
    ins v2.h[1], w12
    umlal v4.4s, v19.4h, v2.h[0]
    umlal2 v12.4s, v19.8h, v2.h[0]
    umlal v4.4s, v20.4h, v2.h[1]
    umlal2 v12.4s, v20.8h, v2.h[1]
    umull v27.2d, v3.2s, v0.2s
    umull2 v28.2d, v3.4s, v0.4s
    uzp2 v27.4s, v27.4s, v28.4s
    ushr v27.4s, v27.4s, 11
    mls v3.4s, v27.4s, v1.4s
    umull v27.2d, v11.2s, v0.2s
    umull2 v28.2d, v11.4s, v0.4s
    uzp2 v27.4s, v27.4s, v28.4s
    ushr v27.4s, v27.4s, 11
    mls v11.4s, v27.4s, v1.4s
    uzp1 v19.8h, v3.8h, v11.8h
    umull v27.2d, v4.2s, v0.2s
    umull2 v28.2d, v4.4s, v0.4s
    uzp2 v27.4s, v27.4s, v28.4s
    ushr v27.4s, v27.4s, 11
    mls v4.4s, v27.4s, v1.4s
    umull v27.2d, v12.2s, v0.2s
    umull2 v28.2d, v12.4s, v0.4s
    uzp2 v27.4s, v27.4s, v28.4s
    ushr v27.4s, v27.4s, 11
    mls v12.4s, v27.4s, v1.4s
    uzp1 v20.8h, v4.8h, v12.8h
    add x10, x0, x6, lsl #1
    st1 {v19.4h}, [x10]
    umov w12, v19.h[4]
    strh w12, [x10, #8]
    umov w12, v19.h[5]
    strh w12, [x10, #10]
    add x10, x10, x4
    st1 {v20.4h}, [x10]
    umov w12, v20.h[4]
    strh w12, [x10, #8]
    umov w12, v20.h[5]
    strh w12, [x10, #10]
r_loop_pr_end:
    ret
