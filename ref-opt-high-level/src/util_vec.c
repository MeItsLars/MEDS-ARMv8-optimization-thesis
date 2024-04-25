#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "log.h"
#include "profiler.h"
#include "util_vec.h"
#include "meds.h"
#include "matrixmod.h"

/*
solve_old(...) {
  // set up core sub-system
  pmod_mat_t N[MEDS_n * (2 * MEDS_m)] = {0};

  for (int i = 0; i < MEDS_m; i++)
    for (int j = 0; j < MEDS_n; j++)
    {
      pmod_mat_set_entry(N, MEDS_n, 2 * MEDS_m, j, i,
                         (MEDS_p - pmod_mat_entry((G0prime + (MEDS_m * MEDS_n)), MEDS_m, MEDS_n, i, j)) % MEDS_p);

      pmod_mat_set_entry(N, MEDS_n, 2 * MEDS_m, j, MEDS_m + i,
                         pmod_mat_entry(G0prime, MEDS_m, MEDS_n, i, j));
    }

  LOG_MAT(N, MEDS_n, 2 * MEDS_m);

  // Systemize core sub-system while pivoting all but the last row.
  int piv;
  if ((piv = pmod_mat_syst_ct_partial(N, MEDS_n, 2 * MEDS_m, MEDS_n - 1)) != 0)
  {
    LOG("no sol %i", __LINE__);
    PROFILER_STOP("solve_vec");
    return -1;
  }

  LOG_MAT(N, MEDS_n, 2 * MEDS_m);

  // Extract 2nd sub-system.
  pmod_mat_t N1[(MEDS_m - 1) * MEDS_m] = {0};

  for (int i = 0; i < MEDS_m; i++)
  {
    pmod_mat_set_entry(N1, MEDS_n, MEDS_m, 0, i,
                       pmod_mat_entry(N, MEDS_n, 2 * MEDS_m, MEDS_n - 1, MEDS_m + i));
  }

  LOG_MAT(N1, MEDS_m - 1, MEDS_m);

  // Remove front diagonale of core sub-system.
  for (int i = 0; i < MEDS_n; i++)
    for (int j = 0; j < MEDS_m; j++)
      pmod_mat_set_entry(N, MEDS_n - 1, MEDS_m, i, j,
                         pmod_mat_entry(N, MEDS_n - 1, 2 * MEDS_m, i, MEDS_m + j));

  // Consider N MEDS_n-1 x MEDS_m, now...
  LOG_MAT(N, MEDS_n - 1, MEDS_m);

  // Fully reduce 2nd sub-system.
  for (int row = 1; row < MEDS_m - 1; row++)
    for (int i = 0; i < MEDS_m; i++)
      for (int j = 0; j < MEDS_m; j++)
      {
        uint64_t tmp0 = pmod_mat_entry(N1, MEDS_m - 1, MEDS_m, row - 1, i);
        uint64_t tmp1 = pmod_mat_entry(N, MEDS_n - 1, MEDS_m, i, j);

        uint64_t prod = (tmp0 * tmp1) % MEDS_p;

        uint64_t tmp2 = pmod_mat_entry(N1, MEDS_m - 1, MEDS_m, row, j);

        int64_t diff = (MEDS_p + tmp2 - prod) % MEDS_p;

        pmod_mat_set_entry(N1, MEDS_m - 1, MEDS_m, row, j, diff);
      }

  LOG_MAT(N1, MEDS_m - 1, MEDS_m);

  GFq_t sol[MEDS_m * MEDS_m + MEDS_n * MEDS_n] = {0};

  int N1_r;

  // Sytemize 2nd sub-system.
  N1_r = pmod_mat_rref(N1, MEDS_m - 1, MEDS_m);

  if (N1_r == -1)
  {
    LOG("RREF failed. Redo.\n");
    PROFILER_STOP("solve_vec");
    return -1;
  }

  LOG_MAT(N1, MEDS_m - 1, MEDS_m);

  LOG("pivot column: %i\n", N1_r);

  // Fill in solutions from the 2nd sub-system.
  for (int i = 0; i < MEDS_m - 1; i++)
    sol[2 * MEDS_m * MEDS_n - (MEDS_m - 1) + i] = pmod_mat_entry(N1, MEDS_m - 1, MEDS_m, i, MEDS_m - 1);

  // Fix to -1.
  sol[MEDS_m * MEDS_m + MEDS_n * MEDS_n - 1] = MEDS_p - 1;

  LOG_VEC(sol, MEDS_m * MEDS_m + MEDS_n * MEDS_n);

  // Fill in solutions from core sub-system.
  pmod_mat_t N_swap[(MEDS_n - 1) * MEDS_m] = {0};

  memcpy(N_swap, N, sizeof(N_swap));

  LOG_MAT(N_swap, MEDS_n - 1, MEDS_m);

  for (int r = 0; r < MEDS_m; r++)
    for (int c = 0; c < MEDS_n - 1; c++)
      GFq_cswap(&pmod_mat_entry(N_swap, MEDS_n - 1, MEDS_m, r, c),
                &pmod_mat_entry(N_swap, MEDS_n - 1, MEDS_m, r, MEDS_m - 1),
                eq(c, N1_r));

  LOG_MAT(N_swap, MEDS_n - 1, MEDS_m);

  for (int r = 0; r < MEDS_m; r++)
    sol[2 * MEDS_m * MEDS_n - MEDS_m - (MEDS_m - 1) + r] = pmod_mat_entry(N_swap, MEDS_n - 1, MEDS_m, r, MEDS_m - 1);

  LOG_VEC(sol, MEDS_m * MEDS_m + MEDS_n * MEDS_n);

  // Back-substitue second last block.
  for (int c = MEDS_m - 2; c >= 0; c--)
    for (int r = 0; r < MEDS_m; r++)
    {
      uint64_t tmp1 = pmod_mat_entry(N_swap, MEDS_n - 1, MEDS_m, r, c);
      uint64_t tmp2 = sol[2 * MEDS_m * MEDS_n - (MEDS_m - 1) + c];

      uint64_t prod = (tmp1 * tmp2) % MEDS_p;

      uint64_t val = sol[2 * MEDS_m * MEDS_n - MEDS_m - (MEDS_m - 1) + r];

      val = ((MEDS_p + val) - prod) % MEDS_p;

      sol[2 * MEDS_m * MEDS_n - MEDS_m - (MEDS_m - 1) + r] = val;
    }

  LOG_VEC(sol, MEDS_m * MEDS_m + MEDS_n * MEDS_n);

  // Prepare substitution-source of first block in bottom part.
  pmod_mat_t P01nt[MEDS_n * MEDS_m] = {0};

  for (int i = 0; i < MEDS_n; i++)
    for (int j = 0; j < MEDS_m; j++)
      pmod_mat_set_entry(P01nt, MEDS_n, MEDS_m, i, j,
                         (MEDS_p - pmod_mat_entry((G0prime + (MEDS_m * MEDS_n)), MEDS_m, MEDS_n, j, i)) % MEDS_p);

  LOG_MAT(P01nt, MEDS_n, MEDS_m);

  // Conditionally swap columns.
  for (int r = 0; r < MEDS_n; r++)
    for (int c = 0; c < MEDS_m; c++)
      GFq_cswap(&pmod_mat_entry(P01nt, MEDS_n, MEDS_m, r, c),
                &pmod_mat_entry(P01nt, MEDS_n, MEDS_m, r, MEDS_m - 1),
                eq(c, N1_r));

  LOG_MAT(P01nt, MEDS_n, MEDS_m);

  // Transfer last column to solution vector.
  for (int r = 0; r < MEDS_n; r++)
    sol[MEDS_m * MEDS_n + r] = pmod_mat_entry(P01nt, MEDS_n, MEDS_m, r, MEDS_m - 1);

  LOG_VEC(sol, MEDS_m * MEDS_m + MEDS_n * MEDS_n);

  // Perfrom back-substution for first block in bottom part.
  for (int c = MEDS_m - 2; c >= 0; c--)
    for (int r = 0; r < MEDS_n; r++)
    {
      uint64_t tmp1 = pmod_mat_entry(P01nt, MEDS_n, MEDS_m, r, c);
      uint64_t tmp2 = pmod_mat_entry(N1, MEDS_m - 1, MEDS_m, c, MEDS_m - 1);

      uint64_t prod = (tmp1 * tmp2) % MEDS_p;

      uint64_t val = sol[MEDS_m * MEDS_n + r];

      val = ((MEDS_p + val) - prod) % MEDS_p;

      sol[MEDS_m * MEDS_n + r] = val;
    }

  LOG_VEC(sol, MEDS_m * MEDS_m + MEDS_n * MEDS_n);

  // Prepare substitution-source for top part.
  pmod_mat_t P00nt[MEDS_n * MEDS_m] = {0};

  for (int i = 0; i < MEDS_n; i++)
    for (int j = 0; j < MEDS_m; j++)
      pmod_mat_set_entry(P00nt, MEDS_n, MEDS_m, i, j,
                         (MEDS_p - pmod_mat_entry(G0prime, MEDS_m, MEDS_n, j, i)) % MEDS_p);

  LOG_MAT(P00nt, MEDS_n, MEDS_m);

  pmod_mat_t P00nt_swap[MEDS_n * MEDS_m] = {0};

  memcpy(P00nt_swap, P00nt, sizeof(P00nt_swap));

  LOG_MAT(P00nt_swap, MEDS_n, MEDS_m);

  for (int r = 0; r < MEDS_n; r++)
    for (int c = 0; c < MEDS_m; c++)
      GFq_cswap(&pmod_mat_entry(P00nt_swap, MEDS_n, MEDS_m, r, c),
                &pmod_mat_entry(P00nt_swap, MEDS_n, MEDS_m, r, MEDS_m - 1),
                eq(c, N1_r));

  LOG_MAT(P00nt_swap, MEDS_n, MEDS_m);

  // For the last block in the top part, transfer last column to solution vector.
  for (int r = 0; r < MEDS_n; r++)
    sol[(MEDS_m - 1) * MEDS_n + r] = pmod_mat_entry(P00nt_swap, MEDS_n, MEDS_m, r, MEDS_m - 1);

  LOG_VEC(sol, MEDS_m * MEDS_m + MEDS_n * MEDS_n);

  // Perfrom back-substution for last block in top part.
  for (int c = MEDS_m - 2; c >= 0; c--)
    for (int r = 0; r < MEDS_n; r++)
    {
      uint64_t tmp1 = pmod_mat_entry(P00nt_swap, MEDS_n, MEDS_m, r, c);
      uint64_t tmp2 = pmod_mat_entry(N1, MEDS_m - 1, MEDS_m, c, MEDS_m - 1);

      uint64_t prod = (tmp1 * tmp2) % MEDS_p;

      uint64_t val = sol[(MEDS_m - 1) * MEDS_n + r];

      val = ((MEDS_p + val) - prod) % MEDS_p;

      sol[(MEDS_m - 1) * MEDS_n + r] = val;
    }

  LOG_VEC(sol, MEDS_m * MEDS_m + MEDS_n * MEDS_n);

  // Perfrom back-substution for all remaining blocks in bottom part.
  for (int b = MEDS_m - 3; b >= 0; b--)
    for (int c = MEDS_m - 1; c >= 0; c--)
      for (int r = 0; r < MEDS_m; r++)
      {
        uint64_t tmp1 = pmod_mat_entry(N, MEDS_n - 1, MEDS_m, r, c);
        uint64_t tmp2 = sol[(MEDS_m + 1) * MEDS_n + b * MEDS_m + MEDS_m + c];

        uint64_t prod = (tmp1 * tmp2) % MEDS_p;

        uint64_t val = sol[(MEDS_m + 1) * MEDS_n + b * MEDS_m + r];

        val = ((MEDS_p + val) - prod) % MEDS_p;

        sol[(MEDS_m + 1) * MEDS_n + b * MEDS_m + r] = val;
      }

  LOG_VEC(sol, MEDS_m * MEDS_m + MEDS_n * MEDS_n);

  // Perfrom back-substution for all remaining blocks in top part.
  for (int b = MEDS_m - 2; b >= 0; b--)
    for (int c = MEDS_m - 1; c >= 0; c--)
      for (int r = 0; r < MEDS_n; r++)
      {
        uint64_t tmp1 = pmod_mat_entry(P00nt, MEDS_n, MEDS_m, r, c);
        uint64_t tmp2 = sol[2 * MEDS_m * MEDS_n - (MEDS_m - 1) - (MEDS_m - 1 - b) * MEDS_m + c];

        uint64_t prod = (tmp1 * tmp2) % MEDS_p;

        uint64_t val = sol[b * MEDS_n + r];

        val = ((MEDS_p + val) - prod) % MEDS_p;

        sol[b * MEDS_n + r] = val;
      }

  LOG_VEC(sol, MEDS_m * MEDS_m + MEDS_n * MEDS_n);

  // Conditionally swap solution entry.
  for (int i = 0; i < MEDS_m - 1; i++)
    GFq_cswap(&sol[MEDS_m * MEDS_m + MEDS_n * MEDS_n - MEDS_m + i],
              &sol[MEDS_m * MEDS_m + MEDS_n * MEDS_n - 1],
              eq(i, N1_r));

  LOG_VEC(sol, MEDS_m * MEDS_m + MEDS_n * MEDS_n);

  for (int i = 0; i < MEDS_m * MEDS_m; i++)
    A_tilde[i] = sol[i + MEDS_n * MEDS_n];

  for (int i = 0; i < MEDS_n * MEDS_n; i++)
    B_tilde_inv[i] = sol[i];

  LOG_MAT(A_tilde, MEDS_m, MEDS_m);
  LOG_MAT(B_tilde_inv, MEDS_n, MEDS_n);

  PROFILER_STOP("solve_vec");
  }
  */

int solve_vec(pmod_mat_vec_t *A_tilde, pmod_mat_vec_t *B_tilde_inv, pmod_mat_vec_t *G0prime)
{
  _Static_assert(MEDS_n == MEDS_m + 1, "solve_opt requires MEDS_n == MEDS_m+1");

  pmod_mat_s_vec_t ret = ZERO_S_VEC;
  pmod_mat_s_vec_t minus_one = SET_S_VEC(-1);

  PROFILER_START("solve_vec");

  // set up core sub-system
  pmod_mat_vec_t N[MEDS_n * (2 * MEDS_m)] = {0};

  for (int i = 0; i < MEDS_m; i++)
    for (int j = 0; j < MEDS_n; j++)
    {
      // There was a % in the original, but that should not be necessary
      pmod_mat_set_entry(N, MEDS_n, 2 * MEDS_m, j, i, SUB_LOW_VEC(MEDS_p_VEC, pmod_mat_entry((G0prime + (MEDS_m * MEDS_n)), MEDS_m, MEDS_n, i, j)));

      pmod_mat_set_entry(N, MEDS_n, 2 * MEDS_m, j, MEDS_m + i,
                         pmod_mat_entry(G0prime, MEDS_m, MEDS_n, i, j));
    }

  // Systemize core sub-system while pivoting all but the last row.
  pmod_mat_s_vec_t piv = pmod_mat_syst_ct_partial_vec(N, MEDS_n, 2 * MEDS_m, MEDS_n - 1);

  // Conditional move -1 to ret if piv is not zero
  pmod_mat_s_vec_cmov(&ret, &minus_one, NOT_S_VEC(TO_S_VEC(EQ0_S_VEC(piv))));

  // Extract 2nd sub-system.
  pmod_mat_vec_t N1[(MEDS_m - 1) * MEDS_m] = {0};

  for (int i = 0; i < MEDS_m; i++)
  {
    pmod_mat_set_entry(N1, MEDS_n, MEDS_m, 0, i,
                       pmod_mat_entry(N, MEDS_n, 2 * MEDS_m, MEDS_n - 1, MEDS_m + i));
  }

  // Remove front diagonale of core sub-system.
  for (int i = 0; i < MEDS_n; i++)
    for (int j = 0; j < MEDS_m; j++)
      pmod_mat_set_entry(N, MEDS_n - 1, MEDS_m, i, j,
                         pmod_mat_entry(N, MEDS_n - 1, 2 * MEDS_m, i, MEDS_m + j));

  // Fully reduce 2nd sub-system.
  for (int row = 1; row < MEDS_m - 1; row++)
    for (int i = 0; i < MEDS_m; i++)
      for (int j = 0; j < MEDS_m; j++)
      {
        pmod_mat_vec_t tmp0 = pmod_mat_entry(N1, MEDS_m - 1, MEDS_m, row - 1, i);
        pmod_mat_vec_t tmp1 = pmod_mat_entry(N, MEDS_n - 1, MEDS_m, i, j);

        pmod_mat_vec_t prod = freeze_vec(reduce_vec(MUL_VEC(tmp0, tmp1)));

        pmod_mat_vec_t tmp2 = pmod_mat_entry(N1, MEDS_m - 1, MEDS_m, row, j);

        pmod_mat_vec_t diff = freeze_vec(SUB_LOW_VEC(ADD_LOW_VEC(MEDS_p_VEC, tmp2), prod));

        pmod_mat_set_entry(N1, MEDS_m - 1, MEDS_m, row, j, diff);
      }

  GFq_vec_t sol[MEDS_m * MEDS_m + MEDS_n * MEDS_n] = {0};

  pmod_mat_s_vec_t N1_r = pmod_mat_rref_vec(N1, MEDS_m - 1, MEDS_m);

  // Conditionally move -1 to ret if N1_r is -1
  pmod_mat_s_vec_cmov(&ret, &minus_one, TO_S_VEC(EQ_S_VEC(N1_r, minus_one)));

  // Fill in solutions from the 2nd sub-system.
  for (int i = 0; i < MEDS_m - 1; i++)
    sol[2 * MEDS_m * MEDS_n - (MEDS_m - 1) + i] = pmod_mat_entry(N1, MEDS_m - 1, MEDS_m, i, MEDS_m - 1);

  // Fix to -1.
  sol[MEDS_m * MEDS_m + MEDS_n * MEDS_n - 1] = SET_VEC(MEDS_p - 1);

  // Fill in solutions from core sub-system
  pmod_mat_vec_t N_swap[(MEDS_n - 1) * MEDS_m] = {0};

  // TODO: Does this work for NEON registers?
  memcpy(N_swap, N, sizeof(N_swap));

  for (int r = 0; r < MEDS_m; r++)
    for (int c = 0; c < MEDS_n - 1; c++)
      pmod_mat_vec_cswap(&pmod_mat_entry(N_swap, MEDS_n - 1, MEDS_m, r, c),
                         &pmod_mat_entry(N_swap, MEDS_n - 1, MEDS_m, r, MEDS_m - 1),
                         EQ_S_VEC(SET_S_VEC(c), N1_r));

  for (int r = 0; r < MEDS_m; r++)
    sol[2 * MEDS_m * MEDS_n - MEDS_m - (MEDS_m - 1) + r] = pmod_mat_entry(N_swap, MEDS_n - 1, MEDS_m, r, MEDS_m - 1);

  // Back-substitute second last block.
  for (int c = MEDS_m - 2; c >= 0; c--)
    for (int r = 0; r < MEDS_m; r++)
    {
      pmod_mat_vec_t tmp1 = pmod_mat_entry(N_swap, MEDS_n - 1, MEDS_m, r, c);
      pmod_mat_vec_t tmp2 = sol[2 * MEDS_m * MEDS_n - (MEDS_m - 1) + c];

      pmod_mat_vec_t prod = freeze_vec(reduce_vec(MUL_VEC(tmp1, tmp2)));

      pmod_mat_vec_t val = sol[2 * MEDS_m * MEDS_n - MEDS_m - (MEDS_m - 1) + r];

      val = freeze_vec(SUB_LOW_VEC(ADD_LOW_VEC(MEDS_p_VEC, val), prod));

      sol[2 * MEDS_m * MEDS_n - MEDS_m - (MEDS_m - 1) + r] = val;
    }

  // Prepare substitution-source of first block in bottom part.
  pmod_mat_vec_t P01nt[MEDS_n * MEDS_m] = {0};

  for (int i = 0; i < MEDS_n; i++)
    for (int j = 0; j < MEDS_m; j++)
      pmod_mat_set_entry(P01nt, MEDS_n, MEDS_m, i, j, SUB_LOW_VEC(MEDS_p_VEC, pmod_mat_entry((G0prime + (MEDS_m * MEDS_n)), MEDS_m, MEDS_n, j, i)));

  // Conditionally swap columns.
  for (int r = 0; r < MEDS_n; r++)
    for (int c = 0; c < MEDS_m; c++)
      pmod_mat_vec_cswap(&pmod_mat_entry(P01nt, MEDS_n, MEDS_m, r, c),
                         &pmod_mat_entry(P01nt, MEDS_n, MEDS_m, r, MEDS_m - 1),
                         EQ_S_VEC(SET_S_VEC(c), N1_r));

  // Transfer last column to solution vector.
  for (int r = 0; r < MEDS_n; r++)
    sol[MEDS_m * MEDS_n + r] = pmod_mat_entry(P01nt, MEDS_n, MEDS_m, r, MEDS_m - 1);

  // Perfrom back-substution for first block in bottom part.
  for (int c = MEDS_m - 2; c >= 0; c--)
    for (int r = 0; r < MEDS_n; r++)
    {
      pmod_mat_vec_t tmp1 = pmod_mat_entry(P01nt, MEDS_n, MEDS_m, r, c);
      pmod_mat_vec_t tmp2 = pmod_mat_entry(N1, MEDS_m - 1, MEDS_m, c, MEDS_m - 1);

      pmod_mat_vec_t prod = freeze_vec(reduce_vec(MUL_VEC(tmp1, tmp2)));

      pmod_mat_vec_t val = sol[MEDS_m * MEDS_n + r];

      val = freeze_vec(SUB_LOW_VEC(ADD_LOW_VEC(MEDS_p_VEC, val), prod));

      sol[MEDS_m * MEDS_n + r] = val;
    }

  // Prepare substitution-source for top part.
  pmod_mat_vec_t P00nt[MEDS_n * MEDS_m] = {0};

  for (int i = 0; i < MEDS_n; i++)
    for (int j = 0; j < MEDS_m; j++)
      pmod_mat_set_entry(P00nt, MEDS_n, MEDS_m, i, j, SUB_LOW_VEC(MEDS_p_VEC, pmod_mat_entry(G0prime, MEDS_m, MEDS_n, j, i)));

  pmod_mat_vec_t P00nt_swap[MEDS_n * MEDS_m] = {0};

  memcpy(P00nt_swap, P00nt, sizeof(P00nt_swap));

  for (int r = 0; r < MEDS_n; r++)
    for (int c = 0; c < MEDS_m; c++)
      pmod_mat_vec_cswap(&pmod_mat_entry(P00nt_swap, MEDS_n, MEDS_m, r, c),
                         &pmod_mat_entry(P00nt_swap, MEDS_n, MEDS_m, r, MEDS_m - 1),
                         EQ_S_VEC(SET_S_VEC(c), N1_r));

  // For the last block in the top part, transfer last column to solution vector.
  for (int r = 0; r < MEDS_n; r++)
    sol[(MEDS_m - 1) * MEDS_n + r] = pmod_mat_entry(P00nt_swap, MEDS_n, MEDS_m, r, MEDS_m - 1);

  // Perfrom back-substution for last block in top part.
  for (int c = MEDS_m - 2; c >= 0; c--)
    for (int r = 0; r < MEDS_n; r++)
    {
      pmod_mat_vec_t tmp1 = pmod_mat_entry(P00nt_swap, MEDS_n, MEDS_m, r, c);
      pmod_mat_vec_t tmp2 = pmod_mat_entry(N1, MEDS_m - 1, MEDS_m, c, MEDS_m - 1);

      pmod_mat_vec_t prod = freeze_vec(reduce_vec(MUL_VEC(tmp1, tmp2)));

      pmod_mat_vec_t val = sol[(MEDS_m - 1) * MEDS_n + r];

      val = freeze_vec(SUB_LOW_VEC(ADD_LOW_VEC(MEDS_p_VEC, val), prod));

      sol[(MEDS_m - 1) * MEDS_n + r] = val;
    }

  // Perfrom back-substution for all remaining blocks in bottom part.
  for (int b = MEDS_m - 3; b >= 0; b--)
    for (int c = MEDS_m - 1; c >= 0; c--)
      for (int r = 0; r < MEDS_m; r++)
      {
        pmod_mat_vec_t tmp1 = pmod_mat_entry(N, MEDS_n - 1, MEDS_m, r, c);
        pmod_mat_vec_t tmp2 = sol[(MEDS_m + 1) * MEDS_n + b * MEDS_m + MEDS_m + c];

        pmod_mat_vec_t prod = freeze_vec(reduce_vec(MUL_VEC(tmp1, tmp2)));

        pmod_mat_vec_t val = sol[(MEDS_m + 1) * MEDS_n + b * MEDS_m + r];

        val = freeze_vec(SUB_LOW_VEC(ADD_LOW_VEC(MEDS_p_VEC, val), prod));

        sol[(MEDS_m + 1) * MEDS_n + b * MEDS_m + r] = val;
      }

  // Perfrom back-substution for all remaining blocks in top part.
  for (int b = MEDS_m - 2; b >= 0; b--)
    for (int c = MEDS_m - 1; c >= 0; c--)
      for (int r = 0; r < MEDS_n; r++)
      {
        pmod_mat_vec_t tmp1 = pmod_mat_entry(P00nt, MEDS_n, MEDS_m, r, c);
        pmod_mat_vec_t tmp2 = sol[2 * MEDS_m * MEDS_n - (MEDS_m - 1) - (MEDS_m - 1 - b) * MEDS_m + c];

        pmod_mat_vec_t prod = freeze_vec(reduce_vec(MUL_VEC(tmp1, tmp2)));

        pmod_mat_vec_t val = sol[b * MEDS_n + r];

        val = freeze_vec(SUB_LOW_VEC(ADD_LOW_VEC(MEDS_p_VEC, val), prod));

        sol[b * MEDS_n + r] = val;
      }

  // Conditionally swap solution entry.
  for (int i = 0; i < MEDS_m - 1; i++)
    pmod_mat_vec_cswap(&sol[MEDS_m * MEDS_m + MEDS_n * MEDS_n - MEDS_m + i],
                       &sol[MEDS_m * MEDS_m + MEDS_n * MEDS_n - 1],
                       EQ_S_VEC(SET_S_VEC(i), N1_r));

  for (int i = 0; i < MEDS_m * MEDS_m; i++)
    A_tilde[i] = sol[i + MEDS_n * MEDS_n];

  for (int i = 0; i < MEDS_n * MEDS_n; i++)
    B_tilde_inv[i] = sol[i];

  PROFILER_STOP("solve_vec");
  return 0;
}