#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <assert.h>

#include <sys/random.h>
#include <sys/time.h>

#include <arm_neon.h>

#include "util.h"
#include "fips202.h"
#include "params.h"
#include "api.h"
#include "meds.h"
#include "matrixmod.h"
#include "cyclecounter.h"
#include "profiler.h"

profileresult profileresults[1000];
int number_of_profileresults = 0;
int profiler_enabled = 0;

#define ROUNDS 1000

// __attribute__((optimize("no-tree-vectorize")))
int test_pmod_mat_syst_ct_partial_swap_backsub(pmod_mat_t *M, int M_r, int M_c, int max_r, int swap, int backsub)
{
  int ret = M_r * swap;

  for (int r = 0; r < max_r; r++)
  {
    /*
    if (swap)
    {
      GFq_t z = 0;

      // compute condition for swap
      for (int r2 = r; r2 < M_r; r2++)
        z |= pmod_mat_entry(M, M_r, M_c, r2, r);

      int do_swap = GFq_eq0(z);

      // conditional swap
      {
        ret = r * do_swap + ret * (1 - do_swap);
        for (int i = 0; i < M_r; i++)
          GFq_cswap(&pmod_mat_entry(M, M_r, M_c, i, r),
                    &pmod_mat_entry(M, M_r, M_c, i, M_c - 1),
                    do_swap);
      }
    }*/

    for (int r2 = r + 1; r2 < M_r; r2++)
    {
      uint64_t Mrr = pmod_mat_entry(M, M_r, M_c, r, r);

      for (int c = r; c < M_c; c++)
      {
        uint64_t val = pmod_mat_entry(M, M_r, M_c, r2, c);

        uint64_t Mrc = pmod_mat_entry(M, M_r, M_c, r, c);

        pmod_mat_set_entry(M, M_r, M_c, r, c, (Mrc + val * GFq_eq0(Mrr)) % MEDS_p);
      }
    }

    /*
    uint64_t val = pmod_mat_entry(M, M_r, M_c, r, r);

    if (val == 0)
      return -1;

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

  return ret;

  /*
  if (!backsub)
  {
    return ret;
  }

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

  return ret;*/
}

extern int pmod_mat_syst_test(pmod_mat_t *M, int M_r, int M_c);

#define A_ROWS 4
#define A_COLS 4

int main(int argc, char *argv[])
{
  enable_cyclecounter();

  uint8_t seed1[MEDS_pub_seed_bytes];

  for (int i = 0; i < MEDS_pub_seed_bytes; i++)
  {
    seed1[i] = i;
  }

  __attribute__((aligned(16))) pmod_mat_t A1[A_ROWS * A_COLS];
  __attribute__((aligned(16))) pmod_mat_t A2[A_ROWS * A_COLS];

  keccak_state shake;
  shake256_absorb_once(&shake, seed1, MEDS_pub_seed_bytes);

  long long old_systemizer_cycles[ROUNDS];
  long long new_systemizer_cycles[ROUNDS];

  // Fill matrices with random values
  for (int r = 0; r < A_ROWS; r++)
    for (int c = 0; c < A_COLS; c++)
    {
      GFq_t rand_el = rnd_GF(&shake);
      pmod_mat_set_entry(A1, A_ROWS, A_COLS, r, c, rand_el);
      pmod_mat_set_entry(A2, A_ROWS, A_COLS, r, c, rand_el);
    }
  pmod_mat_set_entry(A1, A_ROWS, A_COLS, 0, 0, 0);
  pmod_mat_set_entry(A2, A_ROWS, A_COLS, 0, 0, 0);

  printf("A:\n");
  pmod_mat_fprint(stdout, A1, A_ROWS, A_COLS);

  for (int round = 0; round < ROUNDS - 1; round++)
  {
    old_systemizer_cycles[round] = get_cyclecounter();
    test_pmod_mat_syst_ct_partial_swap_backsub(A1, A_ROWS, A_COLS, A_ROWS, 0, 0);
  }
  old_systemizer_cycles[ROUNDS - 1] = get_cyclecounter();

  for (int round = 0; round < ROUNDS - 1; round++)
  {
    new_systemizer_cycles[round] = get_cyclecounter();
    pmod_mat_syst_test(A2, A_ROWS, A_COLS);
  }
  new_systemizer_cycles[ROUNDS - 1] = get_cyclecounter();

  printf("A1:\n");
  pmod_mat_fprint(stdout, A1, A_ROWS, A_COLS);
  printf("A2:\n");
  pmod_mat_fprint(stdout, A2, A_ROWS, A_COLS);

  double old_systemizer_median_cc = median_2(old_systemizer_cycles, ROUNDS, 0);
  double new_systemizer_median_cc = median_2(new_systemizer_cycles, ROUNDS, 0);

  // Print results
  double percentage = new_systemizer_median_cc / old_systemizer_median_cc * 100;
  double improvement = (new_systemizer_median_cc - old_systemizer_median_cc) / old_systemizer_median_cc * 100;
  printf("Old median: %f\n", old_systemizer_median_cc);
  printf("New median: %f\n", new_systemizer_median_cc);
  printf("Percentage: %f%%\n", percentage);
  printf("Improvement: %f%%\n", improvement);

  disable_cyclecounter();

  // Compare matrices
  int equalities = 0;
  for (int r = 0; r < A_ROWS; r++)
    for (int c = 0; c < A_COLS; c++)
      if (pmod_mat_entry(A1, A_ROWS, A_COLS, r, c) == pmod_mat_entry(A2, A_ROWS, A_COLS, r, c))
        equalities++;

  int expected_equalities = A_ROWS * A_COLS;

  if (expected_equalities == equalities)
  {
    printf("Matrices are EQUAL\n");
  }
  else
  {
    printf("Equalities: %d / %d\n", equalities, expected_equalities);
    printf("Matrices are NOT EQUAL\n");
    exit(-1);
  }

  return 0;
}