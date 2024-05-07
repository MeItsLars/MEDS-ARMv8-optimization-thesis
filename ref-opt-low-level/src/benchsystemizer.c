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
    }
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

uint16x4_t reduce(uint32x4_t red, uint16x4_t MEDS_p_16x4, uint32x4_t MEDS_p_32x4)
{
  // >>> Reduce modulo MEDS_p
  // Apply two reductions
  uint32x4_t tmp = vshrq_n_u32(red, GFq_bits);
  tmp = vmulq_n_u32(tmp, MEDS_p);
  red = vsubq_u32(red, tmp);
  tmp = vshrq_n_u32(red, GFq_bits);
  tmp = vmulq_n_u32(tmp, MEDS_p);
  red = vsubq_u32(red, tmp);

  // Reduce to a value between 0 and MEDS_p - 1:
  uint32x4_t diff = vsubq_u32(red, MEDS_p_32x4);
  uint32x4_t mask = vandq_u32(vshrq_n_u32(diff, 31), vdupq_n_u32(1));
  red = vaddq_u32(vmulq_u32(mask, red), vmulq_u32(vsubq_u32(vdupq_n_u32(1), mask), diff));

  // Convert to smaller type
  return vqmovn_u32(red);
}

int test_pmod_mat_syst_ct_partial_swap_backsub_2(pmod_mat_t *M, int M_r, int M_c)
{
  int ret = 0;

  uint16x8_t MEDS_p_16x8 = vdupq_n_u16(MEDS_p);
  uint16x4_t MEDS_p_16x4 = vdup_n_u16(MEDS_p);
  uint32x4_t MEDS_p_32x4 = vdupq_n_u32(MEDS_p);
  uint32x4_t one_32x4 = vdupq_n_u32(1);

  for (int r = 0; r < M_r; r++)
  {
    for (int r2 = r + 1; r2 < M_r; r2++)
    {
      uint64_t Mrr = pmod_mat_entry(M, M_r, M_c, r, r);
      uint16x8_t swap = vdupq_n_u16(GFq_eq0(Mrr));

      int c;
      for (c = r; c < M_c - 8; c += 8)
      {
        // Compute result
        uint16x8_t val = vld1q_u16(&M[(M_c)*r2 + c]);
        uint16x8_t Mrc = vld1q_u16(&M[(M_c)*r + c]);
        uint16x8_t result = vaddq_u16(Mrc, vmulq_u16(val, swap));

        // Subtract MEDS_p if result is larger than MEDS_p
        uint16x8_t mask = vcgtq_u16(result, MEDS_p_16x8);
        result = vsubq_u16(result, vandq_u16(MEDS_p_16x8, mask));

        // Store result
        vst1q_u16(&M[(M_c)*r + c], result);
      }
      for (c = c; c < M_c; c++)
      {
        uint64_t val = pmod_mat_entry(M, M_r, M_c, r2, c);
        uint64_t Mrc = pmod_mat_entry(M, M_r, M_c, r, c);
        pmod_mat_set_entry(M, M_r, M_c, r, c, (Mrc + val * GFq_eq0(Mrr)) % MEDS_p);
      }
    }

    uint64_t val = pmod_mat_entry(M, M_r, M_c, r, r);

    if (val == 0)
      return -1;

    // Compute the multiplicative inverse of the element on the diagonal
    val = GF_inv(val);
    uint16x4_t val_v = vdup_n_u16(val);

    // normalize;
    int c;
    for (c = r; c < M_c - 4; c += 4)
    {
      // Compute result
      uint16x4_t vals = vld1_u16(&M[(M_c)*r + c]);
      uint32x4_t red = vmull_u16(vals, val_v);
      uint16x4_t red_16x4 = reduce(red, MEDS_p_16x4, MEDS_p_32x4);

      // Store result
      vst1_u16(&M[(M_c)*r + c], red_16x4);
    }
    for (c = c; c < M_c; c++)
    {
      uint64_t tmp = ((uint64_t)pmod_mat_entry(M, M_r, M_c, r, c) * val) % MEDS_p;
      pmod_mat_set_entry(M, M_r, M_c, r, c, tmp);
    }

    // eliminate;
    for (int r2 = r + 1; r2 < M_r; r2++)
    {
      // Calculate the factor
      GFq_t factor = pmod_mat_entry(M, M_r, M_c, r2, r);
      uint16x4_t factor_16x4 = vdup_n_u16(factor);

      for (int c = r; c < M_c; c += 4)
      {
        // First, compute val (red)
        uint16x4_t tmp0 = vld1_u16(&M[(M_c)*r + c]);
        uint32x4_t val = vmull_u16(tmp0, factor_16x4);
        uint16x4_t val_16x4 = reduce(val, MEDS_p_16x4, MEDS_p_32x4);

        // Compute the new val
        uint16x4_t tmp1 = vld1_u16(&M[(M_c)*r2 + c]);
        tmp1 = vadd_u16(tmp1, MEDS_p_16x4);
        val_16x4 = vsub_u16(tmp1, val_16x4);

        // Reduce to a value between 0 and MEDS_p - 1:
        uint16x4_t diff = vsub_u16(val_16x4, MEDS_p_16x4);
        uint16x4_t mask = vand_u16(vshr_n_u16(diff, 15), vdup_n_u16(1));
        val_16x4 = vadd_u16(vmul_u16(mask, val_16x4), vmul_u16(vsub_u16(vdup_n_u16(1), mask), diff));

        // Store result
        vst1_u16(&M[(M_c)*r2 + c], val_16x4);
      }
    }
  }

  /*
  // back substitution;
  for (int r = M_r - 1; r >= 0; r--)
    for (int r2 = 0; r2 < r; r2++)
    {
      // Compute the factor
      uint64_t factor = pmod_mat_entry(M, M_r, M_c, r2, r);
      uint16x4_t factor_16x4 = vdup_n_u16(factor);

      // Set the value in the current column
      // >> This section seems to be unnecessary? Removing it still produces the same results
      uint64_t tmp0 = pmod_mat_entry(M, M_r, M_c, r, r);
      uint64_t tmp1 = pmod_mat_entry(M, M_r, M_c, r2, r);

      uint64_t val = (tmp0 * factor) % MEDS_p;

      val = (MEDS_p + tmp1 - val) % MEDS_p;

      pmod_mat_set_entry(M, M_r, M_c, r2, r, val);
      // <<

      // Calculate and set the new values in columns right of the diagonal block.
      // The values in the diagonal block are already 0 so we don't need to do anything with them.
      for (int c = M_r; c < M_c; c += 4)
      {
        // uint64_t tmp0 = pmod_mat_entry(M, M_r, M_c, r, c);
        // uint64_t tmp1 = pmod_mat_entry(M, M_r, M_c, r2, c);

        // uint64_t val = (tmp0 * factor) % MEDS_p;

        // val = (MEDS_p + tmp1 - val) % MEDS_p;

        // pmod_mat_set_entry(M, M_r, M_c, r2, c, val);

        // First, compute val (red)
        uint16x4_t tmp0 = vld1_u16(&M[(M_c)*r + c]);
        uint32x4_t val = vmull_u16(tmp0, factor_16x4);
        uint16x4_t val_16x4 = reduce(val, MEDS_p_16x4, MEDS_p_32x4);

        // Compute the new val
        uint16x4_t tmp1 = vld1_u16(&M[(M_c)*r2 + c]);
        tmp1 = vadd_u16(tmp1, MEDS_p_16x4);
        val_16x4 = vsub_u16(tmp1, val_16x4);

        // Reduce to a value between 0 and MEDS_p - 1:
        uint16x4_t diff = vsub_u16(val_16x4, MEDS_p_16x4);
        uint16x4_t mask = vand_u16(vshr_n_u16(diff, 15), vdup_n_u16(1));
        val_16x4 = vadd_u16(vmul_u16(mask, val_16x4), vmul_u16(vsub_u16(vdup_n_u16(1), mask), diff));

        // Store result
        vst1_u16(&M[(M_c)*r2 + c], val_16x4);
      }
    }*/
  return ret;
}

extern int pmod_mat_syst_test(pmod_mat_t *M);

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
  __attribute__((aligned(16))) pmod_mat_t A3[A_ROWS * A_COLS];

  keccak_state shake;
  shake256_absorb_once(&shake, seed1, MEDS_pub_seed_bytes);

  long long old_systemizer_cycles[ROUNDS];
  long long intrinsic_systemizer_cycles[ROUNDS];
  long long new_systemizer_cycles[ROUNDS];

  // Fill matrices with random values
  for (int r = 0; r < A_ROWS; r++)
    for (int c = 0; c < A_COLS; c++)
    {
      GFq_t rand_el = rnd_GF(&shake);
      pmod_mat_set_entry(A1, A_ROWS, A_COLS, r, c, rand_el);
      pmod_mat_set_entry(A2, A_ROWS, A_COLS, r, c, rand_el);
      pmod_mat_set_entry(A3, A_ROWS, A_COLS, r, c, rand_el);
    }
  pmod_mat_set_entry(A1, A_ROWS, A_COLS, 0, 0, 0);
  pmod_mat_set_entry(A2, A_ROWS, A_COLS, 0, 0, 0);
  pmod_mat_set_entry(A3, A_ROWS, A_COLS, 0, 0, 0);

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
    intrinsic_systemizer_cycles[round] = get_cyclecounter();
    test_pmod_mat_syst_ct_partial_swap_backsub_2(A3, A_ROWS, A_COLS);
  }
  intrinsic_systemizer_cycles[ROUNDS - 1] = get_cyclecounter();

  for (int round = 0; round < ROUNDS - 1; round++)
  {
    new_systemizer_cycles[round] = get_cyclecounter();
    pmod_mat_syst_test(A2);
  }
  new_systemizer_cycles[ROUNDS - 1] = get_cyclecounter();

  printf("A1:\n");
  pmod_mat_fprint(stdout, A1, A_ROWS, A_COLS);
  printf("A2:\n");
  pmod_mat_fprint(stdout, A2, A_ROWS, A_COLS);

  double old_systemizer_median_cc = median_2(old_systemizer_cycles, ROUNDS, 0);
  double intrinsic_systemizer_median_cc = median_2(intrinsic_systemizer_cycles, ROUNDS, 0);
  double new_systemizer_median_cc = median_2(new_systemizer_cycles, ROUNDS, 0);

  // Print results
  double percentage = new_systemizer_median_cc / old_systemizer_median_cc * 100;
  double improvement = (new_systemizer_median_cc - old_systemizer_median_cc) / old_systemizer_median_cc * 100;
  double improvement_intrinsics = (new_systemizer_median_cc - intrinsic_systemizer_median_cc) / intrinsic_systemizer_median_cc * 100;
  printf("Old median: %f\n", old_systemizer_median_cc);
  printf("New median: %f\n", new_systemizer_median_cc);
  printf("Percentage: %f%%\n", percentage);
  printf("Improvement: %f%%\n", improvement);
  printf("Improvement (intrinsics): %f%%\n", improvement_intrinsics);

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