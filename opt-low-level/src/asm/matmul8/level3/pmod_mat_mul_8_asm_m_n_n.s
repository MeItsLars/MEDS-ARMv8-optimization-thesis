.cpu cortex-a72
.arch armv8-a
.global pmod_mat_mul_8_asm_m_n_n
pmod_mat_mul_8_asm_m_n_n:
    mov w10, 0x0481
    movk w10, 0x8018, lsl #16
    dup v0.4s, w10
    mov w10, #4093
    dup v1.4s, w10
    mov x3, #70
    mov x4, #70
    mov x5, #0
r_loop:
    mov x6, #0
c_loop:
    mov x7, #0
k_loop_1:
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
    ld1 {v2.8h}, [x10], x3
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
    ld1 {v2.8h}, [x10], x3
    umull v5.4s, v19.4h, v2.h[0]
    umull2 v13.4s, v19.8h, v2.h[0]
    umlal v5.4s, v20.4h, v2.h[1]
    umlal2 v13.4s, v20.8h, v2.h[1]
    umlal v5.4s, v21.4h, v2.h[2]
    umlal2 v13.4s, v21.8h, v2.h[2]
    umlal v5.4s, v22.4h, v2.h[3]
    umlal2 v13.4s, v22.8h, v2.h[3]
    umlal v5.4s, v23.4h, v2.h[4]
    umlal2 v13.4s, v23.8h, v2.h[4]
    umlal v5.4s, v24.4h, v2.h[5]
    umlal2 v13.4s, v24.8h, v2.h[5]
    umlal v5.4s, v25.4h, v2.h[6]
    umlal2 v13.4s, v25.8h, v2.h[6]
    umlal v5.4s, v26.4h, v2.h[7]
    umlal2 v13.4s, v26.8h, v2.h[7]
    ld1 {v2.8h}, [x10], x3
    umull v6.4s, v19.4h, v2.h[0]
    umull2 v14.4s, v19.8h, v2.h[0]
    umlal v6.4s, v20.4h, v2.h[1]
    umlal2 v14.4s, v20.8h, v2.h[1]
    umlal v6.4s, v21.4h, v2.h[2]
    umlal2 v14.4s, v21.8h, v2.h[2]
    umlal v6.4s, v22.4h, v2.h[3]
    umlal2 v14.4s, v22.8h, v2.h[3]
    umlal v6.4s, v23.4h, v2.h[4]
    umlal2 v14.4s, v23.8h, v2.h[4]
    umlal v6.4s, v24.4h, v2.h[5]
    umlal2 v14.4s, v24.8h, v2.h[5]
    umlal v6.4s, v25.4h, v2.h[6]
    umlal2 v14.4s, v25.8h, v2.h[6]
    umlal v6.4s, v26.4h, v2.h[7]
    umlal2 v14.4s, v26.8h, v2.h[7]
    ld1 {v2.8h}, [x10], x3
    umull v7.4s, v19.4h, v2.h[0]
    umull2 v15.4s, v19.8h, v2.h[0]
    umlal v7.4s, v20.4h, v2.h[1]
    umlal2 v15.4s, v20.8h, v2.h[1]
    umlal v7.4s, v21.4h, v2.h[2]
    umlal2 v15.4s, v21.8h, v2.h[2]
    umlal v7.4s, v22.4h, v2.h[3]
    umlal2 v15.4s, v22.8h, v2.h[3]
    umlal v7.4s, v23.4h, v2.h[4]
    umlal2 v15.4s, v23.8h, v2.h[4]
    umlal v7.4s, v24.4h, v2.h[5]
    umlal2 v15.4s, v24.8h, v2.h[5]
    umlal v7.4s, v25.4h, v2.h[6]
    umlal2 v15.4s, v25.8h, v2.h[6]
    umlal v7.4s, v26.4h, v2.h[7]
    umlal2 v15.4s, v26.8h, v2.h[7]
    ld1 {v2.8h}, [x10], x3
    umull v8.4s, v19.4h, v2.h[0]
    umull2 v16.4s, v19.8h, v2.h[0]
    umlal v8.4s, v20.4h, v2.h[1]
    umlal2 v16.4s, v20.8h, v2.h[1]
    umlal v8.4s, v21.4h, v2.h[2]
    umlal2 v16.4s, v21.8h, v2.h[2]
    umlal v8.4s, v22.4h, v2.h[3]
    umlal2 v16.4s, v22.8h, v2.h[3]
    umlal v8.4s, v23.4h, v2.h[4]
    umlal2 v16.4s, v23.8h, v2.h[4]
    umlal v8.4s, v24.4h, v2.h[5]
    umlal2 v16.4s, v24.8h, v2.h[5]
    umlal v8.4s, v25.4h, v2.h[6]
    umlal2 v16.4s, v25.8h, v2.h[6]
    umlal v8.4s, v26.4h, v2.h[7]
    umlal2 v16.4s, v26.8h, v2.h[7]
    ld1 {v2.8h}, [x10], x3
    umull v9.4s, v19.4h, v2.h[0]
    umull2 v17.4s, v19.8h, v2.h[0]
    umlal v9.4s, v20.4h, v2.h[1]
    umlal2 v17.4s, v20.8h, v2.h[1]
    umlal v9.4s, v21.4h, v2.h[2]
    umlal2 v17.4s, v21.8h, v2.h[2]
    umlal v9.4s, v22.4h, v2.h[3]
    umlal2 v17.4s, v22.8h, v2.h[3]
    umlal v9.4s, v23.4h, v2.h[4]
    umlal2 v17.4s, v23.8h, v2.h[4]
    umlal v9.4s, v24.4h, v2.h[5]
    umlal2 v17.4s, v24.8h, v2.h[5]
    umlal v9.4s, v25.4h, v2.h[6]
    umlal2 v17.4s, v25.8h, v2.h[6]
    umlal v9.4s, v26.4h, v2.h[7]
    umlal2 v17.4s, v26.8h, v2.h[7]
    ld1 {v2.8h}, [x10]
    umull v10.4s, v19.4h, v2.h[0]
    umull2 v18.4s, v19.8h, v2.h[0]
    umlal v10.4s, v20.4h, v2.h[1]
    umlal2 v18.4s, v20.8h, v2.h[1]
    umlal v10.4s, v21.4h, v2.h[2]
    umlal2 v18.4s, v21.8h, v2.h[2]
    umlal v10.4s, v22.4h, v2.h[3]
    umlal2 v18.4s, v22.8h, v2.h[3]
    umlal v10.4s, v23.4h, v2.h[4]
    umlal2 v18.4s, v23.8h, v2.h[4]
    umlal v10.4s, v24.4h, v2.h[5]
    umlal2 v18.4s, v24.8h, v2.h[5]
    umlal v10.4s, v25.4h, v2.h[6]
    umlal2 v18.4s, v25.8h, v2.h[6]
    umlal v10.4s, v26.4h, v2.h[7]
    umlal2 v18.4s, v26.8h, v2.h[7]
    b k_loop_end
k_loop_2:
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
    ld1 {v2.8h}, [x10], x3
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
    ld1 {v2.8h}, [x10], x3
    umlal v5.4s, v19.4h, v2.h[0]
    umlal2 v13.4s, v19.8h, v2.h[0]
    umlal v5.4s, v20.4h, v2.h[1]
    umlal2 v13.4s, v20.8h, v2.h[1]
    umlal v5.4s, v21.4h, v2.h[2]
    umlal2 v13.4s, v21.8h, v2.h[2]
    umlal v5.4s, v22.4h, v2.h[3]
    umlal2 v13.4s, v22.8h, v2.h[3]
    umlal v5.4s, v23.4h, v2.h[4]
    umlal2 v13.4s, v23.8h, v2.h[4]
    umlal v5.4s, v24.4h, v2.h[5]
    umlal2 v13.4s, v24.8h, v2.h[5]
    umlal v5.4s, v25.4h, v2.h[6]
    umlal2 v13.4s, v25.8h, v2.h[6]
    umlal v5.4s, v26.4h, v2.h[7]
    umlal2 v13.4s, v26.8h, v2.h[7]
    ld1 {v2.8h}, [x10], x3
    umlal v6.4s, v19.4h, v2.h[0]
    umlal2 v14.4s, v19.8h, v2.h[0]
    umlal v6.4s, v20.4h, v2.h[1]
    umlal2 v14.4s, v20.8h, v2.h[1]
    umlal v6.4s, v21.4h, v2.h[2]
    umlal2 v14.4s, v21.8h, v2.h[2]
    umlal v6.4s, v22.4h, v2.h[3]
    umlal2 v14.4s, v22.8h, v2.h[3]
    umlal v6.4s, v23.4h, v2.h[4]
    umlal2 v14.4s, v23.8h, v2.h[4]
    umlal v6.4s, v24.4h, v2.h[5]
    umlal2 v14.4s, v24.8h, v2.h[5]
    umlal v6.4s, v25.4h, v2.h[6]
    umlal2 v14.4s, v25.8h, v2.h[6]
    umlal v6.4s, v26.4h, v2.h[7]
    umlal2 v14.4s, v26.8h, v2.h[7]
    ld1 {v2.8h}, [x10], x3
    umlal v7.4s, v19.4h, v2.h[0]
    umlal2 v15.4s, v19.8h, v2.h[0]
    umlal v7.4s, v20.4h, v2.h[1]
    umlal2 v15.4s, v20.8h, v2.h[1]
    umlal v7.4s, v21.4h, v2.h[2]
    umlal2 v15.4s, v21.8h, v2.h[2]
    umlal v7.4s, v22.4h, v2.h[3]
    umlal2 v15.4s, v22.8h, v2.h[3]
    umlal v7.4s, v23.4h, v2.h[4]
    umlal2 v15.4s, v23.8h, v2.h[4]
    umlal v7.4s, v24.4h, v2.h[5]
    umlal2 v15.4s, v24.8h, v2.h[5]
    umlal v7.4s, v25.4h, v2.h[6]
    umlal2 v15.4s, v25.8h, v2.h[6]
    umlal v7.4s, v26.4h, v2.h[7]
    umlal2 v15.4s, v26.8h, v2.h[7]
    ld1 {v2.8h}, [x10], x3
    umlal v8.4s, v19.4h, v2.h[0]
    umlal2 v16.4s, v19.8h, v2.h[0]
    umlal v8.4s, v20.4h, v2.h[1]
    umlal2 v16.4s, v20.8h, v2.h[1]
    umlal v8.4s, v21.4h, v2.h[2]
    umlal2 v16.4s, v21.8h, v2.h[2]
    umlal v8.4s, v22.4h, v2.h[3]
    umlal2 v16.4s, v22.8h, v2.h[3]
    umlal v8.4s, v23.4h, v2.h[4]
    umlal2 v16.4s, v23.8h, v2.h[4]
    umlal v8.4s, v24.4h, v2.h[5]
    umlal2 v16.4s, v24.8h, v2.h[5]
    umlal v8.4s, v25.4h, v2.h[6]
    umlal2 v16.4s, v25.8h, v2.h[6]
    umlal v8.4s, v26.4h, v2.h[7]
    umlal2 v16.4s, v26.8h, v2.h[7]
    ld1 {v2.8h}, [x10], x3
    umlal v9.4s, v19.4h, v2.h[0]
    umlal2 v17.4s, v19.8h, v2.h[0]
    umlal v9.4s, v20.4h, v2.h[1]
    umlal2 v17.4s, v20.8h, v2.h[1]
    umlal v9.4s, v21.4h, v2.h[2]
    umlal2 v17.4s, v21.8h, v2.h[2]
    umlal v9.4s, v22.4h, v2.h[3]
    umlal2 v17.4s, v22.8h, v2.h[3]
    umlal v9.4s, v23.4h, v2.h[4]
    umlal2 v17.4s, v23.8h, v2.h[4]
    umlal v9.4s, v24.4h, v2.h[5]
    umlal2 v17.4s, v24.8h, v2.h[5]
    umlal v9.4s, v25.4h, v2.h[6]
    umlal2 v17.4s, v25.8h, v2.h[6]
    umlal v9.4s, v26.4h, v2.h[7]
    umlal2 v17.4s, v26.8h, v2.h[7]
    ld1 {v2.8h}, [x10]
    umlal v10.4s, v19.4h, v2.h[0]
    umlal2 v18.4s, v19.8h, v2.h[0]
    umlal v10.4s, v20.4h, v2.h[1]
    umlal2 v18.4s, v20.8h, v2.h[1]
    umlal v10.4s, v21.4h, v2.h[2]
    umlal2 v18.4s, v21.8h, v2.h[2]
    umlal v10.4s, v22.4h, v2.h[3]
    umlal2 v18.4s, v22.8h, v2.h[3]
    umlal v10.4s, v23.4h, v2.h[4]
    umlal2 v18.4s, v23.8h, v2.h[4]
    umlal v10.4s, v24.4h, v2.h[5]
    umlal2 v18.4s, v24.8h, v2.h[5]
    umlal v10.4s, v25.4h, v2.h[6]
    umlal2 v18.4s, v25.8h, v2.h[6]
    umlal v10.4s, v26.4h, v2.h[7]
    umlal2 v18.4s, v26.8h, v2.h[7]
k_loop_end:
    add x7, x7, #8
    cmp x7, #32
    blt k_loop_2
k_loop_pk:
    add x8, x8, #16
    add x9, x9, x4, lsl #3
    ld1 {v19.8h}, [x9]
    add x11, x9, x4
    ld1 {v20.8h}, [x11], x4
    ld1 {v21.8h}, [x11]
    ld1 {v2.8h}, [x8]
    add x10, x8, x3
    umlal v3.4s, v19.4h, v2.h[0]
    umlal2 v11.4s, v19.8h, v2.h[0]
    umlal v3.4s, v20.4h, v2.h[1]
    umlal2 v11.4s, v20.8h, v2.h[1]
    umlal v3.4s, v21.4h, v2.h[2]
    umlal2 v11.4s, v21.8h, v2.h[2]
    ld1 {v2.8h}, [x10], x3
    umlal v4.4s, v19.4h, v2.h[0]
    umlal2 v12.4s, v19.8h, v2.h[0]
    umlal v4.4s, v20.4h, v2.h[1]
    umlal2 v12.4s, v20.8h, v2.h[1]
    umlal v4.4s, v21.4h, v2.h[2]
    umlal2 v12.4s, v21.8h, v2.h[2]
    ld1 {v2.8h}, [x10], x3
    umlal v5.4s, v19.4h, v2.h[0]
    umlal2 v13.4s, v19.8h, v2.h[0]
    umlal v5.4s, v20.4h, v2.h[1]
    umlal2 v13.4s, v20.8h, v2.h[1]
    umlal v5.4s, v21.4h, v2.h[2]
    umlal2 v13.4s, v21.8h, v2.h[2]
    ld1 {v2.8h}, [x10], x3
    umlal v6.4s, v19.4h, v2.h[0]
    umlal2 v14.4s, v19.8h, v2.h[0]
    umlal v6.4s, v20.4h, v2.h[1]
    umlal2 v14.4s, v20.8h, v2.h[1]
    umlal v6.4s, v21.4h, v2.h[2]
    umlal2 v14.4s, v21.8h, v2.h[2]
    ld1 {v2.8h}, [x10], x3
    umlal v7.4s, v19.4h, v2.h[0]
    umlal2 v15.4s, v19.8h, v2.h[0]
    umlal v7.4s, v20.4h, v2.h[1]
    umlal2 v15.4s, v20.8h, v2.h[1]
    umlal v7.4s, v21.4h, v2.h[2]
    umlal2 v15.4s, v21.8h, v2.h[2]
    ld1 {v2.8h}, [x10], x3
    umlal v8.4s, v19.4h, v2.h[0]
    umlal2 v16.4s, v19.8h, v2.h[0]
    umlal v8.4s, v20.4h, v2.h[1]
    umlal2 v16.4s, v20.8h, v2.h[1]
    umlal v8.4s, v21.4h, v2.h[2]
    umlal2 v16.4s, v21.8h, v2.h[2]
    ld1 {v2.8h}, [x10], x3
    umlal v9.4s, v19.4h, v2.h[0]
    umlal2 v17.4s, v19.8h, v2.h[0]
    umlal v9.4s, v20.4h, v2.h[1]
    umlal2 v17.4s, v20.8h, v2.h[1]
    umlal v9.4s, v21.4h, v2.h[2]
    umlal2 v17.4s, v21.8h, v2.h[2]
    dup v2.8h, wzr
    ldrh w12, [x10, #0]
    ins v2.h[0], w12
    ldrh w12, [x10, #2]
    ins v2.h[1], w12
    ldrh w12, [x10, #4]
    ins v2.h[2], w12
    umlal v10.4s, v19.4h, v2.h[0]
    umlal2 v18.4s, v19.8h, v2.h[0]
    umlal v10.4s, v20.4h, v2.h[1]
    umlal2 v18.4s, v20.8h, v2.h[1]
    umlal v10.4s, v21.4h, v2.h[2]
    umlal2 v18.4s, v21.8h, v2.h[2]
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
    umull v27.2d, v5.2s, v0.2s
    umull2 v28.2d, v5.4s, v0.4s
    uzp2 v27.4s, v27.4s, v28.4s
    ushr v27.4s, v27.4s, 11
    mls v5.4s, v27.4s, v1.4s
    umull v27.2d, v13.2s, v0.2s
    umull2 v28.2d, v13.4s, v0.4s
    uzp2 v27.4s, v27.4s, v28.4s
    ushr v27.4s, v27.4s, 11
    mls v13.4s, v27.4s, v1.4s
    uzp1 v21.8h, v5.8h, v13.8h
    umull v27.2d, v6.2s, v0.2s
    umull2 v28.2d, v6.4s, v0.4s
    uzp2 v27.4s, v27.4s, v28.4s
    ushr v27.4s, v27.4s, 11
    mls v6.4s, v27.4s, v1.4s
    umull v27.2d, v14.2s, v0.2s
    umull2 v28.2d, v14.4s, v0.4s
    uzp2 v27.4s, v27.4s, v28.4s
    ushr v27.4s, v27.4s, 11
    mls v14.4s, v27.4s, v1.4s
    uzp1 v22.8h, v6.8h, v14.8h
    umull v27.2d, v7.2s, v0.2s
    umull2 v28.2d, v7.4s, v0.4s
    uzp2 v27.4s, v27.4s, v28.4s
    ushr v27.4s, v27.4s, 11
    mls v7.4s, v27.4s, v1.4s
    umull v27.2d, v15.2s, v0.2s
    umull2 v28.2d, v15.4s, v0.4s
    uzp2 v27.4s, v27.4s, v28.4s
    ushr v27.4s, v27.4s, 11
    mls v15.4s, v27.4s, v1.4s
    uzp1 v23.8h, v7.8h, v15.8h
    umull v27.2d, v8.2s, v0.2s
    umull2 v28.2d, v8.4s, v0.4s
    uzp2 v27.4s, v27.4s, v28.4s
    ushr v27.4s, v27.4s, 11
    mls v8.4s, v27.4s, v1.4s
    umull v27.2d, v16.2s, v0.2s
    umull2 v28.2d, v16.4s, v0.4s
    uzp2 v27.4s, v27.4s, v28.4s
    ushr v27.4s, v27.4s, 11
    mls v16.4s, v27.4s, v1.4s
    uzp1 v24.8h, v8.8h, v16.8h
    umull v27.2d, v9.2s, v0.2s
    umull2 v28.2d, v9.4s, v0.4s
    uzp2 v27.4s, v27.4s, v28.4s
    ushr v27.4s, v27.4s, 11
    mls v9.4s, v27.4s, v1.4s
    umull v27.2d, v17.2s, v0.2s
    umull2 v28.2d, v17.4s, v0.4s
    uzp2 v27.4s, v27.4s, v28.4s
    ushr v27.4s, v27.4s, 11
    mls v17.4s, v27.4s, v1.4s
    uzp1 v25.8h, v9.8h, v17.8h
    umull v27.2d, v10.2s, v0.2s
    umull2 v28.2d, v10.4s, v0.4s
    uzp2 v27.4s, v27.4s, v28.4s
    ushr v27.4s, v27.4s, 11
    mls v10.4s, v27.4s, v1.4s
    umull v27.2d, v18.2s, v0.2s
    umull2 v28.2d, v18.4s, v0.4s
    uzp2 v27.4s, v27.4s, v28.4s
    ushr v27.4s, v27.4s, 11
    mls v18.4s, v27.4s, v1.4s
    uzp1 v26.8h, v10.8h, v18.8h
    add x10, x0, x6, lsl #1
    st1 {v19.8h}, [x10], x4
    st1 {v20.8h}, [x10], x4
    st1 {v21.8h}, [x10], x4
    st1 {v22.8h}, [x10], x4
    st1 {v23.8h}, [x10], x4
    st1 {v24.8h}, [x10], x4
    st1 {v25.8h}, [x10], x4
    st1 {v26.8h}, [x10]
c_loop_end:
    add x6, x6, #8
    cmp x6, #32
    blt c_loop
c_loop_pc:
    mov x7, #0
k_loop_pc_1:
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
    ldrh w12, [x11, #0]
    ins v26.h[0], w12
    ldrh w12, [x11, #2]
    ins v26.h[1], w12
    ldrh w12, [x11, #4]
    ins v26.h[2], w12
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
    ld1 {v2.8h}, [x10], x3
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
    ld1 {v2.8h}, [x10], x3
    umull v5.4s, v19.4h, v2.h[0]
    umull2 v13.4s, v19.8h, v2.h[0]
    umlal v5.4s, v20.4h, v2.h[1]
    umlal2 v13.4s, v20.8h, v2.h[1]
    umlal v5.4s, v21.4h, v2.h[2]
    umlal2 v13.4s, v21.8h, v2.h[2]
    umlal v5.4s, v22.4h, v2.h[3]
    umlal2 v13.4s, v22.8h, v2.h[3]
    umlal v5.4s, v23.4h, v2.h[4]
    umlal2 v13.4s, v23.8h, v2.h[4]
    umlal v5.4s, v24.4h, v2.h[5]
    umlal2 v13.4s, v24.8h, v2.h[5]
    umlal v5.4s, v25.4h, v2.h[6]
    umlal2 v13.4s, v25.8h, v2.h[6]
    umlal v5.4s, v26.4h, v2.h[7]
    umlal2 v13.4s, v26.8h, v2.h[7]
    ld1 {v2.8h}, [x10], x3
    umull v6.4s, v19.4h, v2.h[0]
    umull2 v14.4s, v19.8h, v2.h[0]
    umlal v6.4s, v20.4h, v2.h[1]
    umlal2 v14.4s, v20.8h, v2.h[1]
    umlal v6.4s, v21.4h, v2.h[2]
    umlal2 v14.4s, v21.8h, v2.h[2]
    umlal v6.4s, v22.4h, v2.h[3]
    umlal2 v14.4s, v22.8h, v2.h[3]
    umlal v6.4s, v23.4h, v2.h[4]
    umlal2 v14.4s, v23.8h, v2.h[4]
    umlal v6.4s, v24.4h, v2.h[5]
    umlal2 v14.4s, v24.8h, v2.h[5]
    umlal v6.4s, v25.4h, v2.h[6]
    umlal2 v14.4s, v25.8h, v2.h[6]
    umlal v6.4s, v26.4h, v2.h[7]
    umlal2 v14.4s, v26.8h, v2.h[7]
    ld1 {v2.8h}, [x10], x3
    umull v7.4s, v19.4h, v2.h[0]
    umull2 v15.4s, v19.8h, v2.h[0]
    umlal v7.4s, v20.4h, v2.h[1]
    umlal2 v15.4s, v20.8h, v2.h[1]
    umlal v7.4s, v21.4h, v2.h[2]
    umlal2 v15.4s, v21.8h, v2.h[2]
    umlal v7.4s, v22.4h, v2.h[3]
    umlal2 v15.4s, v22.8h, v2.h[3]
    umlal v7.4s, v23.4h, v2.h[4]
    umlal2 v15.4s, v23.8h, v2.h[4]
    umlal v7.4s, v24.4h, v2.h[5]
    umlal2 v15.4s, v24.8h, v2.h[5]
    umlal v7.4s, v25.4h, v2.h[6]
    umlal2 v15.4s, v25.8h, v2.h[6]
    umlal v7.4s, v26.4h, v2.h[7]
    umlal2 v15.4s, v26.8h, v2.h[7]
    ld1 {v2.8h}, [x10], x3
    umull v8.4s, v19.4h, v2.h[0]
    umull2 v16.4s, v19.8h, v2.h[0]
    umlal v8.4s, v20.4h, v2.h[1]
    umlal2 v16.4s, v20.8h, v2.h[1]
    umlal v8.4s, v21.4h, v2.h[2]
    umlal2 v16.4s, v21.8h, v2.h[2]
    umlal v8.4s, v22.4h, v2.h[3]
    umlal2 v16.4s, v22.8h, v2.h[3]
    umlal v8.4s, v23.4h, v2.h[4]
    umlal2 v16.4s, v23.8h, v2.h[4]
    umlal v8.4s, v24.4h, v2.h[5]
    umlal2 v16.4s, v24.8h, v2.h[5]
    umlal v8.4s, v25.4h, v2.h[6]
    umlal2 v16.4s, v25.8h, v2.h[6]
    umlal v8.4s, v26.4h, v2.h[7]
    umlal2 v16.4s, v26.8h, v2.h[7]
    ld1 {v2.8h}, [x10], x3
    umull v9.4s, v19.4h, v2.h[0]
    umull2 v17.4s, v19.8h, v2.h[0]
    umlal v9.4s, v20.4h, v2.h[1]
    umlal2 v17.4s, v20.8h, v2.h[1]
    umlal v9.4s, v21.4h, v2.h[2]
    umlal2 v17.4s, v21.8h, v2.h[2]
    umlal v9.4s, v22.4h, v2.h[3]
    umlal2 v17.4s, v22.8h, v2.h[3]
    umlal v9.4s, v23.4h, v2.h[4]
    umlal2 v17.4s, v23.8h, v2.h[4]
    umlal v9.4s, v24.4h, v2.h[5]
    umlal2 v17.4s, v24.8h, v2.h[5]
    umlal v9.4s, v25.4h, v2.h[6]
    umlal2 v17.4s, v25.8h, v2.h[6]
    umlal v9.4s, v26.4h, v2.h[7]
    umlal2 v17.4s, v26.8h, v2.h[7]
    ld1 {v2.8h}, [x10]
    umull v10.4s, v19.4h, v2.h[0]
    umull2 v18.4s, v19.8h, v2.h[0]
    umlal v10.4s, v20.4h, v2.h[1]
    umlal2 v18.4s, v20.8h, v2.h[1]
    umlal v10.4s, v21.4h, v2.h[2]
    umlal2 v18.4s, v21.8h, v2.h[2]
    umlal v10.4s, v22.4h, v2.h[3]
    umlal2 v18.4s, v22.8h, v2.h[3]
    umlal v10.4s, v23.4h, v2.h[4]
    umlal2 v18.4s, v23.8h, v2.h[4]
    umlal v10.4s, v24.4h, v2.h[5]
    umlal2 v18.4s, v24.8h, v2.h[5]
    umlal v10.4s, v25.4h, v2.h[6]
    umlal2 v18.4s, v25.8h, v2.h[6]
    umlal v10.4s, v26.4h, v2.h[7]
    umlal2 v18.4s, v26.8h, v2.h[7]
    b k_loop_pc_end
k_loop_pc_2:
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
    ldrh w12, [x11, #0]
    ins v26.h[0], w12
    ldrh w12, [x11, #2]
    ins v26.h[1], w12
    ldrh w12, [x11, #4]
    ins v26.h[2], w12
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
    ld1 {v2.8h}, [x10], x3
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
    ld1 {v2.8h}, [x10], x3
    umlal v5.4s, v19.4h, v2.h[0]
    umlal2 v13.4s, v19.8h, v2.h[0]
    umlal v5.4s, v20.4h, v2.h[1]
    umlal2 v13.4s, v20.8h, v2.h[1]
    umlal v5.4s, v21.4h, v2.h[2]
    umlal2 v13.4s, v21.8h, v2.h[2]
    umlal v5.4s, v22.4h, v2.h[3]
    umlal2 v13.4s, v22.8h, v2.h[3]
    umlal v5.4s, v23.4h, v2.h[4]
    umlal2 v13.4s, v23.8h, v2.h[4]
    umlal v5.4s, v24.4h, v2.h[5]
    umlal2 v13.4s, v24.8h, v2.h[5]
    umlal v5.4s, v25.4h, v2.h[6]
    umlal2 v13.4s, v25.8h, v2.h[6]
    umlal v5.4s, v26.4h, v2.h[7]
    umlal2 v13.4s, v26.8h, v2.h[7]
    ld1 {v2.8h}, [x10], x3
    umlal v6.4s, v19.4h, v2.h[0]
    umlal2 v14.4s, v19.8h, v2.h[0]
    umlal v6.4s, v20.4h, v2.h[1]
    umlal2 v14.4s, v20.8h, v2.h[1]
    umlal v6.4s, v21.4h, v2.h[2]
    umlal2 v14.4s, v21.8h, v2.h[2]
    umlal v6.4s, v22.4h, v2.h[3]
    umlal2 v14.4s, v22.8h, v2.h[3]
    umlal v6.4s, v23.4h, v2.h[4]
    umlal2 v14.4s, v23.8h, v2.h[4]
    umlal v6.4s, v24.4h, v2.h[5]
    umlal2 v14.4s, v24.8h, v2.h[5]
    umlal v6.4s, v25.4h, v2.h[6]
    umlal2 v14.4s, v25.8h, v2.h[6]
    umlal v6.4s, v26.4h, v2.h[7]
    umlal2 v14.4s, v26.8h, v2.h[7]
    ld1 {v2.8h}, [x10], x3
    umlal v7.4s, v19.4h, v2.h[0]
    umlal2 v15.4s, v19.8h, v2.h[0]
    umlal v7.4s, v20.4h, v2.h[1]
    umlal2 v15.4s, v20.8h, v2.h[1]
    umlal v7.4s, v21.4h, v2.h[2]
    umlal2 v15.4s, v21.8h, v2.h[2]
    umlal v7.4s, v22.4h, v2.h[3]
    umlal2 v15.4s, v22.8h, v2.h[3]
    umlal v7.4s, v23.4h, v2.h[4]
    umlal2 v15.4s, v23.8h, v2.h[4]
    umlal v7.4s, v24.4h, v2.h[5]
    umlal2 v15.4s, v24.8h, v2.h[5]
    umlal v7.4s, v25.4h, v2.h[6]
    umlal2 v15.4s, v25.8h, v2.h[6]
    umlal v7.4s, v26.4h, v2.h[7]
    umlal2 v15.4s, v26.8h, v2.h[7]
    ld1 {v2.8h}, [x10], x3
    umlal v8.4s, v19.4h, v2.h[0]
    umlal2 v16.4s, v19.8h, v2.h[0]
    umlal v8.4s, v20.4h, v2.h[1]
    umlal2 v16.4s, v20.8h, v2.h[1]
    umlal v8.4s, v21.4h, v2.h[2]
    umlal2 v16.4s, v21.8h, v2.h[2]
    umlal v8.4s, v22.4h, v2.h[3]
    umlal2 v16.4s, v22.8h, v2.h[3]
    umlal v8.4s, v23.4h, v2.h[4]
    umlal2 v16.4s, v23.8h, v2.h[4]
    umlal v8.4s, v24.4h, v2.h[5]
    umlal2 v16.4s, v24.8h, v2.h[5]
    umlal v8.4s, v25.4h, v2.h[6]
    umlal2 v16.4s, v25.8h, v2.h[6]
    umlal v8.4s, v26.4h, v2.h[7]
    umlal2 v16.4s, v26.8h, v2.h[7]
    ld1 {v2.8h}, [x10], x3
    umlal v9.4s, v19.4h, v2.h[0]
    umlal2 v17.4s, v19.8h, v2.h[0]
    umlal v9.4s, v20.4h, v2.h[1]
    umlal2 v17.4s, v20.8h, v2.h[1]
    umlal v9.4s, v21.4h, v2.h[2]
    umlal2 v17.4s, v21.8h, v2.h[2]
    umlal v9.4s, v22.4h, v2.h[3]
    umlal2 v17.4s, v22.8h, v2.h[3]
    umlal v9.4s, v23.4h, v2.h[4]
    umlal2 v17.4s, v23.8h, v2.h[4]
    umlal v9.4s, v24.4h, v2.h[5]
    umlal2 v17.4s, v24.8h, v2.h[5]
    umlal v9.4s, v25.4h, v2.h[6]
    umlal2 v17.4s, v25.8h, v2.h[6]
    umlal v9.4s, v26.4h, v2.h[7]
    umlal2 v17.4s, v26.8h, v2.h[7]
    ld1 {v2.8h}, [x10]
    umlal v10.4s, v19.4h, v2.h[0]
    umlal2 v18.4s, v19.8h, v2.h[0]
    umlal v10.4s, v20.4h, v2.h[1]
    umlal2 v18.4s, v20.8h, v2.h[1]
    umlal v10.4s, v21.4h, v2.h[2]
    umlal2 v18.4s, v21.8h, v2.h[2]
    umlal v10.4s, v22.4h, v2.h[3]
    umlal2 v18.4s, v22.8h, v2.h[3]
    umlal v10.4s, v23.4h, v2.h[4]
    umlal2 v18.4s, v23.8h, v2.h[4]
    umlal v10.4s, v24.4h, v2.h[5]
    umlal2 v18.4s, v24.8h, v2.h[5]
    umlal v10.4s, v25.4h, v2.h[6]
    umlal2 v18.4s, v25.8h, v2.h[6]
    umlal v10.4s, v26.4h, v2.h[7]
    umlal2 v18.4s, v26.8h, v2.h[7]
k_loop_pc_end:
    add x7, x7, #8
    cmp x7, #32
    blt k_loop_pc_2
k_loop_pc_pk:
    add x8, x8, #16
    add x9, x9, x4, lsl #3
    ld1 {v19.8h}, [x9]
    add x11, x9, x4
    ld1 {v20.8h}, [x11], x4
    dup v21.8h, wzr
    ldrh w12, [x11, #0]
    ins v21.h[0], w12
    ldrh w12, [x11, #2]
    ins v21.h[1], w12
    ldrh w12, [x11, #4]
    ins v21.h[2], w12
    ld1 {v2.8h}, [x8]
    add x10, x8, x3
    umlal v3.4s, v19.4h, v2.h[0]
    umlal2 v11.4s, v19.8h, v2.h[0]
    umlal v3.4s, v20.4h, v2.h[1]
    umlal2 v11.4s, v20.8h, v2.h[1]
    umlal v3.4s, v21.4h, v2.h[2]
    umlal2 v11.4s, v21.8h, v2.h[2]
    ld1 {v2.8h}, [x10], x3
    umlal v4.4s, v19.4h, v2.h[0]
    umlal2 v12.4s, v19.8h, v2.h[0]
    umlal v4.4s, v20.4h, v2.h[1]
    umlal2 v12.4s, v20.8h, v2.h[1]
    umlal v4.4s, v21.4h, v2.h[2]
    umlal2 v12.4s, v21.8h, v2.h[2]
    ld1 {v2.8h}, [x10], x3
    umlal v5.4s, v19.4h, v2.h[0]
    umlal2 v13.4s, v19.8h, v2.h[0]
    umlal v5.4s, v20.4h, v2.h[1]
    umlal2 v13.4s, v20.8h, v2.h[1]
    umlal v5.4s, v21.4h, v2.h[2]
    umlal2 v13.4s, v21.8h, v2.h[2]
    ld1 {v2.8h}, [x10], x3
    umlal v6.4s, v19.4h, v2.h[0]
    umlal2 v14.4s, v19.8h, v2.h[0]
    umlal v6.4s, v20.4h, v2.h[1]
    umlal2 v14.4s, v20.8h, v2.h[1]
    umlal v6.4s, v21.4h, v2.h[2]
    umlal2 v14.4s, v21.8h, v2.h[2]
    ld1 {v2.8h}, [x10], x3
    umlal v7.4s, v19.4h, v2.h[0]
    umlal2 v15.4s, v19.8h, v2.h[0]
    umlal v7.4s, v20.4h, v2.h[1]
    umlal2 v15.4s, v20.8h, v2.h[1]
    umlal v7.4s, v21.4h, v2.h[2]
    umlal2 v15.4s, v21.8h, v2.h[2]
    ld1 {v2.8h}, [x10], x3
    umlal v8.4s, v19.4h, v2.h[0]
    umlal2 v16.4s, v19.8h, v2.h[0]
    umlal v8.4s, v20.4h, v2.h[1]
    umlal2 v16.4s, v20.8h, v2.h[1]
    umlal v8.4s, v21.4h, v2.h[2]
    umlal2 v16.4s, v21.8h, v2.h[2]
    ld1 {v2.8h}, [x10], x3
    umlal v9.4s, v19.4h, v2.h[0]
    umlal2 v17.4s, v19.8h, v2.h[0]
    umlal v9.4s, v20.4h, v2.h[1]
    umlal2 v17.4s, v20.8h, v2.h[1]
    umlal v9.4s, v21.4h, v2.h[2]
    umlal2 v17.4s, v21.8h, v2.h[2]
    dup v2.8h, wzr
    ldrh w12, [x10, #0]
    ins v2.h[0], w12
    ldrh w12, [x10, #2]
    ins v2.h[1], w12
    ldrh w12, [x10, #4]
    ins v2.h[2], w12
    umlal v10.4s, v19.4h, v2.h[0]
    umlal2 v18.4s, v19.8h, v2.h[0]
    umlal v10.4s, v20.4h, v2.h[1]
    umlal2 v18.4s, v20.8h, v2.h[1]
    umlal v10.4s, v21.4h, v2.h[2]
    umlal2 v18.4s, v21.8h, v2.h[2]
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
    umull v27.2d, v5.2s, v0.2s
    umull2 v28.2d, v5.4s, v0.4s
    uzp2 v27.4s, v27.4s, v28.4s
    ushr v27.4s, v27.4s, 11
    mls v5.4s, v27.4s, v1.4s
    umull v27.2d, v13.2s, v0.2s
    umull2 v28.2d, v13.4s, v0.4s
    uzp2 v27.4s, v27.4s, v28.4s
    ushr v27.4s, v27.4s, 11
    mls v13.4s, v27.4s, v1.4s
    uzp1 v21.8h, v5.8h, v13.8h
    umull v27.2d, v6.2s, v0.2s
    umull2 v28.2d, v6.4s, v0.4s
    uzp2 v27.4s, v27.4s, v28.4s
    ushr v27.4s, v27.4s, 11
    mls v6.4s, v27.4s, v1.4s
    umull v27.2d, v14.2s, v0.2s
    umull2 v28.2d, v14.4s, v0.4s
    uzp2 v27.4s, v27.4s, v28.4s
    ushr v27.4s, v27.4s, 11
    mls v14.4s, v27.4s, v1.4s
    uzp1 v22.8h, v6.8h, v14.8h
    umull v27.2d, v7.2s, v0.2s
    umull2 v28.2d, v7.4s, v0.4s
    uzp2 v27.4s, v27.4s, v28.4s
    ushr v27.4s, v27.4s, 11
    mls v7.4s, v27.4s, v1.4s
    umull v27.2d, v15.2s, v0.2s
    umull2 v28.2d, v15.4s, v0.4s
    uzp2 v27.4s, v27.4s, v28.4s
    ushr v27.4s, v27.4s, 11
    mls v15.4s, v27.4s, v1.4s
    uzp1 v23.8h, v7.8h, v15.8h
    umull v27.2d, v8.2s, v0.2s
    umull2 v28.2d, v8.4s, v0.4s
    uzp2 v27.4s, v27.4s, v28.4s
    ushr v27.4s, v27.4s, 11
    mls v8.4s, v27.4s, v1.4s
    umull v27.2d, v16.2s, v0.2s
    umull2 v28.2d, v16.4s, v0.4s
    uzp2 v27.4s, v27.4s, v28.4s
    ushr v27.4s, v27.4s, 11
    mls v16.4s, v27.4s, v1.4s
    uzp1 v24.8h, v8.8h, v16.8h
    umull v27.2d, v9.2s, v0.2s
    umull2 v28.2d, v9.4s, v0.4s
    uzp2 v27.4s, v27.4s, v28.4s
    ushr v27.4s, v27.4s, 11
    mls v9.4s, v27.4s, v1.4s
    umull v27.2d, v17.2s, v0.2s
    umull2 v28.2d, v17.4s, v0.4s
    uzp2 v27.4s, v27.4s, v28.4s
    ushr v27.4s, v27.4s, 11
    mls v17.4s, v27.4s, v1.4s
    uzp1 v25.8h, v9.8h, v17.8h
    umull v27.2d, v10.2s, v0.2s
    umull2 v28.2d, v10.4s, v0.4s
    uzp2 v27.4s, v27.4s, v28.4s
    ushr v27.4s, v27.4s, 11
    mls v10.4s, v27.4s, v1.4s
    umull v27.2d, v18.2s, v0.2s
    umull2 v28.2d, v18.4s, v0.4s
    uzp2 v27.4s, v27.4s, v28.4s
    ushr v27.4s, v27.4s, 11
    mls v18.4s, v27.4s, v1.4s
    uzp1 v26.8h, v10.8h, v18.8h
    add x10, x0, x6, lsl #1
    umov w12, v19.h[0]
    strh w12, [x10, #0]
    umov w12, v19.h[1]
    strh w12, [x10, #2]
    umov w12, v19.h[2]
    strh w12, [x10, #4]
    add x10, x10, x4
    umov w12, v20.h[0]
    strh w12, [x10, #0]
    umov w12, v20.h[1]
    strh w12, [x10, #2]
    umov w12, v20.h[2]
    strh w12, [x10, #4]
    add x10, x10, x4
    umov w12, v21.h[0]
    strh w12, [x10, #0]
    umov w12, v21.h[1]
    strh w12, [x10, #2]
    umov w12, v21.h[2]
    strh w12, [x10, #4]
    add x10, x10, x4
    umov w12, v22.h[0]
    strh w12, [x10, #0]
    umov w12, v22.h[1]
    strh w12, [x10, #2]
    umov w12, v22.h[2]
    strh w12, [x10, #4]
    add x10, x10, x4
    umov w12, v23.h[0]
    strh w12, [x10, #0]
    umov w12, v23.h[1]
    strh w12, [x10, #2]
    umov w12, v23.h[2]
    strh w12, [x10, #4]
    add x10, x10, x4
    umov w12, v24.h[0]
    strh w12, [x10, #0]
    umov w12, v24.h[1]
    strh w12, [x10, #2]
    umov w12, v24.h[2]
    strh w12, [x10, #4]
    add x10, x10, x4
    umov w12, v25.h[0]
    strh w12, [x10, #0]
    umov w12, v25.h[1]
    strh w12, [x10, #2]
    umov w12, v25.h[2]
    strh w12, [x10, #4]
    add x10, x10, x4
    umov w12, v26.h[0]
    strh w12, [x10, #0]
    umov w12, v26.h[1]
    strh w12, [x10, #2]
    umov w12, v26.h[2]
    strh w12, [x10, #4]
r_loop_end:
    add x5, x5, #8
    add x0, x0, x4, lsl #3
    cmp x5, #32
    blt r_loop
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
    ld1 {v20.8h}, [x11], x4
    ld1 {v21.8h}, [x11]
    ld1 {v2.8h}, [x8]
    add x10, x8, x3
    umlal v3.4s, v19.4h, v2.h[0]
    umlal2 v11.4s, v19.8h, v2.h[0]
    umlal v3.4s, v20.4h, v2.h[1]
    umlal2 v11.4s, v20.8h, v2.h[1]
    umlal v3.4s, v21.4h, v2.h[2]
    umlal2 v11.4s, v21.8h, v2.h[2]
    dup v2.8h, wzr
    ldrh w12, [x10, #0]
    ins v2.h[0], w12
    ldrh w12, [x10, #2]
    ins v2.h[1], w12
    ldrh w12, [x10, #4]
    ins v2.h[2], w12
    umlal v4.4s, v19.4h, v2.h[0]
    umlal2 v12.4s, v19.8h, v2.h[0]
    umlal v4.4s, v20.4h, v2.h[1]
    umlal2 v12.4s, v20.8h, v2.h[1]
    umlal v4.4s, v21.4h, v2.h[2]
    umlal2 v12.4s, v21.8h, v2.h[2]
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
    cmp x6, #32
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
    ldrh w12, [x11, #0]
    ins v26.h[0], w12
    ldrh w12, [x11, #2]
    ins v26.h[1], w12
    ldrh w12, [x11, #4]
    ins v26.h[2], w12
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
    ldrh w12, [x11, #0]
    ins v26.h[0], w12
    ldrh w12, [x11, #2]
    ins v26.h[1], w12
    ldrh w12, [x11, #4]
    ins v26.h[2], w12
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
    ld1 {v20.8h}, [x11], x4
    dup v21.8h, wzr
    ldrh w12, [x11, #0]
    ins v21.h[0], w12
    ldrh w12, [x11, #2]
    ins v21.h[1], w12
    ldrh w12, [x11, #4]
    ins v21.h[2], w12
    ld1 {v2.8h}, [x8]
    add x10, x8, x3
    umlal v3.4s, v19.4h, v2.h[0]
    umlal2 v11.4s, v19.8h, v2.h[0]
    umlal v3.4s, v20.4h, v2.h[1]
    umlal2 v11.4s, v20.8h, v2.h[1]
    umlal v3.4s, v21.4h, v2.h[2]
    umlal2 v11.4s, v21.8h, v2.h[2]
    dup v2.8h, wzr
    ldrh w12, [x10, #0]
    ins v2.h[0], w12
    ldrh w12, [x10, #2]
    ins v2.h[1], w12
    ldrh w12, [x10, #4]
    ins v2.h[2], w12
    umlal v4.4s, v19.4h, v2.h[0]
    umlal2 v12.4s, v19.8h, v2.h[0]
    umlal v4.4s, v20.4h, v2.h[1]
    umlal2 v12.4s, v20.8h, v2.h[1]
    umlal v4.4s, v21.4h, v2.h[2]
    umlal2 v12.4s, v21.8h, v2.h[2]
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
    umov w12, v19.h[0]
    strh w12, [x10, #0]
    umov w12, v19.h[1]
    strh w12, [x10, #2]
    umov w12, v19.h[2]
    strh w12, [x10, #4]
    add x10, x10, x4
    umov w12, v20.h[0]
    strh w12, [x10, #0]
    umov w12, v20.h[1]
    strh w12, [x10, #2]
    umov w12, v20.h[2]
    strh w12, [x10, #4]
r_loop_pr_end:
    ret
