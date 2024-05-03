.cpu cortex-a72
.arch armv8-a
.global pmod_mat_sys_part
pmod_mat_sys_part:
    // Load MEDS_p into v7
    mov w10, #4093
    dup v7.4s, w10
    // r2 = r + 1
    add w4, w3, 1
    // r2 < M_r
    cmp w4, w1
    bge outer_loop_end
outer_loop:
    // Mrr = M[M_c * r + r]
    // madd x5, x2, x3, x3
    // ldr d0, [x0, x5, lsl #3]
    // // Mrr0 = vceqz_u16(Mrr)
    // cmeq v1.4h, v0.4h, #0
    // c = r
    mov w6, w3
    cmp w6, w2
    bge inner_loop_end
inner_loop:
    // // val = M[M_c * r2 + c]
    // madd x5, x2, x3, x4
    // ldr d2, [x0, x5, lsl #3]
    // // Mrc = M[M_c * r + c]
    // madd w5, w2, w3, w6
    // add x5, x0, x5, lsl #3
    // ldr d3, [x5]
    // // and = vand_u16(val, Mrr0)
    // and v4.16b, v2.16b, v1.16b
    // // add = vaddl_u16(Mrc, and)
    // uaddl v5.4s, v3.4h, v4.4h
    // // red = FREEZE_REDUCE_VEC(add)
    // ushr v6.4s, v5.4s, #12
    // mul v6.4s, v6.4s, v7.4s
    // sub v5.4s, v5.4s, v6.4s
    // ushr v6.4s, v5.4s, #12
    // mul v6.4s, v6.4s, v7.4s
    // sub v5.4s, v5.4s, v6.4s
    // cmhs v6.4s, v5.4s, v7.4s
    // and v6.16b, v6.16b, v7.16b
    // sub v5.4s, v5.4s, v6.4s
    // sqxtn v5.4h, v5.4s
    // M[M_c * r + c] = red
    // madd w5, w2, w3, w6
    // str d5, [x0, x5, lsl #3]
    // Loop back
    add w6, w6, 1
    cmp w6, w2
    blt inner_loop
inner_loop_end:
    // Loop back
    add w4, w4, 1
    cmp w4, w1
    blt outer_loop
outer_loop_end:
    ret
