#include "matrixmod_vec.h"

void pmod_mat_mul_vec(pmod_mat_vec_t *C, int C_r, int C_c, pmod_mat_vec_t *A, int A_r, int A_c, pmod_mat_vec_t *B, int B_r, int B_c)
{
  PROFILER_START("pmod_mat_mul_vec");
  pmod_mat_vec_t tmp[C_r * C_c];

  for (int c = 0; c < C_c; c++)
    for (int r = 0; r < C_r; r++)
    {
      pmod_mat_vec_w_t val = ZERO_VEC_W;

      for (int i = 0; i < A_c; i++)
        val = MUL_ACC_VEC(val, pmod_mat_entry(A, A_r, A_c, r, i), pmod_mat_entry(B, B_r, B_c, i, c));

      tmp[r * C_c + c] = freeze_vec(reduce_vec(val));
    }

  for (int c = 0; c < C_c; c++)
    for (int r = 0; r < C_r; r++)
      pmod_mat_set_entry(C, C_r, C_c, r, c, tmp[r * C_c + c]);
  PROFILER_STOP("pmod_mat_mul_vec");
}

pmod_mat_s_vec_t pmod_mat_syst_ct_vec(pmod_mat_vec_t *M, int M_r, int M_c)
{
  PROFILER_START("pmod_mat_syst_ct_vec");
  pmod_mat_s_vec_t result = pmod_mat_syst_ct_partial_swap_backsub_vec(M, M_r, M_c, M_r, 0, 1);
  PROFILER_STOP("pmod_mat_syst_ct_vec");
  return result;
}

pmod_mat_s_vec_t pmod_mat_syst_ct_partial_vec(pmod_mat_vec_t *M, int M_r, int M_c, int max_r)
{
  PROFILER_START("pmod_mat_syst_ct_partial_vec");
  pmod_mat_s_vec_t result = pmod_mat_syst_ct_partial_swap_backsub_vec(M, M_r, M_c, max_r, 0, 1);
  PROFILER_STOP("pmod_mat_syst_ct_partial_vec");
  return result;
}

pmod_mat_s_vec_t pmod_mat_rref_vec(pmod_mat_vec_t *M, int M_r, int M_c)
{
  PROFILER_START("pmod_mat_rref_vec");
  pmod_mat_s_vec_t result = pmod_mat_syst_ct_partial_swap_backsub_vec(M, M_r, M_c, M_r, 1, 1);
  PROFILER_STOP("pmod_mat_rref_vec");
  return result;
}

// Possible calls:
// pmod_mat_syst_ct_partial_swap_backsub(m-1, m, m, 1, 1)     (solve_opt)
// pmod_mat_syst_ct_partial_swap_backsub(n, n, n, 0, 1)       (pmod_mat_inv)
// pmod_mat_syst_ct_partial_swap_backsub(m, m, m, 0, 1)       (pmod_mat_inv)
// pmod_mat_syst_ct_partial_swap_backsub(k, k, k, 0, 1)       (pmod_mat_inv)
// pmod_mat_syst_ct_partial_swap_backsub(n, 2m, n-1, 0, 1)    (solve_opt)
// pmod_mat_syst_ct_partial_swap_backsub(m, m, m, 0, 0)       (rnd_inv_matrix)
// pmod_mat_syst_ct_partial_swap_backsub(k, k, k, 0, 0)       (rnd_inv_matrix)
pmod_mat_s_vec_t pmod_mat_syst_ct_partial_swap_backsub_vec(pmod_mat_vec_t *M, int M_r, int M_c, int max_r, int swap, int backsub)
{
  PROFILER_START("pmod_mat_syst_ct_partial_swap_backsub_vec");
  pmod_mat_s_vec_t ret = SET_S_VEC(M_r * swap);
  pmod_mat_s_vec_t minus_one = SET_S_VEC(-1);

  for (int r = 0; r < max_r; r++)
  {
    if (swap)
    {
      pmod_mat_vec_t z = ZERO_VEC;

      // compute condition for swap
      for (int r2 = r; r2 < M_r; r2++)
        z = OR_VEC(z, pmod_mat_entry(M, M_r, M_c, r2, r));

      // int do_swap = GFq_eq0(z);
      pmod_mat_s_vec_t do_swap = TO_S_VEC(EQ0_VEC(z));

      // conditional swap
      {
        ret = ADD_S_VEC(AND_S_VEC(SET_S_VEC(r), do_swap), AND_S_VEC(ret, NOT_S_VEC(do_swap)));

        for (int i = 0; i < M_r; i++)
        {
          pmod_mat_vec_cswap_s_cond(&pmod_mat_entry(M, M_r, M_c, i, r),
                             &pmod_mat_entry(M, M_r, M_c, i, M_c - 1),
                             do_swap);
        }
      }
    }

    for (int r2 = r + 1; r2 < M_r; r2++)
    {
      pmod_mat_vec_t Mrr = pmod_mat_entry(M, M_r, M_c, r, r);

      for (int c = r; c < M_c; c++)
      {
        pmod_mat_vec_t val = pmod_mat_entry(M, M_r, M_c, r2, c);
        pmod_mat_vec_t Mrc = pmod_mat_entry(M, M_r, M_c, r, c);

        uint16x4_t abc = freeze_vec(reduce_vec(ADD_VEC(Mrc, AND_VEC(val, EQ0_VEC(Mrr)))));
        pmod_mat_set_entry(M, M_r, M_c, r, c, abc);
      }
    }

    pmod_mat_vec_t val = pmod_mat_entry(M, M_r, M_c, r, r);

    // For each vector element i, if val[i] == 0, then set ret[i] = -1. Otherwise, do nothing.
    pmod_mat_s_vec_cmov(&ret, &minus_one, TO_S_VEC(EQ0_VEC(val)));

    val = GF_inv_vec(val);

    // normalize
    for (int c = r; c < M_c; c++)
    {
      pmod_mat_vec_t tmp = freeze_vec(reduce_vec(MUL_VEC(val, pmod_mat_entry(M, M_r, M_c, r, c))));
      pmod_mat_set_entry(M, M_r, M_c, r, c, tmp);
    }

    // eliminate
    for (int r2 = r + 1; r2 < M_r; r2++)
    {
      pmod_mat_vec_t factor = pmod_mat_entry(M, M_r, M_c, r2, r);

      for (int c = r; c < M_c; c++)
      {
        pmod_mat_vec_t tmp0 = pmod_mat_entry(M, M_r, M_c, r, c);
        pmod_mat_vec_t tmp1 = pmod_mat_entry(M, M_r, M_c, r2, c);

        pmod_mat_vec_t val = freeze_vec(reduce_vec(MUL_VEC(tmp0, factor)));
        val = freeze_vec(SUB_LOW_VEC(ADD_LOW_VEC(MEDS_p_VEC, tmp1), val));
        pmod_mat_set_entry(M, M_r, M_c, r2, c, val);
      }
    }
  }

  if (!backsub)
  {
    PROFILER_STOP("pmod_mat_syst_ct_partial_swap_backsub");
    return ret;
  }

  // back substitution
  for (int r = max_r - 1; r >= 0; r--)
    for (int r2 = 0; r2 < r; r2++)
    {
      pmod_mat_vec_t factor = pmod_mat_entry(M, M_r, M_c, r2, r);

      pmod_mat_vec_t tmp0 = pmod_mat_entry(M, M_r, M_c, r, r);
      pmod_mat_vec_t tmp1 = pmod_mat_entry(M, M_r, M_c, r2, r);

      pmod_mat_vec_t val = freeze_vec(reduce_vec(MUL_VEC(tmp0, factor)));
      val = freeze_vec(SUB_LOW_VEC(ADD_LOW_VEC(MEDS_p_VEC, tmp1), val));

      pmod_mat_set_entry(M, M_r, M_c, r2, r, val);

      for (int c = max_r; c < M_c; c++)
      {
        pmod_mat_vec_t tmp0 = pmod_mat_entry(M, M_r, M_c, r, c);
        pmod_mat_vec_t tmp1 = pmod_mat_entry(M, M_r, M_c, r2, c);

        pmod_mat_vec_t val = freeze_vec(reduce_vec(MUL_VEC(tmp0, factor)));
        val = freeze_vec(SUB_LOW_VEC(ADD_LOW_VEC(MEDS_p_VEC, tmp1), val));

        pmod_mat_set_entry(M, M_r, M_c, r2, c, val);
      }
    }

  PROFILER_STOP("pmod_mat_syst_ct_partial_swap_backsub_vec");
  return ret;
}

GFq_vec_t GF_inv_vec(GFq_vec_t val)
{
  // Use an optimal addition chain for MEDS_p = 4093
  GFq_vec_t tmp_0 = val;                                  // 1
  GFq_vec_t tmp_1 = reduce_vec(MUL_VEC(tmp_0, tmp_0));    // 2
  GFq_vec_t tmp_2 = reduce_vec(MUL_VEC(tmp_1, tmp_1));    // 4
  GFq_vec_t tmp_3 = reduce_vec(MUL_VEC(tmp_2, tmp_0));    // 5
  GFq_vec_t tmp_4 = reduce_vec(MUL_VEC(tmp_3, tmp_3));    // 10
  GFq_vec_t tmp_5 = reduce_vec(MUL_VEC(tmp_4, tmp_3));    // 15
  GFq_vec_t tmp_6 = reduce_vec(MUL_VEC(tmp_5, tmp_5));    // 30
  GFq_vec_t tmp_7 = reduce_vec(MUL_VEC(tmp_6, tmp_6));    // 60
  GFq_vec_t tmp_8 = reduce_vec(MUL_VEC(tmp_7, tmp_7));    // 120
  GFq_vec_t tmp_9 = reduce_vec(MUL_VEC(tmp_8, tmp_8));    // 240
  GFq_vec_t tmp_10 = reduce_vec(MUL_VEC(tmp_9, tmp_5));   // 255
  GFq_vec_t tmp_11 = reduce_vec(MUL_VEC(tmp_10, tmp_10)); // 510
  GFq_vec_t tmp_12 = reduce_vec(MUL_VEC(tmp_11, tmp_11)); // 1020
  GFq_vec_t tmp_13 = reduce_vec(MUL_VEC(tmp_12, tmp_12)); // 2040
  GFq_vec_t tmp_14 = reduce_vec(MUL_VEC(tmp_13, tmp_3));  // 2045
  GFq_vec_t tmp_15 = reduce_vec(MUL_VEC(tmp_14, tmp_14)); // 4090
  GFq_vec_t tmp_16 = reduce_vec(MUL_VEC(tmp_15, tmp_0));  // 4091
  return freeze_vec(tmp_16);
}