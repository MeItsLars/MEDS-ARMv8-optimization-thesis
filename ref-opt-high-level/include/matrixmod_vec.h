#ifndef MATRIXMOD_VEC_H
#define MATRIXMOD_VEC_H

#include "vec_16x4.h"
#include "matrixmod.h"
#include "profiler.h"

static inline void pmod_mat_vec_cswap(pmod_mat_vec_t *a, pmod_mat_vec_t *b, pmod_mat_s_vec_t cond)
{
  pmod_mat_vec_t diff = AND_VEC(XOR_VEC(*a, *b), TO_U_VEC(cond));

  *a = XOR_VEC(*a, diff);
  *b = XOR_VEC(*b, diff);
}

static inline void pmod_mat_vec_cmov(pmod_mat_vec_t *a, pmod_mat_vec_t *b, pmod_mat_vec_t cond)
{
  pmod_mat_vec_t diff = AND_VEC(XOR_VEC(*a, *b), cond);

  *a = XOR_VEC(*a, diff);
}

void pmod_mat_mul_vec(pmod_mat_vec_t *C, int C_r, int C_c, pmod_mat_vec_t *A, int A_r, int A_c, pmod_mat_vec_t *B, int B_r, int B_c);

pmod_mat_s_vec_t pmod_mat_rref_vec(pmod_mat_vec_t *M, int M_r, int M_c);
pmod_mat_s_vec_t pmod_mat_syst_ct_vec(pmod_mat_vec_t *M, int M_r, int M_c);
pmod_mat_s_vec_t pmod_mat_syst_ct_partial_vec(pmod_mat_vec_t *M, int M_r, int M_c, int max_r);
pmod_mat_s_vec_t pmod_mat_syst_ct_partial_swap_backsub_vec(pmod_mat_vec_t *M, int M_r, int M_c, int max_r, int swap, int backsub);

#endif