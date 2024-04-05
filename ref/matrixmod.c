#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <arm_neon.h>
#include <assert.h>

#include "log.h"
#include "benchresult.h"

#include "params.h"
#include "matrixmod.h"

void pmod_mat_print(pmod_mat_t *M, int M_r, int M_c)
{
  pmod_mat_fprint(stdout, M, M_r, M_c);
}

void pmod_mat_fprint(FILE *stream, pmod_mat_t *M, int M_r, int M_c)
{
  GFq_t max = 0;

  for (int i = 0; i < M_r * M_c; i++)
    if (M[i] > max)
      max = M[i];

  char tmp[32];

  snprintf(tmp, sizeof(tmp), "%u", max);

  int len = strlen(tmp);

  char fmt0[32] = {"%"};
  char fmt1[32] = {"%"};

  snprintf(fmt0 + 1, 31, "%iu ", len);
  snprintf(fmt1 + 1, 31, "%iu", len);

  for (int r = 0; r < M_r; r++)
  {
    fprintf(stream, "[");
    for (int c = 0; c < M_c - 1; c++)
      fprintf(stream, fmt0, pmod_mat_entry(M, M_r, M_c, r, c));
    fprintf(stream, fmt1, pmod_mat_entry(M, M_r, M_c, r, M_c - 1));
    fprintf(stream, "]\n");
  }
}

void print_binary_64(uint64_t val)
{
  for (int i = 63; i >= 0; i--)
  {
    if (val >> i & 1)
      printf("1");
    else
      printf("0");

    if (i % 8 == 0)
      printf(" ");
  }
  printf("\n");
}

void print_binary(GFq_t val)
{
  for (int i = GFq_bits - 1; i >= 0; i--)
  {
    if (val & (1 << i))
      printf("1");
    else
      printf("0");

    if (i % 8 == 0)
      printf(" ");
  }
  printf("\n");
}

void pmod_mat_mul_new(
    pmod_mat_t *C_up, int C_r_up, int C_c_up,
    pmod_mat_t *A_up, int A_r_up, int A_c_up,
    pmod_mat_t *B_up, int B_r_up, int B_c_up)
{
  BENCH_START("pmod_mat_mul");
  // Step 1: Prepare matrices. Pad the rows and columns to be a multiple of 4
  int A_r = (A_r_up + 3) & ~3;
  int A_c = (A_c_up + 3) & ~3;
  uint16_t A[A_r * A_c];

  int B_r = (B_r_up + 3) & ~3;
  int B_c = (B_c_up + 3) & ~3;
  uint16_t B[B_r * B_c];

  int C_r = A_r;
  int C_c = B_c;

  for (int r = 0; r < A_r_up; r++)
    for (int c = 0; c < A_c_up; c++)
      A[r * A_c + c] = pmod_mat_entry(A_up, A_r_up, A_c_up, r, c);

  for (int r = A_r_up; r < A_r; r++)
    for (int c = 0; c < A_c; c++)
      A[r * A_c + c] = 0;

  for (int r = 0; r < B_r_up; r++)
    for (int c = 0; c < B_c_up; c++)
      B[r * B_c + c] = pmod_mat_entry(B_up, B_r_up, B_c_up, r, c);

  for (int r = B_r_up; r < B_r; r++)
    for (int c = 0; c < B_c; c++)
      B[r * B_c + c] = 0;

  // Step 2: Multiply
  uint32_t tmp[C_r * C_c];

  int Ai, Bi, Ci;
  uint16x4_t B0, B1, B2, B3;
  uint32x4_t C0, C1, C2, C3;

  for (int c = 0; c < C_c; c += 4)
    for (int r = 0; r < C_r; r += 4)
    {
      // In every inner loop, we compute a 4x4 block of the result matrix
      // This 4x4 block is represented by the vectors (of size 4) C0, C1, C2, C3

      C0 = vmovq_n_u32(0);
      C1 = vmovq_n_u32(0);
      C2 = vmovq_n_u32(0);
      C3 = vmovq_n_u32(0);

      // In every iteration of the following loop, we compute a contribution to the 4x4 result block
      // Specifically, we compute the product of a 4x4 block of A and a 4x4 block of B and add it to the result block
      for (int k = 0; k < A_c; k += 4)
      {
        Ai = r * A_c + k;
        Bi = k * B_c + c;

        int A0 = Ai;
        int A1 = Ai + A_c;
        int A2 = Ai + 2 * A_c;
        int A3 = Ai + 3 * A_c;

        B0 = vld1_u16(&B[Bi]);
        B1 = vld1_u16(&B[Bi + B_c]);
        B2 = vld1_u16(&B[Bi + 2 * B_c]);
        B3 = vld1_u16(&B[Bi + 3 * B_c]);

        C0 = vmlal_n_u16(C0, B0, A[A0 + 0]);
        C0 = vmlal_n_u16(C0, B1, A[A0 + 1]);
        C0 = vmlal_n_u16(C0, B2, A[A0 + 2]);
        C0 = vmlal_n_u16(C0, B3, A[A0 + 3]);

        C1 = vmlal_n_u16(C1, B0, A[A1 + 0]);
        C1 = vmlal_n_u16(C1, B1, A[A1 + 1]);
        C1 = vmlal_n_u16(C1, B2, A[A1 + 2]);
        C1 = vmlal_n_u16(C1, B3, A[A1 + 3]);

        C2 = vmlal_n_u16(C2, B0, A[A2 + 0]);
        C2 = vmlal_n_u16(C2, B1, A[A2 + 1]);
        C2 = vmlal_n_u16(C2, B2, A[A2 + 2]);
        C2 = vmlal_n_u16(C2, B3, A[A2 + 3]);

        C3 = vmlal_n_u16(C3, B0, A[A3 + 0]);
        C3 = vmlal_n_u16(C3, B1, A[A3 + 1]);
        C3 = vmlal_n_u16(C3, B2, A[A3 + 2]);
        C3 = vmlal_n_u16(C3, B3, A[A3 + 3]);
      }

      // Store the result block
      Ci = C_c * r + c;
      vst1q_u32(&tmp[Ci], C0);
      vst1q_u32(&tmp[Ci + C_c], C1);
      vst1q_u32(&tmp[Ci + 2 * C_c], C2);
      vst1q_u32(&tmp[Ci + 3 * C_c], C3);
    }

  // Reduce the result matrix using NEON intrinsics
  uint32x4_t C_red;
  uint16x4_t C_red_u16;
  uint32x4_t C_tmp;
  uint32x4_t C_diff;
  uint32x4_t C_mask;
  uint32x4_t C_MEDS_p = vdupq_n_u32(MEDS_p);
  uint32x4_t C_one = vdupq_n_u32(1);
  for (int r = 0; r < C_r_up; r++)
    for (int c = 0; c < C_c; c += 4)
    {
      // Load 4 values from the result matrix
      C_red = vld1q_u32(&tmp[r * C_c + c]);

      // Apply two reductions
      C_tmp = vshrq_n_u32(C_red, GFq_bits);
      C_tmp = vmulq_n_u32(C_tmp, MEDS_p);
      C_red = vsubq_u32(C_red, C_tmp);
      C_tmp = vshrq_n_u32(C_red, GFq_bits);
      C_tmp = vmulq_n_u32(C_tmp, MEDS_p);
      C_red = vsubq_u32(C_red, C_tmp);

      // Reduce to a value between 0 and MEDS_p - 1: (can be ignored?)
      // C_diff = vsubq_u32(C_red, C_MEDS_p);
      // C_mask = vandq_u32(vshrq_n_u32(C_diff, 31), C_one);
      // C_red = vaddq_u32(vmulq_u32(C_mask, C_red), vmulq_u32(vsubq_u32(C_one, C_mask), C_diff));

      // Convert to smaller type
      C_red_u16 = vqmovn_u32(C_red);

      // Store into the result matrix
      int result_index = r * C_c_up + c;
      // Handle last few elements separately if C_c * C_r is not a multiple of 4
      if (c + 4 <= C_c_up)
        vst1_u16(&C_up[result_index], C_red_u16);
      else
      {
        if (c + 0 < C_c_up)
          pmod_mat_set_entry(C_up, C_r_up, C_c_up, r, c + 0, vget_lane_u16(C_red_u16, 0));
        if (c + 1 < C_c_up)
          pmod_mat_set_entry(C_up, C_r_up, C_c_up, r, c + 1, vget_lane_u16(C_red_u16, 1));
        if (c + 2 < C_c_up)
          pmod_mat_set_entry(C_up, C_r_up, C_c_up, r, c + 2, vget_lane_u16(C_red_u16, 2));
      }
    }

  BENCH_END("pmod_mat_mul");
}

void pmod_mat_mul(pmod_mat_t *C, int C_r, int C_c, pmod_mat_t *A, int A_r, int A_c, pmod_mat_t *B, int B_r, int B_c)
{
  BENCH_START("pmod_mat_mul");
  GFq_t tmp[C_r * C_c];

  for (int c = 0; c < C_c; c++)
    for (int r = 0; r < C_r; r++)
    {
      uint64_t val = 0;

      for (int i = 0; i < A_c; i++)
        val = (val + (uint64_t)pmod_mat_entry(A, A_r, A_c, r, i) * (uint64_t)pmod_mat_entry(B, B_r, B_c, i, c));

      tmp[r * C_c + c] = val % MEDS_p;
    }

  for (int c = 0; c < C_c; c++)
    for (int r = 0; r < C_r; r++)
      pmod_mat_set_entry(C, C_r, C_c, r, c, tmp[r * C_c + c]);
  BENCH_END("pmod_mat_mul");
}

int pmod_mat_syst_ct(pmod_mat_t *M, int M_r, int M_c)
{
  BENCH_START("pmod_mat_syst_ct");
  int result = pmod_mat_syst_ct_partial_swap_backsub(M, M_r, M_c, M_r, 0, 1);
  BENCH_END("pmod_mat_syst_ct");
  return result;
}

int pmod_mat_syst_ct_partial(pmod_mat_t *M, int M_r, int M_c, int max_r)
{
  BENCH_START("pmod_mat_syst_ct_partial");
  int result = pmod_mat_syst_ct_partial_swap_backsub(M, M_r, M_c, max_r, 0, 1);
  BENCH_END("pmod_mat_syst_ct_partial");
  return result;
}

int pmod_mat_rref(pmod_mat_t *M, int M_r, int M_c)
{
  BENCH_START("pmod_mat_rref");
  int result = pmod_mat_syst_ct_partial_swap_backsub(M, M_r, M_c, M_r, 1, 1);
  BENCH_END("pmod_mat_rref");
  return result;
}

/**
 * @brief Solve a system of linear equations using the Gaussian Elimination Algorithm with pivoting:
 * https://web.mit.edu/10.001/Web/Course_Notes/GaussElimPivoting.html
 *
 * @param M The matrix of the system of linear equations.
 * @param M_r The number of rows of the matrix.
 * @param M_c The number of columns of the matrix.
 * @param max_r The maximum number of rows to consider.
 * @param swap If set to 1, the algorithm will perform swaps.
 * @param backsub If set to 1, the algorithm will perform back substitution.
 * @return -1 if the system of linear equations is inconsistent.
 *         0 if we were able to solve the system of linear equations and swap was not set.
 *         The row number of the last swap if we were able to solve the system of linear equations and swap was set.
 */
int pmod_mat_syst_ct_partial_swap_backsub(pmod_mat_t *M, int M_r, int M_c, int max_r, int swap, int backsub)
{
  BENCH_START("pmod_mat_syst_ct_partial_swap_backsub");
  int ret = M_r * swap;

  BENCH_START("[1] pmod_mat_syst_ct_forward_elim");
  // Step 1: Forward elimination
  for (int r = 0; r < max_r; r++)
  {
    // Execute swap (?)
    // I'm really not sure what this is swapping and why
    // Swapping is only used by the 'solve_opt' function.
    if (swap)
    {
      GFq_t z = 0;

      // compute condition for swap
      // After this loop, z will be 1 if there is a non-zero element in the current column (on or after the diagonal)
      for (int r2 = r; r2 < M_r; r2++)
        z |= pmod_mat_entry(M, M_r, M_c, r2, r);

      // We need to swap if the pivot element is zero
      int do_swap = GFq_eq0(z);

      // conditional swap
      {
        // If a swap is needed, set 'ret' to the row number
        ret = r * do_swap + ret * (1 - do_swap);

#if DEBUG
        if (do_swap)
        {
          LOG("swapping %i", r);

          LOG_MAT(M, M_r, M_c);
        }
#endif

        // Seems to swap column r with the last column, but why?
        for (int i = 0; i < M_r; i++)
          GFq_cswap(&pmod_mat_entry(M, M_r, M_c, i, r),
                    &pmod_mat_entry(M, M_r, M_c, i, M_c - 1),
                    do_swap);

#if DEBUG
        if (do_swap)
          LOG_MAT(M, M_r, M_c);
#endif
      }
    }

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
    {
      BENCH_END("pmod_mat_syst_ct_partial_swap_backsub");
      return -1;
    }

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
  BENCH_END("[1] pmod_mat_syst_ct_forward_elim");

  // At this point, the entire matrix is in RREF.

  // If we don't need to perform back substitution, we're done
  if (!backsub)
  {
    return ret;
    BENCH_END("pmod_mat_syst_ct_partial_swap_backsub");
  }

  BENCH_START("[2] pmod_mat_syst_ct_backsub");

  // back substitution;
  // In this loop, we make sure the elements above the diagonal are zero
  // We do this by subtracting the current row from the other rows with the appropriate difference factor
  // The goal is to prepare the previous rows for the next iteration (making sure they have a zero in the columns right of the diagonal)

  // Loop through the rows in reverse order (so we start with the last row which contains only one non-zero element)
  for (int r = max_r - 1; r >= 0; r--)
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
      for (int c = max_r; c < M_c; c++)
      {
        uint64_t tmp0 = pmod_mat_entry(M, M_r, M_c, r, c);
        uint64_t tmp1 = pmod_mat_entry(M, M_r, M_c, r2, c);

        uint64_t val = (tmp0 * factor) % MEDS_p;

        val = (MEDS_p + tmp1 - val) % MEDS_p;

        pmod_mat_set_entry(M, M_r, M_c, r2, c, val);
      }
    }

  BENCH_END("[2] pmod_mat_syst_ct_backsub");
  BENCH_END("pmod_mat_syst_ct_partial_swap_backsub");
  return ret;
}

GFq_t GF_inv(GFq_t val)
{
  BENCH_START("GF_inv");
  //  if (MEDS_p == 8191)
  //  {
  //    // Use an optimal addition chain...
  //    uint64_t tmp_0  = val;                          //  0                1
  //    uint64_t tmp_1  = (tmp_0 * tmp_0) % MEDS_p;     //  1( 0)            2
  //    uint64_t tmp_2  = (tmp_1 * tmp_0) % MEDS_p;     //  2( 1, 0)         3
  //    uint64_t tmp_3  = (tmp_2 * tmp_1) % MEDS_p;     //  3( 2, 1)         5
  //    uint64_t tmp_4  = (tmp_3 * tmp_3) % MEDS_p;     //  4( 3)           10
  //    uint64_t tmp_5  = (tmp_4 * tmp_3) % MEDS_p;     //  5( 4, 3)        15
  //    uint64_t tmp_6  = (tmp_5 * tmp_5) % MEDS_p;     //  6( 5)           30
  //    uint64_t tmp_7  = (tmp_6 * tmp_6) % MEDS_p;     //  7( 6)           60
  //    uint64_t tmp_8  = (tmp_7 * tmp_7) % MEDS_p;     //  8( 7)          120
  //    uint64_t tmp_9  = (tmp_8 * tmp_8) % MEDS_p;     //  9( 8)          240
  //    uint64_t tmp_10 = (tmp_9 * tmp_5) % MEDS_p;     // 10( 9, 5)       255
  //    uint64_t tmp_11 = (tmp_10 * tmp_10) % MEDS_p;   // 11(10)          510
  //    uint64_t tmp_12 = (tmp_11 * tmp_11) % MEDS_p;   // 12(11)         1020
  //    uint64_t tmp_13 = (tmp_12 * tmp_2) % MEDS_p;    // 13(12, 2)      1023
  //    uint64_t tmp_14 = (tmp_13 * tmp_13) % MEDS_p;   // 14(13)         2046
  //    uint64_t tmp_15 = (tmp_14 * tmp_14) % MEDS_p;   // 15(14)         4092
  //    uint64_t tmp_16 = (tmp_15 * tmp_15) % MEDS_p;   // 16(15)         8184
  //    uint64_t tmp_17 = (tmp_16 * tmp_3) % MEDS_p;    // 17(16, 3)      8189
  //
  //    return tmp_17;
  //  }
  //  else if (MEDS_p == 4093)
  //  {
  //    // Use an optimal addition chain...
  //   uint64_t tmp_0  = val;                          //  0                1
  //   uint64_t tmp_1  = (tmp_0 * tmp_0) % MEDS_p;     //  1( 0)            2
  //   uint64_t tmp_2  = (tmp_1 * tmp_1) % MEDS_p;     //  2( 1)            4
  //   uint64_t tmp_3  = (tmp_2 * tmp_0) % MEDS_p;     //  3( 2, 0)         5
  //   uint64_t tmp_4  = (tmp_3 * tmp_3) % MEDS_p;     //  4( 3)           10
  //   uint64_t tmp_5  = (tmp_4 * tmp_3) % MEDS_p;     //  5( 4, 3)        15
  //   uint64_t tmp_6  = (tmp_5 * tmp_5) % MEDS_p;     //  6( 5)           30
  //   uint64_t tmp_7  = (tmp_6 * tmp_6) % MEDS_p;     //  7( 6)           60
  //   uint64_t tmp_8  = (tmp_7 * tmp_7) % MEDS_p;     //  8( 7)          120
  //   uint64_t tmp_9  = (tmp_8 * tmp_8) % MEDS_p;     //  9( 8)          240
  //   uint64_t tmp_10 = (tmp_9 * tmp_5) % MEDS_p;     // 10( 9, 5)       255
  //   uint64_t tmp_11 = (tmp_10 * tmp_10) % MEDS_p;   // 11(10)          510
  //   uint64_t tmp_12 = (tmp_11 * tmp_11) % MEDS_p;   // 12(11)         1020
  //   uint64_t tmp_13 = (tmp_12 * tmp_12) % MEDS_p;   // 13(12)         2040
  //   uint64_t tmp_14 = (tmp_13 * tmp_3) % MEDS_p;    // 14(13, 3)      2045
  //   uint64_t tmp_15 = (tmp_14 * tmp_14) % MEDS_p;   // 15(14)         4090
  //   uint64_t tmp_16 = (tmp_15 * tmp_0) % MEDS_p;    // 16(15, 0)      4091
  //
  //   return tmp_16;
  //  }
  //  else
  {
    uint64_t exponent = MEDS_p - 2;
    uint64_t t = 1;

    while (exponent > 0)
    {
      if ((exponent & 1) != 0)
        t = (t * (uint64_t)val) % MEDS_p;

      val = ((uint64_t)val * (uint64_t)val) % MEDS_p;

      exponent >>= 1;
    }

    BENCH_END("GF_inv");
    return t;
  }
}

int pmod_mat_inv(pmod_mat_t *B, pmod_mat_t *A, int A_r, int A_c)
{
  pmod_mat_t M[A_r * A_c * 2];

  for (int r = 0; r < A_r; r++)
  {
    memcpy(&M[r * A_c * 2], &A[r * A_c], A_c * sizeof(GFq_t));

    for (int c = 0; c < A_c; c++)
      pmod_mat_set_entry(M, A_r, A_c * 2, r, A_c + c, r == c ? 1 : 0);
  }

  int ret = pmod_mat_syst_ct(M, A_r, A_c * 2);

  if ((ret == 0) && B)
    for (int r = 0; r < A_r; r++)
      memcpy(&B[r * A_c], &M[r * A_c * 2 + A_c], A_c * sizeof(GFq_t));

  return ret;
}
