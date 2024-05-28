.cpu cortex-a72
.arch armv8-a
.global pmod_mat_syst_test
pmod_mat_syst_test:
    mov w4, 0x0481
    movk w4, 0x8018, lsl #16
    dup v25.4s, w4
    mov x4, #4093
    dup v16.4h, w4
    dup v17.8h, w4
    dup v18.4s, w4
    ldr q3, [x0]
    mov x14, #1000
    dup v7.8h, w14
    umull v19.4s, v3.4h, v7.4h
    umull2 v20.4s, v3.8h, v7.8h
    umull v21.2d, v19.2s, v25.2s
    umull2 v22.2d, v19.4s, v25.4s
    umull v23.2d, v20.2s, v25.2s
    umull2 v24.2d, v20.4s, v25.4s
    uzp2 v21.4s, v21.4s, v22.4s
    uzp2 v23.4s, v23.4s, v24.4s
    ushr v21.4s, v21.4s, 11
    ushr v23.4s, v23.4s, 11
    mls v19.4s, v21.4s, v18.4s
    mls v20.4s, v23.4s, v18.4s
    uzp1 v3.8h, v19.8h, v20.8h
    str q3, [x0]
    ret
