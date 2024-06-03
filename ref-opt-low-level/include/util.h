#ifndef UTILS_H
#define UTILS_H

#include "params.h"
#include "matrixmod.h"
#include "fips202.h"

#include <arm_neon.h>

#define MAGIC_VEC vdupq_n_u32(0x80180481)

#define MEDS_p_VEC_16x4 vdup_n_u16(MEDS_p)
#define MEDS_p_VEC_16x8 vdupq_n_u16(MEDS_p)
#define MEDS_p_VEC_32x4 vdupq_n_u32(MEDS_p)

#define FREEZE_VEC_16x4(_v2)              \
  ({                                      \
    uint16x4_t _t2;                       \
    _t2 = vcge_u16(_v2, MEDS_p_VEC_16x4); \
    _t2 = vand_u16(_t2, MEDS_p_VEC_16x4); \
    vsub_u16(_v2, _t2);                   \
  })

#define FREEZE_VEC_16x8(_v2)               \
  ({                                       \
    uint16x8_t _t2;                        \
    _t2 = vcgeq_u16(_v2, MEDS_p_VEC_16x8); \
    _t2 = vandq_u16(_t2, MEDS_p_VEC_16x8); \
    vsubq_u16(_v2, _t2);                   \
  })

#define FREEZE_REDUCE_VEC_16x4(_v4)                                         \
  ({                                                                        \
    uint32x4_t _t4_1 = _v4;                                                 \
    uint32x4_t _t4_2 = MAGIC_VEC;                                           \
    uint64x2_t _t4_3 = vmull_u32(vget_low_u32(_t4_1), vget_low_u32(_t4_2)); \
    uint64x2_t _t4_4 = vmull_high_u32(_t4_1, _t4_2);                        \
    uint32x4_t _t4_5 = vuzp2q_u32((uint32x4_t)_t4_3, (uint32x4_t)_t4_4);    \
    uint32x4_t _t4_6 = vshrq_n_u32(_t4_5, 11);                              \
    vmlsq_u32(_t4_1, _t4_6, MEDS_p_VEC_32x4);                               \
  })

#define FREEZE_REDUCE_VEC_16x4_SHRINK(_v5)          \
  ({                                                \
    uint32x4_t _t5_1 = FREEZE_REDUCE_VEC_16x4(_v5); \
    vmovn_u32(_t5_1);                               \
  })

void XOF(uint8_t **buf, size_t *length, const uint8_t *seed, size_t seed_len, int num);

GFq_t rnd_GF(keccak_state *shake);

void rnd_sys_mat(pmod_mat_t *M, int M_r, int M_c, const uint8_t *seed, size_t seed_len);

void rnd_inv_matrix(pmod_mat_t *M, int M_r, int M_c, uint8_t *seed, size_t seed_len, int (*pmod_mat_syst_fun)(pmod_mat_t *));

int parse_hash(uint8_t *digest, int digest_len, uint8_t *h, int len_h);

int solve(pmod_mat_t *A, pmod_mat_t *B_inv, pmod_mat_t *G0prime, int ct); //, bool partial);

void pi(pmod_mat_t *Gout, pmod_mat_t *A, pmod_mat_t *B, pmod_mat_t *G);

int SF(pmod_mat_t *Gprime, pmod_mat_t *G);

#endif
