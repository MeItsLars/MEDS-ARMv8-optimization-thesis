#include <stdlib.h>
#include <stdio.h>
#include <string.h>

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

GFq_t modulo_reduce(uint32_t r)
{
  r = r - MEDS_p * (r >> GFq_bits);
  r = r - MEDS_p * (r >> GFq_bits);
  r = r - MEDS_p * (r >> GFq_bits);
  int32_t diff = r - MEDS_p;
  int32_t mask = (diff >> 31) & 0x1;
  return mask * r + (1 - mask) * diff;
}

void pmod_mat_mul2(pmod_mat_t *C, int C_r, int C_c, pmod_mat_t *A, int A_r, int A_c, pmod_mat_t *B, int B_r, int B_c)
{
  BENCH_START("pmod_mat_mul");
  GFq_t tmp[C_r * C_c];

  for (int c = 0; c < C_c; c++)
    for (int r = 0; r < C_r; r++)
    {
      uint32_t val = 0;

      for (int i = 0; i < A_c; i++)
        val = (val + (uint32_t)pmod_mat_entry(A, A_r, A_c, r, i) * (uint32_t)pmod_mat_entry(B, B_r, B_c, i, c));

      tmp[r * C_c + c] = modulo_reduce(val);
    }

  for (int c = 0; c < C_c; c++)
    for (int r = 0; r < C_r; r++)
      pmod_mat_set_entry(C, C_r, C_c, r, c, tmp[r * C_c + c]);
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
  return pmod_mat_syst_ct_partial_swap_backsub(M, M_r, M_c, M_r, 0, 1);
}

int pmod_mat_syst_ct_partial(pmod_mat_t *M, int M_r, int M_c, int max_r)
{
  return pmod_mat_syst_ct_partial_swap_backsub(M, M_r, M_c, max_r, 0, 1);
}

int pmod_mat_rref(pmod_mat_t *M, int M_r, int M_c)
{
  return pmod_mat_syst_ct_partial_swap_backsub(M, M_r, M_c, M_r, 1, 1);
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
  BENCH_START("pmod_mat_syst_ct");
  int ret = M_r * swap;

  BENCH_START("pmod_mat_syst_ct_forward_elim");
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
  BENCH_END("pmod_mat_syst_ct_forward_elim");

  // At this point, the entire matrix is in RREF.

  // If we don't need to perform back substitution, we're done
  if (!backsub)
    return ret;

  BENCH_START("pmod_mat_syst_ct_backsub");

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

  BENCH_END("pmod_mat_syst_ct_backsub");
  BENCH_END("pmod_mat_syst_ct");
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
