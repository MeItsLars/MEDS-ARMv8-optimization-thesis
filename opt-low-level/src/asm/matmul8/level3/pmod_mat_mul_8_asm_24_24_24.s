.cpu cortex-a72
.arch armv8-a
.global pmod_mat_mul_8_asm_24_24_24
pmod_mat_mul_8_asm_24_24_24:
    mov w10, 0x0481
    movk w10, 0x8018, lsl #16
    dup v0.4s, w10
    mov w10, #4093
    dup v1.4s, w10
    mov x3, #48
    mov x4, #48
    mov x5, #0
r_loop:
    mov x6, #0
c_loop:
    mov x7, #0
k_loop_1:
    madd x8, x3, x5, x1
    add x9, x2, x6, lsl #1
    ld1 {v18.8h}, [x8]
    add x10, x8, x3
    ld1 {v19.8h}, [x10], x3
    ld1 {v20.8h}, [x10], x3
    ld1 {v21.8h}, [x10], x3
    ld1 {v22.8h}, [x10], x3
    ld1 {v23.8h}, [x10], x3
    ld1 {v24.8h}, [x10], x3
    ld1 {v25.8h}, [x10]
    ld1 {v26.8h}, [x9]
    add x10, x9, x4
    ld1 {v27.8h}, [x10], x4
    ld1 {v28.8h}, [x10], x4
    ld1 {v29.8h}, [x10], x4
    ld1 {v30.8h}, [x10], x4
    ld1 {v31.8h}, [x10], x4
    ld1 {v32.8h}, [x10], x4
    ld1 {v33.8h}, [x10]
    umull v2.4s, v26.4s, v18.8h[0]
    umull2 v10.4s, v26.8h, v18.8h[0]
    umlal v2.4s, v27.4s, v18.8h[1]
    umlal2 v10.4s, v27.8h, v18.8h[1]
    umlal v2.4s, v28.4s, v18.8h[2]
    umlal2 v10.4s, v28.8h, v18.8h[2]
    umlal v2.4s, v29.4s, v18.8h[3]
    umlal2 v10.4s, v29.8h, v18.8h[3]
    umlal v2.4s, v30.4s, v18.8h[4]
    umlal2 v10.4s, v30.8h, v18.8h[4]
    umlal v2.4s, v31.4s, v18.8h[5]
    umlal2 v10.4s, v31.8h, v18.8h[5]
    umlal v2.4s, v32.4s, v18.8h[6]
    umlal2 v10.4s, v32.8h, v18.8h[6]
    umlal v2.4s, v33.4s, v18.8h[7]
    umlal2 v10.4s, v33.8h, v18.8h[7]
    umull v3.4s, v26.4s, v19.8h[0]
    umull2 v11.4s, v26.8h, v19.8h[0]
    umlal v3.4s, v27.4s, v19.8h[1]
    umlal2 v11.4s, v27.8h, v19.8h[1]
    umlal v3.4s, v28.4s, v19.8h[2]
    umlal2 v11.4s, v28.8h, v19.8h[2]
    umlal v3.4s, v29.4s, v19.8h[3]
    umlal2 v11.4s, v29.8h, v19.8h[3]
    umlal v3.4s, v30.4s, v19.8h[4]
    umlal2 v11.4s, v30.8h, v19.8h[4]
    umlal v3.4s, v31.4s, v19.8h[5]
    umlal2 v11.4s, v31.8h, v19.8h[5]
    umlal v3.4s, v32.4s, v19.8h[6]
    umlal2 v11.4s, v32.8h, v19.8h[6]
    umlal v3.4s, v33.4s, v19.8h[7]
    umlal2 v11.4s, v33.8h, v19.8h[7]
    umull v4.4s, v26.4s, v20.8h[0]
    umull2 v12.4s, v26.8h, v20.8h[0]
    umlal v4.4s, v27.4s, v20.8h[1]
    umlal2 v12.4s, v27.8h, v20.8h[1]
    umlal v4.4s, v28.4s, v20.8h[2]
    umlal2 v12.4s, v28.8h, v20.8h[2]
    umlal v4.4s, v29.4s, v20.8h[3]
    umlal2 v12.4s, v29.8h, v20.8h[3]
    umlal v4.4s, v30.4s, v20.8h[4]
    umlal2 v12.4s, v30.8h, v20.8h[4]
    umlal v4.4s, v31.4s, v20.8h[5]
    umlal2 v12.4s, v31.8h, v20.8h[5]
    umlal v4.4s, v32.4s, v20.8h[6]
    umlal2 v12.4s, v32.8h, v20.8h[6]
    umlal v4.4s, v33.4s, v20.8h[7]
    umlal2 v12.4s, v33.8h, v20.8h[7]
    umull v5.4s, v26.4s, v21.8h[0]
    umull2 v13.4s, v26.8h, v21.8h[0]
    umlal v5.4s, v27.4s, v21.8h[1]
    umlal2 v13.4s, v27.8h, v21.8h[1]
    umlal v5.4s, v28.4s, v21.8h[2]
    umlal2 v13.4s, v28.8h, v21.8h[2]
    umlal v5.4s, v29.4s, v21.8h[3]
    umlal2 v13.4s, v29.8h, v21.8h[3]
    umlal v5.4s, v30.4s, v21.8h[4]
    umlal2 v13.4s, v30.8h, v21.8h[4]
    umlal v5.4s, v31.4s, v21.8h[5]
    umlal2 v13.4s, v31.8h, v21.8h[5]
    umlal v5.4s, v32.4s, v21.8h[6]
    umlal2 v13.4s, v32.8h, v21.8h[6]
    umlal v5.4s, v33.4s, v21.8h[7]
    umlal2 v13.4s, v33.8h, v21.8h[7]
    umull v6.4s, v26.4s, v22.8h[0]
    umull2 v14.4s, v26.8h, v22.8h[0]
    umlal v6.4s, v27.4s, v22.8h[1]
    umlal2 v14.4s, v27.8h, v22.8h[1]
    umlal v6.4s, v28.4s, v22.8h[2]
    umlal2 v14.4s, v28.8h, v22.8h[2]
    umlal v6.4s, v29.4s, v22.8h[3]
    umlal2 v14.4s, v29.8h, v22.8h[3]
    umlal v6.4s, v30.4s, v22.8h[4]
    umlal2 v14.4s, v30.8h, v22.8h[4]
    umlal v6.4s, v31.4s, v22.8h[5]
    umlal2 v14.4s, v31.8h, v22.8h[5]
    umlal v6.4s, v32.4s, v22.8h[6]
    umlal2 v14.4s, v32.8h, v22.8h[6]
    umlal v6.4s, v33.4s, v22.8h[7]
    umlal2 v14.4s, v33.8h, v22.8h[7]
    umull v7.4s, v26.4s, v23.8h[0]
    umull2 v15.4s, v26.8h, v23.8h[0]
    umlal v7.4s, v27.4s, v23.8h[1]
    umlal2 v15.4s, v27.8h, v23.8h[1]
    umlal v7.4s, v28.4s, v23.8h[2]
    umlal2 v15.4s, v28.8h, v23.8h[2]
    umlal v7.4s, v29.4s, v23.8h[3]
    umlal2 v15.4s, v29.8h, v23.8h[3]
    umlal v7.4s, v30.4s, v23.8h[4]
    umlal2 v15.4s, v30.8h, v23.8h[4]
    umlal v7.4s, v31.4s, v23.8h[5]
    umlal2 v15.4s, v31.8h, v23.8h[5]
    umlal v7.4s, v32.4s, v23.8h[6]
    umlal2 v15.4s, v32.8h, v23.8h[6]
    umlal v7.4s, v33.4s, v23.8h[7]
    umlal2 v15.4s, v33.8h, v23.8h[7]
    umull v8.4s, v26.4s, v24.8h[0]
    umull2 v16.4s, v26.8h, v24.8h[0]
    umlal v8.4s, v27.4s, v24.8h[1]
    umlal2 v16.4s, v27.8h, v24.8h[1]
    umlal v8.4s, v28.4s, v24.8h[2]
    umlal2 v16.4s, v28.8h, v24.8h[2]
    umlal v8.4s, v29.4s, v24.8h[3]
    umlal2 v16.4s, v29.8h, v24.8h[3]
    umlal v8.4s, v30.4s, v24.8h[4]
    umlal2 v16.4s, v30.8h, v24.8h[4]
    umlal v8.4s, v31.4s, v24.8h[5]
    umlal2 v16.4s, v31.8h, v24.8h[5]
    umlal v8.4s, v32.4s, v24.8h[6]
    umlal2 v16.4s, v32.8h, v24.8h[6]
    umlal v8.4s, v33.4s, v24.8h[7]
    umlal2 v16.4s, v33.8h, v24.8h[7]
    umull v9.4s, v26.4s, v25.8h[0]
    umull2 v17.4s, v26.8h, v25.8h[0]
    umlal v9.4s, v27.4s, v25.8h[1]
    umlal2 v17.4s, v27.8h, v25.8h[1]
    umlal v9.4s, v28.4s, v25.8h[2]
    umlal2 v17.4s, v28.8h, v25.8h[2]
    umlal v9.4s, v29.4s, v25.8h[3]
    umlal2 v17.4s, v29.8h, v25.8h[3]
    umlal v9.4s, v30.4s, v25.8h[4]
    umlal2 v17.4s, v30.8h, v25.8h[4]
    umlal v9.4s, v31.4s, v25.8h[5]
    umlal2 v17.4s, v31.8h, v25.8h[5]
    umlal v9.4s, v32.4s, v25.8h[6]
    umlal2 v17.4s, v32.8h, v25.8h[6]
    umlal v9.4s, v33.4s, v25.8h[7]
    umlal2 v17.4s, v33.8h, v25.8h[7]
    b k_loop_end
k_loop_2:
    add x8, x8, #8
    add x9, x9, x4, lsl #2
    ld1 {v18.8h}, [x8]
    add x10, x8, x3
    ld1 {v19.8h}, [x10], x3
    ld1 {v20.8h}, [x10], x3
    ld1 {v21.8h}, [x10], x3
    ld1 {v22.8h}, [x10], x3
    ld1 {v23.8h}, [x10], x3
    ld1 {v24.8h}, [x10], x3
    ld1 {v25.8h}, [x10]
    ld1 {v26.8h}, [x9]
    add x10, x9, x4
    ld1 {v27.8h}, [x10], x4
    ld1 {v28.8h}, [x10], x4
    ld1 {v29.8h}, [x10], x4
    ld1 {v30.8h}, [x10], x4
    ld1 {v31.8h}, [x10], x4
    ld1 {v32.8h}, [x10], x4
    ld1 {v33.8h}, [x10]
    umlal v2.4s, v26.4s, v18.8h[0]
    umlal2 v10.4s, v26.8h, v18.8h[0]
    umlal v2.4s, v27.4s, v18.8h[1]
    umlal2 v10.4s, v27.8h, v18.8h[1]
    umlal v2.4s, v28.4s, v18.8h[2]
    umlal2 v10.4s, v28.8h, v18.8h[2]
    umlal v2.4s, v29.4s, v18.8h[3]
    umlal2 v10.4s, v29.8h, v18.8h[3]
    umlal v2.4s, v30.4s, v18.8h[4]
    umlal2 v10.4s, v30.8h, v18.8h[4]
    umlal v2.4s, v31.4s, v18.8h[5]
    umlal2 v10.4s, v31.8h, v18.8h[5]
    umlal v2.4s, v32.4s, v18.8h[6]
    umlal2 v10.4s, v32.8h, v18.8h[6]
    umlal v2.4s, v33.4s, v18.8h[7]
    umlal2 v10.4s, v33.8h, v18.8h[7]
    umlal v3.4s, v26.4s, v19.8h[0]
    umlal2 v11.4s, v26.8h, v19.8h[0]
    umlal v3.4s, v27.4s, v19.8h[1]
    umlal2 v11.4s, v27.8h, v19.8h[1]
    umlal v3.4s, v28.4s, v19.8h[2]
    umlal2 v11.4s, v28.8h, v19.8h[2]
    umlal v3.4s, v29.4s, v19.8h[3]
    umlal2 v11.4s, v29.8h, v19.8h[3]
    umlal v3.4s, v30.4s, v19.8h[4]
    umlal2 v11.4s, v30.8h, v19.8h[4]
    umlal v3.4s, v31.4s, v19.8h[5]
    umlal2 v11.4s, v31.8h, v19.8h[5]
    umlal v3.4s, v32.4s, v19.8h[6]
    umlal2 v11.4s, v32.8h, v19.8h[6]
    umlal v3.4s, v33.4s, v19.8h[7]
    umlal2 v11.4s, v33.8h, v19.8h[7]
    umlal v4.4s, v26.4s, v20.8h[0]
    umlal2 v12.4s, v26.8h, v20.8h[0]
    umlal v4.4s, v27.4s, v20.8h[1]
    umlal2 v12.4s, v27.8h, v20.8h[1]
    umlal v4.4s, v28.4s, v20.8h[2]
    umlal2 v12.4s, v28.8h, v20.8h[2]
    umlal v4.4s, v29.4s, v20.8h[3]
    umlal2 v12.4s, v29.8h, v20.8h[3]
    umlal v4.4s, v30.4s, v20.8h[4]
    umlal2 v12.4s, v30.8h, v20.8h[4]
    umlal v4.4s, v31.4s, v20.8h[5]
    umlal2 v12.4s, v31.8h, v20.8h[5]
    umlal v4.4s, v32.4s, v20.8h[6]
    umlal2 v12.4s, v32.8h, v20.8h[6]
    umlal v4.4s, v33.4s, v20.8h[7]
    umlal2 v12.4s, v33.8h, v20.8h[7]
    umlal v5.4s, v26.4s, v21.8h[0]
    umlal2 v13.4s, v26.8h, v21.8h[0]
    umlal v5.4s, v27.4s, v21.8h[1]
    umlal2 v13.4s, v27.8h, v21.8h[1]
    umlal v5.4s, v28.4s, v21.8h[2]
    umlal2 v13.4s, v28.8h, v21.8h[2]
    umlal v5.4s, v29.4s, v21.8h[3]
    umlal2 v13.4s, v29.8h, v21.8h[3]
    umlal v5.4s, v30.4s, v21.8h[4]
    umlal2 v13.4s, v30.8h, v21.8h[4]
    umlal v5.4s, v31.4s, v21.8h[5]
    umlal2 v13.4s, v31.8h, v21.8h[5]
    umlal v5.4s, v32.4s, v21.8h[6]
    umlal2 v13.4s, v32.8h, v21.8h[6]
    umlal v5.4s, v33.4s, v21.8h[7]
    umlal2 v13.4s, v33.8h, v21.8h[7]
    umlal v6.4s, v26.4s, v22.8h[0]
    umlal2 v14.4s, v26.8h, v22.8h[0]
    umlal v6.4s, v27.4s, v22.8h[1]
    umlal2 v14.4s, v27.8h, v22.8h[1]
    umlal v6.4s, v28.4s, v22.8h[2]
    umlal2 v14.4s, v28.8h, v22.8h[2]
    umlal v6.4s, v29.4s, v22.8h[3]
    umlal2 v14.4s, v29.8h, v22.8h[3]
    umlal v6.4s, v30.4s, v22.8h[4]
    umlal2 v14.4s, v30.8h, v22.8h[4]
    umlal v6.4s, v31.4s, v22.8h[5]
    umlal2 v14.4s, v31.8h, v22.8h[5]
    umlal v6.4s, v32.4s, v22.8h[6]
    umlal2 v14.4s, v32.8h, v22.8h[6]
    umlal v6.4s, v33.4s, v22.8h[7]
    umlal2 v14.4s, v33.8h, v22.8h[7]
    umlal v7.4s, v26.4s, v23.8h[0]
    umlal2 v15.4s, v26.8h, v23.8h[0]
    umlal v7.4s, v27.4s, v23.8h[1]
    umlal2 v15.4s, v27.8h, v23.8h[1]
    umlal v7.4s, v28.4s, v23.8h[2]
    umlal2 v15.4s, v28.8h, v23.8h[2]
    umlal v7.4s, v29.4s, v23.8h[3]
    umlal2 v15.4s, v29.8h, v23.8h[3]
    umlal v7.4s, v30.4s, v23.8h[4]
    umlal2 v15.4s, v30.8h, v23.8h[4]
    umlal v7.4s, v31.4s, v23.8h[5]
    umlal2 v15.4s, v31.8h, v23.8h[5]
    umlal v7.4s, v32.4s, v23.8h[6]
    umlal2 v15.4s, v32.8h, v23.8h[6]
    umlal v7.4s, v33.4s, v23.8h[7]
    umlal2 v15.4s, v33.8h, v23.8h[7]
    umlal v8.4s, v26.4s, v24.8h[0]
    umlal2 v16.4s, v26.8h, v24.8h[0]
    umlal v8.4s, v27.4s, v24.8h[1]
    umlal2 v16.4s, v27.8h, v24.8h[1]
    umlal v8.4s, v28.4s, v24.8h[2]
    umlal2 v16.4s, v28.8h, v24.8h[2]
    umlal v8.4s, v29.4s, v24.8h[3]
    umlal2 v16.4s, v29.8h, v24.8h[3]
    umlal v8.4s, v30.4s, v24.8h[4]
    umlal2 v16.4s, v30.8h, v24.8h[4]
    umlal v8.4s, v31.4s, v24.8h[5]
    umlal2 v16.4s, v31.8h, v24.8h[5]
    umlal v8.4s, v32.4s, v24.8h[6]
    umlal2 v16.4s, v32.8h, v24.8h[6]
    umlal v8.4s, v33.4s, v24.8h[7]
    umlal2 v16.4s, v33.8h, v24.8h[7]
    umlal v9.4s, v26.4s, v25.8h[0]
    umlal2 v17.4s, v26.8h, v25.8h[0]
    umlal v9.4s, v27.4s, v25.8h[1]
    umlal2 v17.4s, v27.8h, v25.8h[1]
    umlal v9.4s, v28.4s, v25.8h[2]
    umlal2 v17.4s, v28.8h, v25.8h[2]
    umlal v9.4s, v29.4s, v25.8h[3]
    umlal2 v17.4s, v29.8h, v25.8h[3]
    umlal v9.4s, v30.4s, v25.8h[4]
    umlal2 v17.4s, v30.8h, v25.8h[4]
    umlal v9.4s, v31.4s, v25.8h[5]
    umlal2 v17.4s, v31.8h, v25.8h[5]
    umlal v9.4s, v32.4s, v25.8h[6]
    umlal2 v17.4s, v32.8h, v25.8h[6]
    umlal v9.4s, v33.4s, v25.8h[7]
    umlal2 v17.4s, v33.8h, v25.8h[7]
k_loop_end:
    add x7, x7, #8
    cmp x7, #24
    blt k_loop_2
    umull v26.2d, v2.2s, v0.2s
    umull2 v27.2d, v2.4s, v0.4s
    uzp2 v26.4s, v26.4s, v27.4s
    ushr v26.4s, v26.4s, 11
    mls v2.4s, v26.4s, v1.4s
    umull v26.2d, v10.2s, v0.2s
    umull2 v27.2d, v10.4s, v0.4s
    uzp2 v26.4s, v26.4s, v27.4s
    ushr v26.4s, v26.4s, 11
    mls v10.4s, v26.4s, v1.4s
    uzp1 v18.8h, v2.8h, v10.8h
    umull v26.2d, v3.2s, v0.2s
    umull2 v27.2d, v3.4s, v0.4s
    uzp2 v26.4s, v26.4s, v27.4s
    ushr v26.4s, v26.4s, 11
    mls v3.4s, v26.4s, v1.4s
    umull v26.2d, v11.2s, v0.2s
    umull2 v27.2d, v11.4s, v0.4s
    uzp2 v26.4s, v26.4s, v27.4s
    ushr v26.4s, v26.4s, 11
    mls v11.4s, v26.4s, v1.4s
    uzp1 v19.8h, v3.8h, v11.8h
    umull v26.2d, v4.2s, v0.2s
    umull2 v27.2d, v4.4s, v0.4s
    uzp2 v26.4s, v26.4s, v27.4s
    ushr v26.4s, v26.4s, 11
    mls v4.4s, v26.4s, v1.4s
    umull v26.2d, v12.2s, v0.2s
    umull2 v27.2d, v12.4s, v0.4s
    uzp2 v26.4s, v26.4s, v27.4s
    ushr v26.4s, v26.4s, 11
    mls v12.4s, v26.4s, v1.4s
    uzp1 v20.8h, v4.8h, v12.8h
    umull v26.2d, v5.2s, v0.2s
    umull2 v27.2d, v5.4s, v0.4s
    uzp2 v26.4s, v26.4s, v27.4s
    ushr v26.4s, v26.4s, 11
    mls v5.4s, v26.4s, v1.4s
    umull v26.2d, v13.2s, v0.2s
    umull2 v27.2d, v13.4s, v0.4s
    uzp2 v26.4s, v26.4s, v27.4s
    ushr v26.4s, v26.4s, 11
    mls v13.4s, v26.4s, v1.4s
    uzp1 v21.8h, v5.8h, v13.8h
    umull v26.2d, v6.2s, v0.2s
    umull2 v27.2d, v6.4s, v0.4s
    uzp2 v26.4s, v26.4s, v27.4s
    ushr v26.4s, v26.4s, 11
    mls v6.4s, v26.4s, v1.4s
    umull v26.2d, v14.2s, v0.2s
    umull2 v27.2d, v14.4s, v0.4s
    uzp2 v26.4s, v26.4s, v27.4s
    ushr v26.4s, v26.4s, 11
    mls v14.4s, v26.4s, v1.4s
    uzp1 v22.8h, v6.8h, v14.8h
    umull v26.2d, v7.2s, v0.2s
    umull2 v27.2d, v7.4s, v0.4s
    uzp2 v26.4s, v26.4s, v27.4s
    ushr v26.4s, v26.4s, 11
    mls v7.4s, v26.4s, v1.4s
    umull v26.2d, v15.2s, v0.2s
    umull2 v27.2d, v15.4s, v0.4s
    uzp2 v26.4s, v26.4s, v27.4s
    ushr v26.4s, v26.4s, 11
    mls v15.4s, v26.4s, v1.4s
    uzp1 v23.8h, v7.8h, v15.8h
    umull v26.2d, v8.2s, v0.2s
    umull2 v27.2d, v8.4s, v0.4s
    uzp2 v26.4s, v26.4s, v27.4s
    ushr v26.4s, v26.4s, 11
    mls v8.4s, v26.4s, v1.4s
    umull v26.2d, v16.2s, v0.2s
    umull2 v27.2d, v16.4s, v0.4s
    uzp2 v26.4s, v26.4s, v27.4s
    ushr v26.4s, v26.4s, 11
    mls v16.4s, v26.4s, v1.4s
    uzp1 v24.8h, v8.8h, v16.8h
    umull v26.2d, v9.2s, v0.2s
    umull2 v27.2d, v9.4s, v0.4s
    uzp2 v26.4s, v26.4s, v27.4s
    ushr v26.4s, v26.4s, 11
    mls v9.4s, v26.4s, v1.4s
    umull v26.2d, v17.2s, v0.2s
    umull2 v27.2d, v17.4s, v0.4s
    uzp2 v26.4s, v26.4s, v27.4s
    ushr v26.4s, v26.4s, 11
    mls v17.4s, v26.4s, v1.4s
    uzp1 v25.8h, v9.8h, v17.8h
    add x10, x0, x6, lsl #1
    st1 {v18.8h}, [x10], x4
    st1 {v18.8h}, [x10], x4
    st1 {v19.8h}, [x10], x4
    st1 {v20.8h}, [x10], x4
    st1 {v21.8h}, [x10], x4
    st1 {v22.8h}, [x10], x4
    st1 {v23.8h}, [x10], x4
    st1 {v24.8h}, [x10], x4
    st1 {v25.8h}, [x10]
c_loop_end:
    add x6, x6, #8
    cmp x6, #24
    blt c_loop
r_loop_end:
    add x5, x5, #8
    add x0, x0, x4, lsl #2
    cmp x5, #24
    blt r_loop
    ret
