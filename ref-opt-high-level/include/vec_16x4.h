#ifndef VEC_16X4_H
#define VEC_16X4_H

#include <arm_neon.h>

#include "params.h"

// TODO: aligned?
// #define aligned __attribute__((aligned(16)))

// UNSIGNED INTEGER INSTRUCTIONS & DATA TYPES
#define ADD_LOW_VEC(a, b) vadd_u16(a, b)
#define ADD_VEC(a, b) vaddl_u16(a, b)
#define SUB_LOW_VEC(a, b) vsub_u16(a, b)
#define SUB_VEC(a, b) vsubl_u16(a, b)
// #define ADD_VEC_W(a, b) vaddq_u32(a, b)
#define MUL_VEC(a, b) vmull_u16(a, b)
#define MUL_ACC_VEC(a, b, c) vmlal_u16(a, b, c)
#define OR_VEC(a, b) vorr_u16(a, b)
#define AND_VEC(a, b) vand_u16(a, b)
#define XOR_VEC(a, b) veor_u16(a, b)

// #define EQ_VEC(a, b) vceq_u16(a, b)
#define EQ0_VEC(a) vceqz_u16(a)
#define EQ_VEC(a, b) vceq_u16(a, b)

#define SET_VEC(a) vdup_n_u16(a)
#define TO_U_VEC(a) vreinterpret_u16_s16(a)

#define MEDS_p_VEC vdup_n_u16(MEDS_p)
#define ZERO_VEC vdup_n_u16(0)
#define ZERO_VEC_W vdupq_n_u32(0)

#define GFq_vec_t uint16x4_t
#define GFq_vec_w_t uint32x4_t
#define pmod_mat_vec_t GFq_vec_t
#define pmod_mat_vec_w_t GFq_vec_w_t

// SIGNED INSTRUCTIONS & DATA TYPES
#define ADD_S_VEC(a, b) vadd_s16(a, b)
// #define MUL_LOW_S_VEC(a, b) vmul_s16(a, b)
#define AND_S_VEC(a, b) vand_s16(a, b)
#define XOR_S_VEC(a, b) veor_s16(a, b)
#define NOT_S_VEC(a) vmvn_s16(a)

#define EQ0_S_VEC(a) vceqz_s16(a)
#define EQ_S_VEC(a, b) vceq_s16(a, b)
#define GEQ_S_VEC(a, b) vcge_s16(a, b)

#define SET_S_VEC(a) vdup_n_s16(a)
#define TO_S_VEC(a) vreinterpret_s16_u16(a)

#define ZERO_S_VEC vdup_n_s16(0)

#define GET_LANE_S_VEC(a, i) vget_lane_s16(a, i)

#define pmod_mat_s_vec_t int16x4_t

// DEFINITIONS
#define BATCH_SIZE 4
#define VEC_TRUE 0xffff
#define VEC_FALSE 0x0000

// FUNCTIONS
// uint16x4_t reduce_vec(uint32x4_t a);
// uint16x4_t freeze_vec(uint16x4_t a);

#define REDUCE(val) val % MEDS_p;

#define REDUCE_VEC(v1)                \
  ({                                  \
    uint32x4_t _t1;                   \
    uint32x4_t _v1 = v1;              \
    _t1 = vshrq_n_u32(_v1, GFq_bits); \
    _t1 = vmulq_n_u32(_t1, MEDS_p);   \
    _v1 = vsubq_u32(_v1, _t1);        \
    _t1 = vshrq_n_u32(_v1, GFq_bits); \
    _t1 = vmulq_n_u32(_t1, MEDS_p);   \
    _v1 = vsubq_u32(_v1, _t1);        \
    vqmovn_u32(_v1);                  \
  })

#define FREEZE_VEC(_v2)              \
  ({                                 \
    uint16x4_t _t2;                  \
    _t2 = vcge_u16(_v2, MEDS_p_VEC); \
    _t2 = vand_u16(_t2, MEDS_p_VEC); \
    vsub_u16(_v2, _t2);              \
  })

#define FREEZE_REDUCE_VEC(_v3)        \
  ({                                  \
    uint16x4_t _t3 = REDUCE_VEC(_v3); \
    FREEZE_VEC(_t3);                  \
  })

pmod_mat_vec_t load_vec(uint16_t *M[], int M_r, int M_c, int r, int c);
void store_vec(uint16_t *M[], int M_r, int M_c, int r, int c, pmod_mat_vec_t val);

#endif