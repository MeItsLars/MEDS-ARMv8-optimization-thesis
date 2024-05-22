#ifndef UTILS_H
#define UTILS_H

#include "params.h"
#include "matrixmod.h"
#include "fips202.h"

#include <arm_neon.h>

#define MEDS_p_VEC vdup_n_u16(MEDS_p)

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

void XOF(uint8_t **buf, size_t *length, const uint8_t *seed, size_t seed_len, int num);

GFq_t rnd_GF(keccak_state *shake);

void rnd_sys_mat(pmod_mat_t *M, int M_r, int M_c, const uint8_t *seed, size_t seed_len);

void rnd_inv_matrix(pmod_mat_t *M, int M_r, int M_c, uint8_t *seed, size_t seed_len, int (*pmod_mat_syst_fun)(pmod_mat_t *));

int parse_hash(uint8_t *digest, int digest_len, uint8_t *h, int len_h);

int solve(pmod_mat_t *A, pmod_mat_t *B_inv, pmod_mat_t *G0prime, int ct); //, bool partial);

void pi(pmod_mat_t *Gout, pmod_mat_t *A, pmod_mat_t *B, pmod_mat_t *G);

int SF(pmod_mat_t *Gprime, pmod_mat_t *G);

#endif
