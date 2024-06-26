#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"
#include "fips202.h"
#include "log.h"
#include "profiler.h"
#include "meds.h"
#include "matrixmod.h"

void XOF(uint8_t **buf, size_t *length, const uint8_t *seed, size_t seed_len, int num)
{
  keccak_state shake;

  shake256_absorb_once(&shake, seed, seed_len);

  for (int i = 0; i < num; i++)
    shake256_squeeze(buf[i], length[i], &shake);
}

GFq_t rnd_GF(keccak_state *shake)
{
  GFq_t val = MEDS_p;

  while (val >= MEDS_p)
  {
    uint8_t data[sizeof(GFq_t)];

    shake256_squeeze(data, sizeof(GFq_t), shake);

    val = 0;
    for (int i = 0; i < sizeof(GFq_t); i++)
      val |= data[i] << (i * 8);

    val = val & ((1 << GFq_bits) - 1);
  }

  return val;
}

void rnd_sys_mat(pmod_mat_t *M, int M_r, int M_c, const uint8_t *seed, size_t seed_len)
{
  PROFILER_START("rnd_sys_mat");
  keccak_state shake;
  shake256_absorb_once(&shake, seed, seed_len);

  for (int r = 0; r < M_r; r++)
    for (int c = M_r; c < M_c; c++)
      pmod_mat_set_entry(M, M_r, M_c, r, c, rnd_GF(&shake));

  for (int r = 0; r < M_r; r++)
    for (int c = 0; c < M_r; c++)
      if (r == c)
        pmod_mat_set_entry(M, M_r, M_c, r, c, 1);
      else
        pmod_mat_set_entry(M, M_r, M_c, r, c, 0);
  PROFILER_STOP("rnd_sys_mat");
}

void rnd_inv_matrix(pmod_mat_t *M, int M_r, int M_c, uint8_t *seed, size_t seed_len, int (*pmod_mat_syst_fun)(pmod_mat_t *))
{
  PROFILER_START("rnd_inv_matrix");
  keccak_state shake;
  shake256_absorb_once(&shake, seed, seed_len);

  while (0 == 0)
  {
    for (int r = 0; r < M_r; r++)
      for (int c = 0; c < M_c; c++)
        pmod_mat_set_entry(M, M_r, M_c, r, c, rnd_GF(&shake));

    pmod_mat_t tmp[M_r * M_c];

    memcpy(tmp, M, M_r * M_c * sizeof(GFq_t));

    PROFILER_START("pmod_mat_syst");
    if (pmod_mat_syst_fun(tmp) == 0)
    {
      PROFILER_STOP("pmod_mat_syst");
      PROFILER_STOP("rnd_inv_matrix");
      return;
    }
  }
}

int parse_hash(uint8_t *digest, int digest_len, uint8_t *h, int len_h)
{
  PROFILER_START("parse_hash");
  if (len_h < MEDS_t)
    return -1;

#ifdef DEBUG
  fprintf(stderr, "(%s) digest: [", __func__);
  for (int i = 0; i < MEDS_digest_bytes; i++)
  {
    fprintf(stderr, "%i", digest[i]);
    if (i < MEDS_digest_bytes - 1)
      fprintf(stderr, ", ");
  }
  fprintf(stderr, "]\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "(%s) digest len: %i\n", __func__, digest_len);
  fprintf(stderr, "\n");
#endif

  keccak_state shake;

  shake256_init(&shake);
  shake256_absorb(&shake, digest, digest_len);
  shake256_finalize(&shake);

  for (int i = 0; i < MEDS_t; i++)
    h[i] = 0;

  int i = 0;
  while (i < MEDS_w)
  {
    uint64_t pos = 0;
    uint8_t buf[MEDS_t_bytes] = {0};

    shake256_squeeze(buf, MEDS_t_bytes, &shake);

    for (int j = 0; j < MEDS_t_bytes; j++)
      pos |= buf[j] << (j * 8);

    pos = pos & MEDS_t_mask;

    if (pos >= MEDS_t)
      continue;

    if (h[pos] > 0)
      continue;

#ifdef DEBUG
    fprintf(stderr, "(%s) pos: %lu\n", __func__, pos);
    fprintf(stderr, "\n");
#endif

    uint8_t val = 0;

    while ((val < 1) || (val > MEDS_s - 1))
    {
      shake256_squeeze(&val, 1, &shake);
      val = val & MEDS_s_mask;
    }

#ifdef DEBUG
    fprintf(stderr, "(%s) p: %lu  v: %u\n", __func__, pos, val);
    fprintf(stderr, "\n");
#endif

    h[pos] = val;

    i++;
  }

  PROFILER_STOP("parse_hash");
  return 0;
}

// For testing only! NOT CONSTANT TIME!
// int solve_symb(pmod_mat_t *A, pmod_mat_t *B_inv, pmod_mat_t *G0prime)
// {
//   _Static_assert(MEDS_n == MEDS_m + 1, "solve_symb requires MEDS_n == MEDS_m+1");

//   pmod_mat_t Pj0[MEDS_m * MEDS_n] = {0};
//   pmod_mat_t Pj1[MEDS_m * MEDS_n] = {0};
//   pmod_mat_t *Pj[2] = {Pj0, Pj1};

//   for (int i = 0; i < MEDS_m; i++)
//   {
//     Pj0[i * (MEDS_m + 1)] = 1;
//     Pj1[i * (MEDS_m + 1) + 1] = 1;
//   }

//   pmod_mat_t rsys[(2 * MEDS_m * MEDS_n) * (MEDS_m * MEDS_m + MEDS_n * MEDS_n)] = {0};

//   pmod_mat_t *tmp = rsys;

//   // set up lineat eq system
//   for (int l = 0; l < MEDS_m; l++)
//     for (int j = 0; j < MEDS_n; j++)
//     {
//       for (int ii = 0; ii < MEDS_n; ii++)
//         tmp[ii * MEDS_n + j] = Pj[0][l * MEDS_m + ii];

//       for (int ii = 0; ii < MEDS_m; ii++)
//         tmp[MEDS_n * MEDS_n + l * MEDS_m + ii] = (MEDS_p - G0prime[ii * MEDS_n + j]) % MEDS_p;

//       tmp += MEDS_m * MEDS_m + MEDS_n * MEDS_n;
//     }

//   for (int l = 0; l < MEDS_m; l++)
//     for (int j = 0; j < MEDS_n; j++)
//     {
//       for (int ii = 0; ii < MEDS_n; ii++)
//         tmp[ii * MEDS_n + j] = Pj[1][l * MEDS_m + ii];

//       for (int ii = 0; ii < MEDS_m; ii++)
//         tmp[MEDS_n * MEDS_n + l * MEDS_m + ii] = (MEDS_p - G0prime[MEDS_m * MEDS_n + ii * MEDS_n + j]) % MEDS_p;

//       tmp += MEDS_m * MEDS_m + MEDS_n * MEDS_n;
//     }

//   LOG_MAT(rsys, (2 * MEDS_m * MEDS_n), (MEDS_m * MEDS_m + MEDS_n * MEDS_n));

//   int N1_r = pmod_mat_syst_2mn_mmann_2mn_1_1(rsys);

//   LOG_MAT(rsys, (2 * MEDS_m * MEDS_n), (MEDS_m * MEDS_m + MEDS_n * MEDS_n));

//   LOG("N1_r: %i", N1_r);

//   GFq_t sol[MEDS_m * MEDS_m + MEDS_n * MEDS_n];

//   for (int i = 0; i < 2 * MEDS_m * MEDS_n; i++)
//     sol[i] = rsys[i * (MEDS_m * MEDS_m + MEDS_n * MEDS_n) + MEDS_m * MEDS_m + MEDS_n * MEDS_n - 1];

//   sol[MEDS_m * MEDS_m + MEDS_n * MEDS_n - 1] = MEDS_p - 1;

//   LOG_VEC(sol, MEDS_m * MEDS_m + MEDS_n * MEDS_n);

//   // Swap solution entry.
//   GFq_t diff = sol[MEDS_m * MEDS_m + MEDS_n * MEDS_n - 1] ^ sol[N1_r]; // NOT CONSTANT TIME!

//   sol[N1_r] ^= diff; // NOT CONSTANT TIME!
//   sol[MEDS_m * MEDS_m + MEDS_n * MEDS_n - 1] ^= diff;

//   LOG_VEC(sol, MEDS_m * MEDS_m + MEDS_n * MEDS_n);

//   for (int i = 0; i < MEDS_m * MEDS_m; i++)
//     A[i] = sol[i + MEDS_n * MEDS_n];

//   for (int i = 0; i < MEDS_n * MEDS_n; i++)
//     B_inv[i] = sol[i];

//   LOG_MAT(A, MEDS_m, MEDS_m);
//   LOG_MAT(B_inv, MEDS_n, MEDS_n);

//   return 0;
// }

int solve_opt(pmod_mat_t *A_tilde, pmod_mat_t *B_tilde_inv, pmod_mat_t *G0prime, int ct)
{
  _Static_assert(MEDS_n == MEDS_m + 1, "solve_opt requires MEDS_n == MEDS_m+1");

  PROFILER_START("solve_opt");
  PROFILER_START("solve_opt_raw");

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
  PROFILER_STOP("solve_opt_raw");

  PROFILER_START("pmod_mat_syst");
  int piv = ct ? pmod_mat_syst_n_2m_nr1_0_1(N) : pmod_mat_syst_n_2m_nr1_0_1_nct(N, mod_inverse_table);
  PROFILER_STOP("pmod_mat_syst");
  if (piv != 0)
  {
    LOG("no sol %i", __LINE__);
    PROFILER_STOP("solve_opt");
    return -1;
  }
  PROFILER_START("solve_opt_raw");

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
  PROFILER_START("solve_opt_raw_1");
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
  PROFILER_STOP("solve_opt_raw_1");

  LOG_MAT(N1, MEDS_m - 1, MEDS_m);

  GFq_t sol[MEDS_m * MEDS_m + MEDS_n * MEDS_n] = {0};

  int N1_r;

  PROFILER_STOP("solve_opt_raw");
  // Sytemize 2nd sub-system.
  PROFILER_START("pmod_mat_syst");
  N1_r = ct ? pmod_mat_syst_mr1_m_mr1_1_1(N1) : pmod_mat_syst_mr1_m_mr1_1_1_nct(N1, mod_inverse_table);
  PROFILER_STOP("pmod_mat_syst");

  if (N1_r == -1)
  {
    LOG("RREF failed. Redo.\n");
    PROFILER_STOP("solve_opt");
    return -1;
  }
  PROFILER_START("solve_opt_raw");

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

  if (ct == 1)
  {
    for (int r = 0; r < MEDS_m; r++)
      for (int c = 0; c < MEDS_n - 1; c++)
        GFq_cswap(&pmod_mat_entry(N_swap, MEDS_n - 1, MEDS_m, r, c),
                  &pmod_mat_entry(N_swap, MEDS_n - 1, MEDS_m, r, MEDS_m - 1),
                  eq(c, N1_r));
  }
  else
  {
    for (int r = 0; r < MEDS_m; r++)
    {
      uint16_t a = pmod_mat_entry(N_swap, MEDS_n - 1, MEDS_m, r, N1_r);
      uint16_t b = pmod_mat_entry(N_swap, MEDS_n - 1, MEDS_m, r, MEDS_m - 1);
      pmod_mat_set_entry(N_swap, MEDS_n - 1, MEDS_m, r, N1_r, b);
      pmod_mat_set_entry(N_swap, MEDS_n - 1, MEDS_m, r, MEDS_m - 1, a);
    }
  }

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
  if (ct == 1)
  {
    for (int r = 0; r < MEDS_n; r++)
      for (int c = 0; c < MEDS_m; c++)
        GFq_cswap(&pmod_mat_entry(P01nt, MEDS_n, MEDS_m, r, c),
                  &pmod_mat_entry(P01nt, MEDS_n, MEDS_m, r, MEDS_m - 1),
                  eq(c, N1_r));
  }
  else
  {
    for (int r = 0; r < MEDS_n; r++)
    {
      uint16_t a = pmod_mat_entry(P01nt, MEDS_n, MEDS_m, r, N1_r);
      uint16_t b = pmod_mat_entry(P01nt, MEDS_n, MEDS_m, r, MEDS_m - 1);
      pmod_mat_set_entry(P01nt, MEDS_n, MEDS_m, r, N1_r, b);
      pmod_mat_set_entry(P01nt, MEDS_n, MEDS_m, r, MEDS_m - 1, a);
    }
  }

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

  if (ct == 1)
  {
    for (int r = 0; r < MEDS_n; r++)
      for (int c = 0; c < MEDS_m; c++)
        GFq_cswap(&pmod_mat_entry(P00nt_swap, MEDS_n, MEDS_m, r, c),
                  &pmod_mat_entry(P00nt_swap, MEDS_n, MEDS_m, r, MEDS_m - 1),
                  eq(c, N1_r));
  }
  else
  {
    for (int r = 0; r < MEDS_n; r++)
    {
      uint16_t a = pmod_mat_entry(P00nt_swap, MEDS_n, MEDS_m, r, N1_r);
      uint16_t b = pmod_mat_entry(P00nt_swap, MEDS_n, MEDS_m, r, MEDS_m - 1);
      pmod_mat_set_entry(P00nt_swap, MEDS_n, MEDS_m, r, N1_r, b);
      pmod_mat_set_entry(P00nt_swap, MEDS_n, MEDS_m, r, MEDS_m - 1, a);
    }
  }

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

  PROFILER_START("solve_opt_raw_2");
  // Perfrom back-substution for all remaining blocks in bottom part.
  for (int b = MEDS_m - 3; b >= 0; b--)
    for (int c = MEDS_m - 1; c >= 0; c--)
    {
      uint16x8_t tmp2_16x8 = vdupq_n_u16(sol[(MEDS_m + 1) * MEDS_n + b * MEDS_m + MEDS_m + c]);
      uint16x4_t tmp2_16x4 = vdup_n_u16(sol[(MEDS_m + 1) * MEDS_n + b * MEDS_m + MEDS_m + c]);
      uint16_t tmp2 = sol[(MEDS_m + 1) * MEDS_n + b * MEDS_m + MEDS_m + c];
      int r = 0;
      for (; r < MEDS_m - 8; r += 8)
      {
        // uint64_t tmp1 = pmod_mat_entry(N, MEDS_n - 1, MEDS_m, r, c);
        uint16_t tmp1_0 = pmod_mat_entry(N, MEDS_n - 1, MEDS_m, r, c);
        uint16_t tmp1_1 = pmod_mat_entry(N, MEDS_n - 1, MEDS_m, r + 1, c);
        uint16_t tmp1_2 = pmod_mat_entry(N, MEDS_n - 1, MEDS_m, r + 2, c);
        uint16_t tmp1_3 = pmod_mat_entry(N, MEDS_n - 1, MEDS_m, r + 3, c);
        uint16_t tmp1_4 = pmod_mat_entry(N, MEDS_n - 1, MEDS_m, r + 4, c);
        uint16_t tmp1_5 = pmod_mat_entry(N, MEDS_n - 1, MEDS_m, r + 5, c);
        uint16_t tmp1_6 = pmod_mat_entry(N, MEDS_n - 1, MEDS_m, r + 6, c);
        uint16_t tmp1_7 = pmod_mat_entry(N, MEDS_n - 1, MEDS_m, r + 7, c);

        uint16x8_t tmp1 = {tmp1_0, tmp1_1, tmp1_2, tmp1_3, tmp1_4, tmp1_5, tmp1_6, tmp1_7};

        uint32x4_t prod_low = vmull_u16(vget_low_u16(tmp1), vget_low_u16(tmp2_16x8));
        uint32x4_t prod_high = vmull_high_u16(tmp1, tmp2_16x8);
        uint16x8_t prod_low_red = (uint16x8_t)FREEZE_REDUCE_VEC_16x4(prod_low);
        uint16x8_t prod_high_red = (uint16x8_t)FREEZE_REDUCE_VEC_16x4(prod_high);
        uint16x8_t prod = vuzp1q_u16(prod_low_red, prod_high_red);

        // uint64_t val = sol[(MEDS_m + 1) * MEDS_n + b * MEDS_m + r];
        uint16x8_t val = vld1q_u16((uint16_t *)&sol[(MEDS_m + 1) * MEDS_n + b * MEDS_m + r]);

        // val = ((MEDS_p + val) - prod) % MEDS_p;
        val = vsubq_u16(vaddq_u16(val, MEDS_p_VEC_16x8), prod);
        val = FREEZE_VEC_16x8(val);

        // sol[(MEDS_m + 1) * MEDS_n + b * MEDS_m + r] = val;
        vst1q_u16((uint16_t *)&sol[(MEDS_m + 1) * MEDS_n + b * MEDS_m + r], val);
      }
      for (; r < MEDS_m - 4; r += 4)
      {
        // uint64_t tmp1 = pmod_mat_entry(N, MEDS_n - 1, MEDS_m, r, c);
        uint16_t tmp1_0 = pmod_mat_entry(N, MEDS_n - 1, MEDS_m, r, c);
        uint16_t tmp1_1 = pmod_mat_entry(N, MEDS_n - 1, MEDS_m, r + 1, c);
        uint16_t tmp1_2 = pmod_mat_entry(N, MEDS_n - 1, MEDS_m, r + 2, c);
        uint16_t tmp1_3 = pmod_mat_entry(N, MEDS_n - 1, MEDS_m, r + 3, c);
        uint16x4_t tmp1 = {tmp1_0, tmp1_1, tmp1_2, tmp1_3};

        // uint64_t prod = (tmp1 * tmp2) % MEDS_p;
        uint32x4_t prod = vmull_u16(tmp1, tmp2_16x4);
        uint16x4_t prod_red = FREEZE_REDUCE_VEC_16x4_SHRINK(prod);

        // uint64_t val = sol[(MEDS_m + 1) * MEDS_n + b * MEDS_m + r];
        uint16x4_t val = vld1_u16((uint16_t *)&sol[(MEDS_m + 1) * MEDS_n + b * MEDS_m + r]);

        // val = ((MEDS_p + val) - prod) % MEDS_p;
        val = vsub_u16(vadd_u16(val, MEDS_p_VEC_16x4), prod_red);
        val = FREEZE_VEC_16x4(val);

        // sol[(MEDS_m + 1) * MEDS_n + b * MEDS_m + r] = val;
        vst1_u16((uint16_t *)&sol[(MEDS_m + 1) * MEDS_n + b * MEDS_m + r], val);
      }
      for (; r < MEDS_m; r++)
      {
        uint64_t tmp1 = pmod_mat_entry(N, MEDS_n - 1, MEDS_m, r, c);

        uint64_t prod = (tmp1 * tmp2) % MEDS_p;

        uint64_t val = sol[(MEDS_m + 1) * MEDS_n + b * MEDS_m + r];

        val = ((MEDS_p + val) - prod) % MEDS_p;

        sol[(MEDS_m + 1) * MEDS_n + b * MEDS_m + r] = val;
      }
    }
  PROFILER_STOP("solve_opt_raw_2");

  LOG_VEC(sol, MEDS_m * MEDS_m + MEDS_n * MEDS_n);

  // Perfrom back-substution for all remaining blocks in top part.
  PROFILER_START("solve_opt_raw_3");

  for (int b = MEDS_m - 2; b >= 0; b--)
    for (int c = MEDS_m - 1; c >= 0; c--)
    {
      // uint64_t tmp2 = sol[2 * MEDS_m * MEDS_n - (MEDS_m - 1) - (MEDS_m - 1 - b) * MEDS_m + c];
      uint16x8_t tmp2_16x8 = vdupq_n_u16(sol[2 * MEDS_m * MEDS_n - (MEDS_m - 1) - (MEDS_m - 1 - b) * MEDS_m + c]);
      uint16x4_t tmp2_16x4 = vdup_n_u16(sol[2 * MEDS_m * MEDS_n - (MEDS_m - 1) - (MEDS_m - 1 - b) * MEDS_m + c]);
      uint16_t tmp2 = sol[2 * MEDS_m * MEDS_n - (MEDS_m - 1) - (MEDS_m - 1 - b) * MEDS_m + c];
      int r = 0;
      for (; r < MEDS_n - 8; r += 8)
      {
        // uint64_t tmp1 = pmod_mat_entry(P00nt, MEDS_n, MEDS_m, r, c);
        uint16_t tmp1_0 = pmod_mat_entry(P00nt, MEDS_n, MEDS_m, r, c);
        uint16_t tmp1_1 = pmod_mat_entry(P00nt, MEDS_n, MEDS_m, r + 1, c);
        uint16_t tmp1_2 = pmod_mat_entry(P00nt, MEDS_n, MEDS_m, r + 2, c);
        uint16_t tmp1_3 = pmod_mat_entry(P00nt, MEDS_n, MEDS_m, r + 3, c);
        uint16_t tmp1_4 = pmod_mat_entry(P00nt, MEDS_n, MEDS_m, r + 4, c);
        uint16_t tmp1_5 = pmod_mat_entry(P00nt, MEDS_n, MEDS_m, r + 5, c);
        uint16_t tmp1_6 = pmod_mat_entry(P00nt, MEDS_n, MEDS_m, r + 6, c);
        uint16_t tmp1_7 = pmod_mat_entry(P00nt, MEDS_n, MEDS_m, r + 7, c);
        uint16x8_t tmp1 = {tmp1_0, tmp1_1, tmp1_2, tmp1_3, tmp1_4, tmp1_5, tmp1_6, tmp1_7};

        // uint64_t prod = (tmp1 * tmp2) % MEDS_p;
        uint32x4_t prod_low = vmull_u16(vget_low_u16(tmp1), vget_low_u16(tmp2_16x8));
        uint32x4_t prod_high = vmull_high_u16(tmp1, tmp2_16x8);
        uint16x8_t prod_low_red = (uint16x8_t)FREEZE_REDUCE_VEC_16x4(prod_low);
        uint16x8_t prod_high_red = (uint16x8_t)FREEZE_REDUCE_VEC_16x4(prod_high);
        uint16x8_t prod = vuzp1q_u16(prod_low_red, prod_high_red);

        // uint64_t val = sol[b * MEDS_n + r];
        uint16x8_t val = vld1q_u16((uint16_t *)&sol[b * MEDS_n + r]);

        // val = ((MEDS_p + val) - prod) % MEDS_p;
        val = vsubq_u16(vaddq_u16(val, MEDS_p_VEC_16x8), prod);
        val = FREEZE_VEC_16x8(val);

        // sol[b * MEDS_n + r] = val;
        vst1q_u16((uint16_t *)&sol[b * MEDS_n + r], val);
      }
      for (; r < MEDS_n - 4; r += 4)
      {
        // uint64_t tmp1 = pmod_mat_entry(P00nt, MEDS_n, MEDS_m, r, c);
        uint16_t tmp1_0 = pmod_mat_entry(P00nt, MEDS_n, MEDS_m, r, c);
        uint16_t tmp1_1 = pmod_mat_entry(P00nt, MEDS_n, MEDS_m, r + 1, c);
        uint16_t tmp1_2 = pmod_mat_entry(P00nt, MEDS_n, MEDS_m, r + 2, c);
        uint16_t tmp1_3 = pmod_mat_entry(P00nt, MEDS_n, MEDS_m, r + 3, c);
        uint16x4_t tmp1 = {tmp1_0, tmp1_1, tmp1_2, tmp1_3};

        // uint64_t prod = (tmp1 * tmp2) % MEDS_p;
        uint32x4_t prod = vmull_u16(tmp1, tmp2_16x4);
        uint16x4_t prod_red = FREEZE_REDUCE_VEC_16x4_SHRINK(prod);

        // uint64_t val = sol[b * MEDS_n + r];
        uint16x4_t val = vld1_u16((uint16_t *)&sol[b * MEDS_n + r]);

        // val = ((MEDS_p + val) - prod) % MEDS_p;
        val = vsub_u16(vadd_u16(val, MEDS_p_VEC_16x4), prod_red);
        val = FREEZE_VEC_16x4(val);

        // sol[b * MEDS_n + r] = val;
        vst1_u16((uint16_t *)&sol[b * MEDS_n + r], val);
      }
      for (; r < MEDS_n; r++)
      {
        uint64_t tmp1 = pmod_mat_entry(P00nt, MEDS_n, MEDS_m, r, c);

        uint64_t prod = (tmp1 * tmp2) % MEDS_p;

        uint64_t val = sol[b * MEDS_n + r];

        val = ((MEDS_p + val) - prod) % MEDS_p;

        sol[b * MEDS_n + r] = val;
      }
    }
  PROFILER_STOP("solve_opt_raw_3");

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

  PROFILER_STOP("solve_opt_raw");
  PROFILER_STOP("solve_opt");
  return 0;
}

void pi(pmod_mat_t *Gout, pmod_mat_t *A, pmod_mat_t *B, pmod_mat_t *G)
{
  PROFILER_START("pi");
  pmod_mat_t Gtmp[MEDS_k * MEDS_m * MEDS_n];
  for (int i = 0; i < MEDS_k; i++)
  {
    PROFILER_START("pmod_mat_mul");
    PROFILER_START("pmod_mat_mul_asm_m_n_m");
    pmod_mat_mul_8_asm_m_n_m(&Gtmp[i * MEDS_m * MEDS_n], A, &G[i * MEDS_m * MEDS_n]);
    PROFILER_STOP("pmod_mat_mul_asm_m_n_m");
    PROFILER_STOP("pmod_mat_mul");
    PROFILER_START("pmod_mat_mul");
    PROFILER_START("pmod_mat_mul_asm_m_n_n");
    pmod_mat_mul_8_asm_m_n_n(&Gout[i * MEDS_m * MEDS_n], &Gtmp[i * MEDS_m * MEDS_n], B);
    PROFILER_STOP("pmod_mat_mul_asm_m_n_n");
    PROFILER_STOP("pmod_mat_mul");
  }
  PROFILER_STOP("pi");
}

int SF(pmod_mat_t *Gprime, pmod_mat_t *G)
{
  PROFILER_START("SF");
  pmod_mat_t M[MEDS_k * MEDS_k];
  // Temporary matrix so we can use matrix multiplication that stores directly to result matrix
  pmod_mat_t Gtmp[MEDS_k * MEDS_m * MEDS_n];

  for (int r = 0; r < MEDS_k; r++)
  {
    memcpy(&M[r * MEDS_k], &G[r * MEDS_m * MEDS_n], MEDS_k * sizeof(GFq_t));
    // Copy G into Gtmp
    memcpy(&Gtmp[r * MEDS_m * MEDS_n], &G[r * MEDS_m * MEDS_n], MEDS_m * MEDS_n * sizeof(GFq_t));
  }

  if (pmod_mat_inv(M, M, MEDS_k, MEDS_k, pmod_mat_syst_k_2k_k_0_1) == 0)
  {
    PROFILER_START("pmod_mat_mul");
    PROFILER_START("pmod_mat_mul_asm_k_mn_k");
    pmod_mat_mul_8_asm_k_mn_k(Gprime, M, Gtmp);
    PROFILER_STOP("pmod_mat_mul_asm_k_mn_k");
    PROFILER_STOP("pmod_mat_mul");

    PROFILER_STOP("SF");
    return 0;
  }

  PROFILER_STOP("SF");
  return -1;
}
