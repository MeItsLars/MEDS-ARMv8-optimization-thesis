#ifndef VEC_16X8_H
#define VEC_16X8_H

#include <arm_neon.h>

#include "params.h"

// UNSIGNED INTEGER INSTRUCTIONS & DATA TYPES
#define COMBINE_VEC(a, b) vcombine_u16(a, b)
#define ADD_LOW_VEC(a, b) vaddl_u16(vget_low_u16(a), vget_low_u16(b))
#define ADD_HIGH_VEC(a, b) vaddl_high_u16(a, b)
// #define SUB_LOW_VEC(a, b) vsub_u16(a, b)
// #define SUB_VEC(a, b) vsubl_u16(a, b)
// #define MUL_VEC(a, b) vmull_u16(a, b)
// #define MUL_ACC_VEC(a, b, c) vmlal_u16(a, b, c)
#define MUL_ACC_VEC_LOW(v, a, b) vmlal_u16(v, vget_low_u16(a), vget_low_u16(b))
#define MUL_ACC_VEC_HIGH(v, a, b) vmlal_high_u16(v, a, b)

#define OR_VEC(a, b) vorrq_u16(a, b)
#define AND_VEC(a, b) vandq_u16(a, b)
#define XOR_VEC(a, b) veorq_u16(a, b)

#define EQ_VEC(a, b) vceqq_u16(a, b)
#define EQ0_VEC(a) vceqzq_u16(a)
#define EQ_VEC(a, b) vceqq_u16(a, b)

#define SET_VEC(a) vdupq_n_u16(a)
#define TO_U_VEC(a) vreinterpretq_u16_s16(a)

// #define MEDS_p_VEC vdup_n_u16(MEDS_p)
#define ZERO_VEC vdupq_n_u16(0)
#define ZERO_VEC_W vdupq_n_u32(0)

#define SET_LANE_VEC(a, i, val) vsetq_lane_u16(val, a, i)

#define GFq_vec_t uint16x8_t
#define GFq_vec_w_t uint32x4_t
#define pmod_mat_vec_t GFq_vec_t
#define pmod_mat_vec_w_t GFq_vec_w_t

// SIGNED INSTRUCTIONS & DATA TYPES
#define ADD_S_VEC(a, b) vaddq_s16(a, b)
#define AND_S_VEC(a, b) vandq_s16(a, b)
#define XOR_S_VEC(a, b) veorq_s16(a, b)
#define NOT_S_VEC(a) vmvnq_s16(a)

#define EQ0_S_VEC(a) vceqzq_s16(a)
#define EQ_S_VEC(a, b) vceqq_s16(a, b)
#define GEQ_S_VEC(a, b) vcgeq_s16(a, b)

#define SET_S_VEC(a) vdupq_n_s16(a)
#define TO_S_VEC(a) vreinterpretq_s16_u16(a)

#define ZERO_S_VEC vdupq_n_s16(0)

#define GET_LANE_S_VEC(a, i) vgetq_lane_s16(a, i)

#define pmod_mat_s_vec_t int16x8_t

// DEFINITIONS
#define BATCH_SIZE 8
#define VEC_TRUE 0xffff
#define VEC_FALSE 0x0000

#if MEDS_p == 4093
#define MAGIC_VEC vdupq_n_u32(0x80180481)

#define MEDS_p_VEC_16x4 vdup_n_u16(MEDS_p)
#define MEDS_p_VEC_32x4 vdupq_n_u32(MEDS_p)

#define FREEZE_VEC(_v2)                   \
  ({                                      \
    uint16x4_t _t2;                       \
    _t2 = vcge_u16(_v2, MEDS_p_VEC_16x4); \
    _t2 = vand_u16(_t2, MEDS_p_VEC_16x4); \
    vsub_u16(_v2, _t2);                   \
  })

#define REDUCE_VEC(_v4)                                                     \
  ({                                                                        \
    uint32x4_t _t4_1 = _v4;                                                 \
    uint32x4_t _t4_2 = MAGIC_VEC;                                           \
    uint64x2_t _t4_3 = vmull_u32(vget_low_u32(_t4_1), vget_low_u32(_t4_2)); \
    uint64x2_t _t4_4 = vmull_high_u32(_t4_1, _t4_2);                        \
    uint32x4_t _t4_5 = vuzp2q_u32((uint32x4_t)_t4_3, (uint32x4_t)_t4_4);    \
    uint32x4_t _t4_6 = vshrq_n_u32(_t4_5, 11);                              \
    uint32x4_t _t4_7 = vmlsq_u32(_t4_1, _t4_6, MEDS_p_VEC_32x4);            \
    vmovn_u32(_t4_7);                                                       \
  })

#define FREEZE_REDUCE_VEC(_v5) REDUCE_VEC(_v5)

#else
// Use a simple reduction technique in order for the 'toy' parameter set to function.
#define REDUCE_VEC(_v4)       \
  ({                          \
    uint16x4_t _t4;           \
    _t4[0] = _v4[0] % MEDS_p; \
    _t4[1] = _v4[1] % MEDS_p; \
    _t4[2] = _v4[2] % MEDS_p; \
    _t4[3] = _v4[3] % MEDS_p; \
    _t4;                      \
  })
#define FREEZE_VEC(_v5) REDUCE_VEC(_v5)
#define FREEZE_REDUCE_VEC(_v5) REDUCE_VEC(_v5)
#endif

pmod_mat_vec_t load_vec(GFq_t *M[], int M_r, int M_c, int r, int c);
void store_vec(GFq_t *M[], int M_r, int M_c, int r, int c, pmod_mat_vec_t val, int amount);

void store_bitstream_12bit(uint8_t **bs_buf, int bs_index, pmod_mat_vec_t *G_vec, int batch_size);

#endif