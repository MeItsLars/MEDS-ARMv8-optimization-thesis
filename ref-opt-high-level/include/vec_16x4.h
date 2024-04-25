#ifndef VEC_16X4_H
#define VEC_16X4_H

#include <arm_neon.h>

#include "params.h"

// TODO: aligned?
// #define aligned __attribute__((aligned(16)))

// UNSIGNED INTEGER INSTRUCTIONS & DATA TYPES
// #define ADD_LOW_VEC(a, b) vadd_u16(a, b)
#define ADD_VEC(a, b) vaddl_u16(a, b)
// #define ADD_VEC_W(a, b) vaddq_u32(a, b)
#define MUL_ACC_VEC(a, b, c) vmlal_u16(a, b, c)
#define OR_VEC(a, b) vorr_u16(a, b)
#define AND_VEC(a, b) vand_u16(a, b)
#define XOR_VEC(a, b) veor_u16(a, b)

// #define EQ_VEC(a, b) vceq_u16(a, b)
#define EQ0_VEC(a) vceqz_u16(a)

#define SET_VEC(a) vdup_n_u16(a)
#define TO_U_VEC(a) vreinterpret_u16_s16(a)

#define MEDS_p_VEC vdup_n_u16(MEDS_p)
#define ZERO_VEC vdup_n_u16(0)
#define ZERO_VEC_W vdupq_n_u32(0)

#define pmod_mat_vec_t uint16x4_t
#define pmod_mat_vec_w_t uint32x4_t

// SIGNED INSTRUCTIONS & DATA TYPES
#define ADD_S_VEC(a, b) vadd_s16(a, b)
// #define MUL_LOW_S_VEC(a, b) vmul_s16(a, b)
#define AND_S_VEC(a, b) vand_s16(a, b)
#define NOT_S_VEC(a) vmvn_s16(a)

#define SET_S_VEC(a) vdup_n_s16(a)
#define TO_S_VEC(a) vreinterpret_s16_u16(a)

#define pmod_mat_s_vec_t int16x4_t

uint16x4_t reduce_vec(uint32x4_t a);
uint16x4_t freeze_vec(uint16x4_t a);

#endif