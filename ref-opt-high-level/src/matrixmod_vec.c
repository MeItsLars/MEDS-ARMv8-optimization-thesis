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

pmod_mat_s_vec_t pmod_mat_syst_ct_partial_swap_backsub_vec(pmod_mat_vec_t *M, int M_r, int M_c, int max_r, int swap, int backsub)
{
  PROFILER_START("pmod_mat_syst_ct_partial_swap_backsub_vec");
  pmod_mat_s_vec_t ret = SET_S_VEC(M_r * swap);

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
          pmod_mat_vec_cswap(&pmod_mat_entry(M, M_r, M_c, i, r),
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

        pmod_mat_set_entry(M, M_r, M_c, r, c, freeze_vec(reduce_vec(ADD_VEC(Mrc, AND_VEC(val, EQ0_VEC(Mrr))))));
      }
    }

    pmod_mat_vec_t VAL = pmod_mat_entry(M, M_r, M_c, r, r);

    // For each vector element i, if val[i] == 0, then set ret[i] = -1. Otherwise, do nothing.
    ret = pmod_mat_vec_cmov(&ret, SET_S_VEC(-1), EQ0_VEC(VAL));

    /*
    val = GF_inv(val);

    // normalize
    for (int c = r; c < M_c; c++)
    {
      uint64_t tmp = ((uint64_t)pmod_mat_entry(M, M_r, M_c, r, c) * val) % MEDS_p;
      pmod_mat_set_entry(M, M_r, M_c, r, c, tmp);
    }

    // eliminate
    for (int r2 = r + 1; r2 < M_r; r2++)
    {
      uint64_t factor = pmod_mat_entry(M, M_r, M_c, r2, r);

      for (int c = r; c < M_c; c++)
      {
        uint64_t tmp0 = pmod_mat_entry(M, M_r, M_c, r, c);
        uint64_t tmp1 = pmod_mat_entry(M, M_r, M_c, r2, c);

        uint64_t val = (tmp0 * factor) % MEDS_p;

        val = (MEDS_p + tmp1 - val) % MEDS_p;

        pmod_mat_set_entry(M, M_r, M_c, r2, c, val);
      }
    }*/
  }

  if (!backsub)
  {
    PROFILER_STOP("pmod_mat_syst_ct_partial_swap_backsub");
    return ret;
  }
  // TODO: remove this lol
  return ret;

  /*
  // back substitution
  for (int r = max_r - 1; r >= 0; r--)
    for (int r2 = 0; r2 < r; r2++)
    {
      uint64_t factor = pmod_mat_entry(M, M_r, M_c, r2, r);

      uint64_t tmp0 = pmod_mat_entry(M, M_r, M_c, r, r);
      uint64_t tmp1 = pmod_mat_entry(M, M_r, M_c, r2, r);

      uint64_t val = (tmp0 * factor) % MEDS_p;

      val = (MEDS_p + tmp1 - val) % MEDS_p;

      pmod_mat_set_entry(M, M_r, M_c, r2, r, val);

      for (int c = max_r; c < M_c; c++)
      {
        uint64_t tmp0 = pmod_mat_entry(M, M_r, M_c, r, c);
        uint64_t tmp1 = pmod_mat_entry(M, M_r, M_c, r2, c);

        uint64_t val = (tmp0 * factor) % MEDS_p;

        val = (MEDS_p + tmp1 - val) % MEDS_p;

        pmod_mat_set_entry(M, M_r, M_c, r2, c, val);
      }
    }

  PROFILER_STOP("pmod_mat_syst_ct_partial_swap_backsub_vec");
  return ret;*/
}