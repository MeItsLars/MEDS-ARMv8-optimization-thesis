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

#define MATMUL_ROUNDS 1000

// Default matrix multiplication implementation
void pmod_mat_mul_1(pmod_mat_t *C, int C_r, int C_c, pmod_mat_t *A, int A_r, int A_c, pmod_mat_t *B, int B_r, int B_c)
{
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
}

// uint32_t montgomery_redc(uint32_t t)
// {
//   // m = ((t mod R) * N') mod R (mod R is implemented as a mask)
//   uint32_t m = ((t & MG_Rmask) * MG_Nprime) & MG_Rmask;
//   // res = (t + m * N) / R (/ R is implemented as a bitshift)
//   uint32_t res = (t + m * MEDS_p) >> MG_Rbits;
//   // if res >= MEDS_p, return res - MEDS_p, else return res (constant time)
//   int32_t diff = res - MEDS_p;
//   int32_t mask = (diff >> 31) & 0x1;
//   return mask * res + (1 - mask) * diff;
//   ;
// }

// uint32_t montgomery_mul(uint32_t a, uint32_t b)
// {
//   return montgomery_redc(a * b);
// }

// uint32_t montgomery_add(uint32_t a, uint32_t b)
// {
//   uint32_t t = a + b;
//   // if t >= MEDS_p, return t - MEDS_p, else return t (constant time)
//   int32_t diff = t - MEDS_p;
//   int32_t mask = (diff >> 31) & 0x1;
//   return mask * t + (1 - mask) * diff;
// }

// uint32_t montgomery_to(GFq_t a)
// {
//   return (((uint32_t)a) << MG_Rbits) % MEDS_p;
// }

// GFq_t montgomery_from(uint32_t a)
// {
//   return (a * MG_Rinv) % MEDS_p;
// }

// // Matrix multiplication with Montgomery reduction
// void pmod_mat_mul_2(pmod_mat_t *C, int C_r, int C_c, pmod_mat_t *A, int A_r, int A_c, pmod_mat_t *B, int B_r, int B_c)
// {
//   uint32_t tmp[C_r * C_c];

//   // Convert to Montgomery domain
//   for (int r = 0; r < A_r; r++)
//     for (int c = 0; c < A_c; c++)
//       pmod_mat_set_entry(A, A_r, A_c, r, c, montgomery_to(pmod_mat_entry(A, A_r, A_c, r, c)));
//   for (int r = 0; r < B_r; r++)
//     for (int c = 0; c < B_c; c++)
//       pmod_mat_set_entry(B, B_r, B_c, r, c, montgomery_to(pmod_mat_entry(B, B_r, B_c, r, c)));

//   // Multiply
//   for (int c = 0; c < C_c; c++)
//     for (int r = 0; r < C_r; r++)
//     {
//       uint32_t val = 0;
//       for (int i = 0; i < A_c; i++)
//       {
//         val = montgomery_add(val, montgomery_mul(pmod_mat_entry(A, A_r, A_c, r, i), pmod_mat_entry(B, B_r, B_c, i, c)));
//       }
//       tmp[r * C_c + c] = val;
//     }

//   // Convert back from Montgomery domain
//   for (int c = 0; c < C_c; c++)
//     for (int r = 0; r < C_r; r++)
//       pmod_mat_set_entry(C, C_r, C_c, r, c, montgomery_from(tmp[r * C_c + c]));
// }

uint32_t modulo_mul(GFq_t a, GFq_t b)
{
  uint32_t res = (uint32_t)a * (uint32_t)b;
  res = res - MEDS_p * (res >> GFq_bits);
  return res;
}

uint32_t mod_reduce(uint32_t r)
{
  // Reduce to a value between 0 and 2^GFq_bits - 1 (constant time)
  r = r - MEDS_p * (r >> GFq_bits);
  r = r - MEDS_p * (r >> GFq_bits);
  r = r - MEDS_p * (r >> GFq_bits);
  // Reduce to a value between 0 and MEDS_p - 1:
  // If r >= MEDS_p, return r - MEDS_p, else return r (constant time)
  int32_t diff = r - MEDS_p;
  int32_t mask = (diff >> 31) & 0x1;
  return mask * r + (1 - mask) * diff;
}

// Matrix multiplication with a smart modulo reduction
__attribute__((optimize("no-tree-vectorize"))) void pmod_mat_mul_3(pmod_mat_t *C, int C_r, int C_c, pmod_mat_t *A, int A_r, int A_c, pmod_mat_t *B, int B_r, int B_c)
{
  uint32_t tmp[C_r * C_c];

  // Multiply
  for (int c = 0; c < C_c; c++)
    for (int r = 0; r < C_r; r++)
    {
      uint32_t val = 0;
      for (int i = 0; i < A_c; i++)
        val += (uint32_t)pmod_mat_entry(A, A_r, A_c, r, i) * (uint32_t)pmod_mat_entry(B, B_r, B_c, i, c);
      tmp[r * C_c + c] = val;
    }

  for (int c = 0; c < C_c; c++)
    for (int r = 0; r < C_r; r++)
      pmod_mat_set_entry(C, C_r, C_c, r, c, mod_reduce(tmp[r * C_c + c]));
}

// Matrix multiplication with a smart modulo reduction and NEON intrinsics in the first multiply loop
void pmod_mat_mul_4(pmod_mat_t *C, int C_r, int C_c, uint16_t *A, int A_r, int A_c, uint16_t *B, int B_r, int B_c)
{
  // Step 1: Prepare matrices. Pad the rows and columns to be a multiple of 4
  // int A_r_pad = (A_r + 3) & ~3;
  // int A_c_pad = (A_c + 3) & ~3;
  // uint32_t A_pad[A_r_pad * A_c_pad];

  // int B_r_pad = (B_r + 3) & ~3;
  // int B_c_pad = (B_c + 3) & ~3;
  // uint32_t B_pad[B_r_pad * B_c_pad];

  // int C_r_pad = A_r_pad;
  // int C_c_pad = B_c_pad;

  // for (int r = 0; r < A_r; r++)
  //   for (int c = 0; c < A_c; c++)
  //     A_pad[r * A_c_pad + c] = pmod_mat_entry(A, A_r, A_c, r, c);

  // for (int r = A_r; r < A_r_pad; r++)
  //   for (int c = 0; c < A_c_pad; c++)
  //     A_pad[r * A_c_pad + c] = 0;

  // for (int r = 0; r < B_r; r++)
  //   for (int c = 0; c < B_c; c++)
  //     B_pad[r * B_c_pad + c] = pmod_mat_entry(B, B_r, B_c, r, c);

  // for (int r = B_r; r < B_r_pad; r++)
  //   for (int c = 0; c < B_c_pad; c++)
  //     B_pad[r * B_c_pad + c] = 0;

  // Step 2: Multiply
  uint32_t tmp[C_r * C_c];

  int Ai, Bi, Ci;
  // uint16x4_t A0, A1, A2, A3;
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

        // A0 = vld1_u16(&A[Ai]);
        // A1 = vld1_u16(&A[Ai + A_c]);
        // A2 = vld1_u16(&A[Ai + 2 * A_c]);
        // A3 = vld1_u16(&A[Ai + 3 * A_c]);

        int A0 = Ai;
        int A1 = Ai + A_c;
        int A2 = Ai + 2 * A_c;
        int A3 = Ai + 3 * A_c;

        B0 = vld1_u16(&B[Bi]);
        B1 = vld1_u16(&B[Bi + B_c]);
        B2 = vld1_u16(&B[Bi + 2 * B_c]);
        B3 = vld1_u16(&B[Bi + 3 * B_c]);

        // Was: vmlaq_laneq_u32. But this is faster!
        // Reordering the following instructions did not seem to improve performance
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

        // TODO: Consider using vmlal_n_u16. Better throughput.
        // C0 = vmlaq_laneq_u32(C0, B0, A0, 0);
        // C0 = vmlaq_laneq_u32(C0, B1, A0, 1);
        // C0 = vmlaq_laneq_u32(C0, B2, A0, 2);
        // C0 = vmlaq_laneq_u32(C0, B3, A0, 3);

        // C1 = vmlaq_laneq_u32(C1, B0, A1, 0);
        // C1 = vmlaq_laneq_u32(C1, B1, A1, 1);
        // C1 = vmlaq_laneq_u32(C1, B2, A1, 2);
        // C1 = vmlaq_laneq_u32(C1, B3, A1, 3);

        // C2 = vmlaq_laneq_u32(C2, B0, A2, 0);
        // C2 = vmlaq_laneq_u32(C2, B1, A2, 1);
        // C2 = vmlaq_laneq_u32(C2, B2, A2, 2);
        // C2 = vmlaq_laneq_u32(C2, B3, A2, 3);

        // C3 = vmlaq_laneq_u32(C3, B0, A3, 0);
        // C3 = vmlaq_laneq_u32(C3, B1, A3, 1);
        // C3 = vmlaq_laneq_u32(C3, B2, A3, 2);
        // C3 = vmlaq_laneq_u32(C3, B3, A3, 3);
      }

      // Store the result block
      Ci = C_c * r + c;
      vst1q_u32(&tmp[Ci], C0);
      vst1q_u32(&tmp[Ci + C_c], C1);
      vst1q_u32(&tmp[Ci + 2 * C_c], C2);
      vst1q_u32(&tmp[Ci + 3 * C_c], C3);

      // Reducing the results immediately here and ignoring the 'tmp' array while storing the result
      // directly into the result matrix should theoretically be faster. But, after trying it, the result
      // turned out to be a bit slower.
    }
  
  // A loop that reduces all elements in 'tmp' using the % operator seems to achieve the same performance
  // as the parallel version below. However, the parallel version allows us to establish a lower bound on
  // the number of cycles required for the reduction.

  // Reduce the result matrix using NEON intrinsics
  uint32x4_t C_red;
  uint16x4_t C_red_u16;
  uint32x4_t C_tmp;
  uint32x4_t C_diff;
  uint32x4_t C_mask;
  uint32x4_t C_MEDS_p = vdupq_n_u32(MEDS_p);
  uint32x4_t C_one = vdupq_n_u32(1);
  for (int r = 0; r < C_r; r++)
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

      // Reduce to a value between 0 and MEDS_p - 1:
      C_diff = vsubq_u32(C_red, C_MEDS_p);
      C_mask = vandq_u32(vshrq_n_u32(C_diff, 31), C_one);
      C_red = vaddq_u32(vmulq_u32(C_mask, C_red), vmulq_u32(vsubq_u32(C_one, C_mask), C_diff));

      // Convert to smaller type
      C_red_u16 = vqmovn_u32(C_red);

      // Store into the result matrix
      int result_index = r * C_c + c;
      // TODO: Handle last few elements separately if C_c * C_r is not a multiple of 4
      assert(result_index < C_c * C_r);
      vst1_u16(&C[result_index], C_red_u16);
    }
}

float min_cycle_bound(int m, int o, int n)
{
  return 0.25 * m * o * (n + 13) + 0.25 * (m * n + n * o + m * o);
}

#define A_ROWS 24
#define A_COLS 24 * 24
#define B_ROWS A_COLS
#define B_COLS 24
#define C_ROWS A_ROWS
#define C_COLS B_COLS

int main(int argc, char *argv[])
{
  enable_cyclecounter();

  uint8_t seed1[MEDS_pub_seed_bytes];

  for (int i = 0; i < MEDS_pub_seed_bytes; i++)
  {
    seed1[i] = i;
  }

  pmod_mat_t A[A_ROWS * A_COLS] __attribute__((aligned(16)));
  pmod_mat_t B[B_ROWS * B_COLS] __attribute__((aligned(16)));
  pmod_mat_t A2[A_ROWS * A_COLS] __attribute__((aligned(16)));
  pmod_mat_t B2[B_ROWS * B_COLS] __attribute__((aligned(16)));
  pmod_mat_t C1[C_ROWS * C_COLS] __attribute__((aligned(16)));
  pmod_mat_t C2[C_ROWS * C_COLS] __attribute__((aligned(16)));

  keccak_state shake;
  shake256_absorb_once(&shake, seed1, MEDS_pub_seed_bytes);

  long long old_matmul_cycles[MATMUL_ROUNDS];
  long long new_matmul_cycles[MATMUL_ROUNDS];

  for (int round = 0; round < MATMUL_ROUNDS; round++)
  {
    // Fill matrices with random values
    for (int r = 0; r < A_ROWS; r++)
      for (int c = 0; c < A_COLS; c++)
      {
        pmod_mat_set_entry(A, A_ROWS, A_COLS, r, c, rnd_GF(&shake));
        A2[r * A_COLS + c] = pmod_mat_entry(A, A_ROWS, A_COLS, r, c);
      }

    for (int r = 0; r < B_ROWS; r++)
      for (int c = 0; c < B_COLS; c++)
      {
        pmod_mat_set_entry(B, B_ROWS, B_COLS, r, c, rnd_GF(&shake));
        B2[r * B_COLS + c] = pmod_mat_entry(B, B_ROWS, B_COLS, r, c);
      }

    long long old_matmul_cc = -get_cyclecounter();
    pmod_mat_mul_1(C1, C_ROWS, C_COLS, A, A_ROWS, A_COLS, B, B_ROWS, B_COLS);
    old_matmul_cc += get_cyclecounter();

    long long new_matmul_cc = -get_cyclecounter();
    pmod_mat_mul_4(C2, C_ROWS, C_COLS, A2, A_ROWS, A_COLS, B2, B_ROWS, B_COLS);
    new_matmul_cc += get_cyclecounter();

    old_matmul_cycles[round] = old_matmul_cc;
    new_matmul_cycles[round] = new_matmul_cc;
  }

  // Print results
  double old_matmul_median_cc = median(old_matmul_cycles, MATMUL_ROUNDS);
  double new_matmul_median_cc = median(new_matmul_cycles, MATMUL_ROUNDS);
  float cycle_bound = min_cycle_bound(MEDS_k, MEDS_m * MEDS_n, MEDS_k);
  float improvement_possible = (new_matmul_median_cc / cycle_bound) * 100;
  float old_matmul_std = standard_deviation(old_matmul_cycles, MATMUL_ROUNDS);
  float new_matmul_std = standard_deviation(new_matmul_cycles, MATMUL_ROUNDS);
  double percentage = new_matmul_median_cc / old_matmul_median_cc * 100;
  double improvement = (new_matmul_median_cc - old_matmul_median_cc) / old_matmul_median_cc * 100;
  printf("Minimum cycle bound: %f\n", cycle_bound);
  printf("Improvement possible: -%f%% (x%f%%)\n", (improvement_possible - 100), improvement_possible / 100);
  printf("Old median: %f\n", old_matmul_median_cc);
  printf("New median: %f\n", new_matmul_median_cc);
  printf("Old std: %f\n", old_matmul_std);
  printf("New std: %f\n", new_matmul_std);
  printf("Percentage: %f%%\n", percentage);
  printf("Improvement: %f%%\n", improvement);

  disable_cyclecounter();

  // Compare matrices
  int exact_equalities = 0;
  int modulo_equalities = 0;
  for (int i = 0; i < MEDS_k; i++)
    for (int j = 0; j < MEDS_k; j++)
    {
      GFq_t val1 = pmod_mat_entry(C1, MEDS_k, MEDS_k, i, j);
      GFq_t val2 = pmod_mat_entry(C2, MEDS_k, MEDS_k, i, j);
      if (val1 == val2)
        exact_equalities++;
      val1 = val1 > MEDS_p ? val1 - MEDS_p : val1;
      val2 = val2 > MEDS_p ? val2 - MEDS_p : val2;
      if (val1 == val2)
        modulo_equalities++;
    }

  int expected_equalities = MEDS_k * MEDS_k;

  printf("Equalities: %d / %d\n", exact_equalities, expected_equalities);
  printf("Modulo equalities: %d / %d\n", modulo_equalities, expected_equalities);
  if (expected_equalities == exact_equalities)
  {
    printf("Matrices are EQUAL\n");
  }
  else if (modulo_equalities == expected_equalities)
  {
    printf("Matrices are EQUAL modulo MEDS_p\n");
  }
  else
  {
    printf("Matrices are NOT EQUAL\n");
    exit(-1);
  }

  return 0;
}