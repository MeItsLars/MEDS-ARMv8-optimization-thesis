#ifndef MATRIXMOD_VEC_H
#define MATRIXMOD_VEC_H

#include <string.h>

#include "vec_16x4.h"
#include "matrixmod.h"
#include "profiler.h"

// Precomputed inverse table for MEDS_p = 4093
extern uint16_t mod_inverse_table[4093];

static inline void pmod_mat_vec_cswap(pmod_mat_vec_t *a, pmod_mat_vec_t *b, pmod_mat_vec_t cond)
{
  pmod_mat_vec_t diff = AND_VEC(XOR_VEC(*a, *b), cond);

  *a = XOR_VEC(*a, diff);
  *b = XOR_VEC(*b, diff);
}

static inline void pmod_mat_vec_cswap_s_cond(pmod_mat_vec_t *a, pmod_mat_vec_t *b, pmod_mat_s_vec_t cond)
{
  pmod_mat_vec_cswap(a, b, TO_U_VEC(cond));
}

static inline void pmod_mat_s_vec_cmov(pmod_mat_s_vec_t *a, pmod_mat_s_vec_t *b, pmod_mat_s_vec_t cond)
{
  pmod_mat_s_vec_t diff = AND_S_VEC(XOR_S_VEC(*a, *b), cond);

  *a = XOR_S_VEC(*a, diff);
}

void pmod_mat_mul_vec(pmod_mat_vec_t *C, int C_r, int C_c, pmod_mat_vec_t *A, int A_r, int A_c, pmod_mat_vec_t *B, int B_r, int B_c);

pmod_mat_s_vec_t pmod_mat_rref_vec(pmod_mat_vec_t *M, int M_r, int M_c, int ct);
pmod_mat_s_vec_t pmod_mat_syst_ct_vec(pmod_mat_vec_t *M, int M_r, int M_c, int ct);
pmod_mat_s_vec_t pmod_mat_syst_ct_partial_vec(pmod_mat_vec_t *M, int M_r, int M_c, int max_r, int ct);
pmod_mat_s_vec_t pmod_mat_syst_ct_partial_swap_backsub_vec(pmod_mat_vec_t *M, int M_r, int M_c, int max_r, int swap, int backsub, int ct);

GFq_vec_t GF_inv_vec(GFq_vec_t val, int ct);

pmod_mat_s_vec_t pmod_mat_inv_vec(pmod_mat_vec_t *B, pmod_mat_vec_t *A, int A_r, int A_c, int ct);

#endif