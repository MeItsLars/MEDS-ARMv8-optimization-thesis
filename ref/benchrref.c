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
#include "benchresult.h"

benchresult benchresults[1000];
int number_of_benchresults = 0;
int benchmark_enabled = 0;

#define rref_ROUNDS 1000

int test_pmod_mat_syst_ct(pmod_mat_t *M, int M_r, int M_c)
{
  int ret = 0;

  // Step 1: Forward elimination
  for (int r = 0; r < M_r; r++)
  {
    // Step 2: Elimination
    // In this loop, we eliminate the elements below the diagonal
    // We do this by multiplying the pivot row by the inverse of the pivot element (the element on the diagonal)
    // and then subtracting the result from the other rows

    // Loop from the next row to the last row
    for (int r2 = r + 1; r2 < M_r; r2++)
    {
      // Get the element on the diagonal
      uint64_t Mrr = pmod_mat_entry(M, M_r, M_c, r, r);

      // If the element on the diagonal is zero, add the entire row r2 to the current row r
      // This is done to make the element on the diagonal non-zero
      for (int c = r; c < M_c; c++)
      {
        uint64_t val = pmod_mat_entry(M, M_r, M_c, r2, c);
        uint64_t Mrc = pmod_mat_entry(M, M_r, M_c, r, c);
        pmod_mat_set_entry(M, M_r, M_c, r, c, (Mrc + val * GFq_eq0(Mrr)) % MEDS_p);
      }
    }

    uint64_t val = pmod_mat_entry(M, M_r, M_c, r, r);

    // If, after everything, the element on the diagonal is (still) zero, return that we failed
    // to convert this row in the matrix to RREF
    if (val == 0)
      return -1;

    // Compute the multiplicative inverse of the element on the diagonal
    val = GF_inv(val);

    // normalize;
    // multiply every element in the row with the multiplicative inverse of the element on the diagonal
    // The goal of this is to make sure the element on the diagonal is exactly 1.
    for (int c = r; c < M_c; c++)
    {
      uint64_t tmp = ((uint64_t)pmod_mat_entry(M, M_r, M_c, r, c) * val) % MEDS_p;
      pmod_mat_set_entry(M, M_r, M_c, r, c, tmp);
    }

    // eliminate;
    // For all rows below the current one, make sure the element in the current column is zero
    // This is done by subtracting the current row from the other rows with the appropriate difference factor
    // The goal is to prepare the next rows for the next iteration (making sure they have a zero in the columns left of the diagonal)
    for (int r2 = r + 1; r2 < M_r; r2++)
    {
      // Calculate the factor
      uint64_t factor = pmod_mat_entry(M, M_r, M_c, r2, r);

      for (int c = r; c < M_c; c++)
      {
        // Calculate and set the new value
        uint64_t tmp0 = pmod_mat_entry(M, M_r, M_c, r, c);
        uint64_t tmp1 = pmod_mat_entry(M, M_r, M_c, r2, c);

        uint64_t val = (tmp0 * factor) % MEDS_p;

        val = (MEDS_p + tmp1 - val) % MEDS_p;

        pmod_mat_set_entry(M, M_r, M_c, r2, c, val);
      }
    }
  }

  // At this point, the entire matrix is in RREF.

  // back substitution;
  // In this loop, we make sure the elements above the diagonal are zero
  // We do this by subtracting the current row from the other rows with the appropriate difference factor
  // The goal is to prepare the previous rows for the next iteration (making sure they have a zero in the columns right of the diagonal)

  // Loop through the rows in reverse order (so we start with the last row which contains only one non-zero element)
  for (int r = M_r - 1; r >= 0; r--)
    // For every row above the current one, make sure the element in the current column is zero
    for (int r2 = 0; r2 < r; r2++)
    {
      // Compute the factor
      uint64_t factor = pmod_mat_entry(M, M_r, M_c, r2, r);

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
      for (int c = M_r; c < M_c; c++)
      {
        uint64_t tmp0 = pmod_mat_entry(M, M_r, M_c, r, c);
        uint64_t tmp1 = pmod_mat_entry(M, M_r, M_c, r2, c);

        uint64_t val = (tmp0 * factor) % MEDS_p;

        val = (MEDS_p + tmp1 - val) % MEDS_p;

        pmod_mat_set_entry(M, M_r, M_c, r2, c, val);
      }
    }
  return ret;
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

int test_pmod_mat_syst_ct_2(pmod_mat_t *M, int M_r, int M_c)
{
  int ret = 0;

  uint16x8_t MEDS_p_16x8 = vdupq_n_u16(MEDS_p);
  uint16x4_t MEDS_p_16x4 = vdup_n_u16(MEDS_p);
  uint32x4_t MEDS_p_32x4 = vdupq_n_u32(MEDS_p);
  uint32x4_t one_32x4 = vdupq_n_u32(1);

  // Step 1: Forward elimination
  for (int r = 0; r < M_r; r++)
  {
    // Step 2: Elimination
    // In this loop, we eliminate the elements below the diagonal
    // We do this by multiplying the pivot row by the inverse of the pivot element (the element on the diagonal)
    // and then subtracting the result from the other rows

    // Loop from the next row to the last row
    for (int r2 = r + 1; r2 < M_r; r2++)
    {
      // Get the element on the diagonal
      uint64_t Mrr = pmod_mat_entry(M, M_r, M_c, r, r);
      uint16x8_t swap = vdupq_n_u16(GFq_eq0(Mrr));

      // If the element on the diagonal is zero, add the entire row r2 to the current row r
      // This is done to make the element on the diagonal non-zero
      for (int c = r; c < M_c; c += 8)
      {
        // uint64_t val = pmod_mat_entry(M, M_r, M_c, r2, c);
        // uint64_t Mrc = pmod_mat_entry(M, M_r, M_c, r, c);
        // pmod_mat_set_entry(M, M_r, M_c, r, c, (Mrc + val * GFq_eq0(Mrr)) % MEDS_p);

        // Compute result
        uint16x8_t val = vld1q_u16(&M[(M_c)*r2 + c]);
        uint16x8_t Mrc = vld1q_u16(&M[(M_c)*r + c]);
        uint16x8_t result = vaddq_u16(Mrc, vmulq_u16(val, swap));

        // TODO: This would be necessary, but we will be reducing results later anyway
        // Subtract MEDS_p if result is larger than MEDS_p
        // uint16x8_t mask = vcgtq_u16(result, MEDS_p_16x8);
        // result = vsubq_u16(result, vandq_u16(MEDS_p_16x8, mask));

        // Store result
        vst1q_u16(&M[(M_c)*r + c], result);
      }
    }

    uint64_t val = pmod_mat_entry(M, M_r, M_c, r, r);

    // If, after everything, the element on the diagonal is (still) zero, return that we failed
    // to convert this row in the matrix to RREF
    if (val == 0)
      return -1;

    // Compute the multiplicative inverse of the element on the diagonal
    val = GF_inv(val);
    uint16x4_t val_v = vdup_n_u16(val);

    // normalize;
    // multiply every element in the row with the multiplicative inverse of the element on the diagonal
    // The goal of this is to make sure the element on the diagonal is exactly 1.
    for (int c = r; c < M_c; c += 4)
    {
      // uint64_t tmp = ((uint64_t)pmod_mat_entry(M, M_r, M_c, r, c) * val) % MEDS_p;
      // pmod_mat_set_entry(M, M_r, M_c, r, c, tmp);

      // Compute result
      uint16x4_t vals = vld1_u16(&M[(M_c)*r + c]);
      uint32x4_t red = vmull_u16(vals, val_v);
      uint16x4_t red_16x4 = reduce(red, MEDS_p_16x4, MEDS_p_32x4);

      // Store result
      vst1_u16(&M[(M_c)*r + c], red_16x4);
    }

    // eliminate;
    // For all rows below the current one, make sure the element in the current column is zero
    // This is done by subtracting the current row from the other rows with the appropriate difference factor
    // The goal is to prepare the next rows for the next iteration (making sure they have a zero in the columns left of the diagonal)
    for (int r2 = r + 1; r2 < M_r; r2++)
    {
      // Calculate the factor
      GFq_t factor = pmod_mat_entry(M, M_r, M_c, r2, r);
      uint16x4_t factor_16x4 = vdup_n_u16(factor);

      for (int c = r; c < M_c; c += 4)
      {
        // Calculate and set the new value
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
    }
  }

  // At this point, the entire matrix is in RREF.

  // back substitution;
  // In this loop, we make sure the elements above the diagonal are zero
  // We do this by subtracting the current row from the other rows with the appropriate difference factor
  // The goal is to prepare the previous rows for the next iteration (making sure they have a zero in the columns right of the diagonal)

  // Loop through the rows in reverse order (so we start with the last row which contains only one non-zero element)
  for (int r = M_r - 1; r >= 0; r--)
    // For every row above the current one, make sure the element in the current column is zero
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
    }
  return ret;
}

#define A_ROWS 24
#define A_COLS 24

int main(int argc, char *argv[])
{
  enable_cyclecounter();

  uint8_t seed1[MEDS_pub_seed_bytes];

  for (int i = 0; i < MEDS_pub_seed_bytes; i++)
  {
    seed1[i] = i;
  }

  pmod_mat_t A1[A_ROWS * A_COLS];
  pmod_mat_t A2[A_ROWS * A_COLS];

  keccak_state shake;
  shake256_absorb_once(&shake, seed1, MEDS_pub_seed_bytes);

  long long old_rref_cycles[rref_ROUNDS];
  long long new_rref_cycles[rref_ROUNDS];

  for (int round = 0; round < rref_ROUNDS; round++)
  {
    // Fill matrices with random values
    for (int r = 0; r < A_ROWS; r++)
      for (int c = 0; c < A_COLS; c++)
      {
        GFq_t rand_el = rnd_GF(&shake);
        pmod_mat_set_entry(A1, A_ROWS, A_COLS, r, c, rand_el);
        pmod_mat_set_entry(A2, A_ROWS, A_COLS, r, c, rand_el);
      }

    // Run test A1
    long long old_rref_cc = -get_cyclecounter();
    test_pmod_mat_syst_ct(A1, A_ROWS, A_COLS);
    old_rref_cc += get_cyclecounter();

    // Run test A2
    long long new_rref_cc = -get_cyclecounter();
    test_pmod_mat_syst_ct_2(A2, A_ROWS, A_COLS);
    new_rref_cc += get_cyclecounter();

    old_rref_cycles[round] = old_rref_cc;
    new_rref_cycles[round] = new_rref_cc;
  }

  // Print results
  double old_rref_median_cc = median(old_rref_cycles, rref_ROUNDS);
  double new_rref_median_cc = median(new_rref_cycles, rref_ROUNDS);
  // float old_rref_std = standard_deviation(old_rref_cycles, rref_ROUNDS);
  // float new_rref_std = standard_deviation(new_rref_cycles, rref_ROUNDS);
  double percentage = new_rref_median_cc / old_rref_median_cc * 100;
  double improvement = (new_rref_median_cc - old_rref_median_cc) / old_rref_median_cc * 100;
  printf("Old median: %f\n", old_rref_median_cc);
  printf("New median: %f\n", new_rref_median_cc);
  // printf("Old std: %f\n", old_rref_std);
  // printf("New std: %f\n", new_rref_std);
  printf("Percentage: %f%%\n", percentage);
  printf("Improvement: %f%%\n", improvement);

  disable_cyclecounter();

  // Compare matrices
  int equalities = 0;
  for (int r = 0; r < A_ROWS; r++)
    for (int c = 0; c < A_COLS; c++)
      if (pmod_mat_entry(A1, A_ROWS, A_COLS, r, c) == pmod_mat_entry(A2, A_ROWS, A_COLS, r, c))
      {
        equalities++;
      }

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