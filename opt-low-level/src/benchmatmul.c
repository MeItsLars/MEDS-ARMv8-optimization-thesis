#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <math.h>

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

#define MATMUL_ROUNDS 500
#define ROUNDS 128

extern void nop_test();

extern void pmod_mat_mul_asm(uint16_t *C, uint16_t *A, uint16_t *B, int m, int o, int n);

extern void pmod_mat_mul_asm_1_4_4(uint16_t *C, uint16_t *A, uint16_t *B);
extern void pmod_mat_mul_asm_4_4_4(uint16_t *C, uint16_t *A, uint16_t *B);
extern void pmod_mat_mul_8_asm_8_8(uint16_t *C, uint16_t *A, uint16_t *B);
extern void pmod_mat_mul_asm_4_4_5(uint16_t *C, uint16_t *A, uint16_t *B);
extern void pmod_mat_mul_asm_4_5_4(uint16_t *C, uint16_t *A, uint16_t *B);
extern void pmod_mat_mul_asm_4_4_67(uint16_t *C, uint16_t *A, uint16_t *B);
extern void pmod_mat_mul_asm_24_4_67(uint16_t *C, uint16_t *A, uint16_t *B);
extern void pmod_mat_mul_asm_5_4_4(uint16_t *C, uint16_t *A, uint16_t *B);
extern void pmod_mat_mul_asm_6_4_4(uint16_t *C, uint16_t *A, uint16_t *B);
extern void pmod_mat_mul_asm_7_4_4(uint16_t *C, uint16_t *A, uint16_t *B);
extern void pmod_mat_mul_asm_13_8_8(uint16_t *C, uint16_t *A, uint16_t *B);
extern void pmod_mat_mul_8_asm_13_8(uint16_t *C, uint16_t *A, uint16_t *B);
extern void pmod_mat_mul_8_asm_8_13(uint16_t *C, uint16_t *A, uint16_t *B);
extern void pmod_mat_mul_asm_13_13_13(uint16_t *C, uint16_t *A, uint16_t *B);
extern void pmod_mat_mul_asm_16_16_16(uint16_t *C, uint16_t *A, uint16_t *B);
extern void pmod_mat_mul_asm_24_24_24(uint16_t *C, uint16_t *A, uint16_t *B);
extern void pmod_mat_mul_asm_30_30_30(uint16_t *C, uint16_t *A, uint16_t *B);
extern void pmod_mat_mul_asm_40_40_40(uint16_t *C, uint16_t *A, uint16_t *B);
extern void pmod_mat_mul_asm_50_50_50(uint16_t *C, uint16_t *A, uint16_t *B);
extern void pmod_mat_mul_asm_55_55_55(uint16_t *C, uint16_t *A, uint16_t *B);
extern void pmod_mat_mul_asm_60_60_60(uint16_t *C, uint16_t *A, uint16_t *B);
extern void pmod_mat_mul_asm_70_70_70(uint16_t *C, uint16_t *A, uint16_t *B);
extern void pmod_mat_mul_8_asm0_80_80(uint16_t *C, uint16_t *A, uint16_t *B);
extern void pmod_mat_mul_asm_24_120_24(uint16_t *C, uint16_t *A, uint16_t *B);
extern void pmod_mat_mul_asm_24_168_24(uint16_t *C, uint16_t *A, uint16_t *B);
extern void pmod_mat_mul_asm_24_192_24(uint16_t *C, uint16_t *A, uint16_t *B);
extern void pmod_mat_mul_asm_24_240_24(uint16_t *C, uint16_t *A, uint16_t *B);
extern void pmod_mat_mul_asm_24_576_24(uint16_t *C, uint16_t *A, uint16_t *B);
extern void pmod_mat_mul_asm_29_103_65(uint16_t *C, uint16_t *A, uint16_t *B);

extern void pmod_mat_mul_8_asm_8_8_8(uint16_t *C, uint16_t *A, uint16_t *B);
extern void pmod_mat_mul_8_asm_13_8_8(uint16_t *C, uint16_t *A, uint16_t *B);
extern void pmod_mat_mul_8_asm_8_13_8(uint16_t *C, uint16_t *A, uint16_t *B);
extern void pmod_mat_mul_8_asm_8_8_13(uint16_t *C, uint16_t *A, uint16_t *B);
extern void pmod_mat_mul_8_asm_13_13_13(uint16_t *C, uint16_t *A, uint16_t *B);
extern void pmod_mat_mul_8_asm_16_16_16(uint16_t *C, uint16_t *A, uint16_t *B);
extern void pmod_mat_mul_8_asm_24_24_24(uint16_t *C, uint16_t *A, uint16_t *B);
extern void pmod_mat_mul_8_asm_32_32_32(uint16_t *C, uint16_t *A, uint16_t *B);

extern void pmod_mat_reduce_asm(uint16_t *C, int C_r, int C_c, uint16_t *tmp);

extern void pmod_mat_mul_asm_2_k_k(uint16_t *C, uint16_t *A, uint16_t *B);
extern void pmod_mat_mul_asm_2_mn_k(uint16_t *C, uint16_t *A, uint16_t *B);
extern void pmod_mat_mul_asm_k_mn_k(uint16_t *C, uint16_t *A, uint16_t *B);
extern void pmod_mat_mul_asm_m_n_m(uint16_t *C, uint16_t *A, uint16_t *B);
extern void pmod_mat_mul_asm_m_n_n(uint16_t *C, uint16_t *A, uint16_t *B);

extern void pmod_mat_mul_8_asm_2_k_k(uint16_t *C, uint16_t *A, uint16_t *B);
extern void pmod_mat_mul_8_asm_2_mn_k(uint16_t *C, uint16_t *A, uint16_t *B);
extern void pmod_mat_mul_8_asm_k_mn_k(uint16_t *C, uint16_t *A, uint16_t *B);
extern void pmod_mat_mul_8_asm_m_n_m(uint16_t *C, uint16_t *A, uint16_t *B);
extern void pmod_mat_mul_8_asm_m_n_n(uint16_t *C, uint16_t *A, uint16_t *B);

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

// Default matrix multiplication, but directly stored into C
void pmod_mat_mul_1_prime(pmod_mat_t *C, int C_r, int C_c, pmod_mat_t *A, int A_r, int A_c, pmod_mat_t *B, int B_r, int B_c)
{
  for (int c = 0; c < C_c; c++)
    for (int r = 0; r < C_r; r++)
    {
      uint64_t val = 0;

      for (int i = 0; i < A_c; i++)
        val = (val + (uint64_t)pmod_mat_entry(A, A_r, A_c, r, i) * (uint64_t)pmod_mat_entry(B, B_r, B_c, i, c));

      pmod_mat_set_entry(C, C_r, C_c, r, c, val % MEDS_p);
    }
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

// NEON matrix multiplication, assuming C_r and C_c are multiples of 4
void pmod_mat_mul_simd_1(pmod_mat_t *C, int C_r, int C_c, pmod_mat_t *A, int A_r, int A_c, pmod_mat_t *B, int B_r, int B_c)
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
      vst1_u16(&C[result_index], C_red_u16);
    }
}

// NEON matrix multiplication, assuming C_r and C_c are multiples of 2
void pmod_mat_mul_simd_1_pad2(pmod_mat_t *C, int C_r, int C_c, pmod_mat_t *A, int A_r, int A_c, pmod_mat_t *B, int B_r, int B_c)
{
  // Add at least 2 elements to the result matrix to prevent a segmentation fault
  uint32_t tmp[C_r * C_c + 2];

  int Ai, Bi, Ci;
  uint16x4_t B0, B1, B2, B3;
  uint32x4_t C0, C1, C2, C3;

  for (int c = 0; c < C_c; c += 4)
    for (int r = 0; r < C_r; r += 4)
    {
      // In every inner loop, we compute a 4x4 block of the result matrix
      // This 4x4 block is represented by the vectors (of size 4) C0, C1, C2, C3

      int r_do_4 = r + 4 <= C_r; // Whether r is at least 4 away from the bottom of the result matrix
      int c_do_4 = c + 4 <= C_c; // Whether c is at least 4 away from the right of the result matrix

      C0 = vmovq_n_u32(0);
      C1 = vmovq_n_u32(0);
      C2 = vmovq_n_u32(0);
      C3 = vmovq_n_u32(0);

      // In every iteration of the following loop, we compute a contribution to the 4x4 result block
      // Specifically, we compute the product of a 4x4 block of A and a 4x4 block of B and add it to the result block
      for (int k = 0; k < A_c; k += 4)
      {
        int k_do_4 = k + 4 <= A_c; // Whether k is at least 4 away from the right of A and the bottom of B

        Ai = r * A_c + k; // Index A[r][k]
        Bi = k * B_c + c; // Index B[k][c]

        int A0 = Ai;           // A[r + 0][k]
        int A1 = Ai + A_c;     // A[r + 1][k]
        int A2 = Ai + 2 * A_c; // A[r + 2][k]
        int A3 = Ai + 3 * A_c; // A[r + 3][k]

        if (k_do_4)
        {
          // Compute a 4x4 submatrix of the result

          // Load the 16 elements from B.
          // In the case where c_do_4 is 0, we can still load the excess elements, as they do not matter.
          // However, to prevent a segmentation fault, we can't do this for B3.
          B0 = vld1_u16(&B[Bi]);
          B1 = vld1_u16(&B[Bi + B_c]);
          B2 = vld1_u16(&B[Bi + 2 * B_c]);
          if (c_do_4)
            B3 = vld1_u16(&B[Bi + 3 * B_c]);
          else
          {
            // c is 2 away from the right of the result matrix, meaning we can only load 2 elements.
            B3 = vmov_n_u16(0);
            B3[0] = B[Bi + 3 * B_c];
            B3[1] = B[Bi + 3 * B_c + 1];
          }

          C0 = vmlal_n_u16(C0, B0, A[A0 + 0]);
          C0 = vmlal_n_u16(C0, B1, A[A0 + 1]);
          C0 = vmlal_n_u16(C0, B2, A[A0 + 2]);
          C0 = vmlal_n_u16(C0, B3, A[A0 + 3]);

          C1 = vmlal_n_u16(C1, B0, A[A1 + 0]);
          C1 = vmlal_n_u16(C1, B1, A[A1 + 1]);
          C1 = vmlal_n_u16(C1, B2, A[A1 + 2]);
          C1 = vmlal_n_u16(C1, B3, A[A1 + 3]);

          // Only compute the bottom two rows if r is at least 4 away from the bottom of the result matrix
          if (r_do_4)
          {
            C2 = vmlal_n_u16(C2, B0, A[A2 + 0]);
            C2 = vmlal_n_u16(C2, B1, A[A2 + 1]);
            C2 = vmlal_n_u16(C2, B2, A[A2 + 2]);
            C2 = vmlal_n_u16(C2, B3, A[A2 + 3]);

            C3 = vmlal_n_u16(C3, B0, A[A3 + 0]);
            C3 = vmlal_n_u16(C3, B1, A[A3 + 1]);
            C3 = vmlal_n_u16(C3, B2, A[A3 + 2]);
            C3 = vmlal_n_u16(C3, B3, A[A3 + 3]);
          }
        }
        else
        {
          // Compute a 2x2 submatrix of the result
          B0 = vld1_u16(&B[Bi]);
          B1 = vld1_u16(&B[Bi + B_c]);

          C0 = vmlal_n_u16(C0, B0, A[A0 + 0]);
          C0 = vmlal_n_u16(C0, B1, A[A0 + 1]);

          C1 = vmlal_n_u16(C1, B0, A[A1 + 0]);
          C1 = vmlal_n_u16(C1, B1, A[A1 + 1]);

          C2 = vmlal_n_u16(C2, B0, A[A2 + 0]);
          C2 = vmlal_n_u16(C2, B1, A[A2 + 1]);

          C3 = vmlal_n_u16(C3, B0, A[A3 + 0]);
          C3 = vmlal_n_u16(C3, B1, A[A3 + 1]);
        }
      }

      // Store the result block
      // Depending on whether r and c are far enough from the bottom and right of the result matrix,
      // we store the result block in different ways.
      Ci = C_c * r + c; // Index C[r][c]
      if (c_do_4)
      {
        // Store 4 elements at a time
        vst1q_u32(&tmp[Ci], C0);
        vst1q_u32(&tmp[Ci + C_c], C1);
        // Only store the bottom two rows if r is at least 4 away from the bottom of the result matrix
        if (r_do_4)
        {
          vst1q_u32(&tmp[Ci + 2 * C_c], C2);
          vst1q_u32(&tmp[Ci + 3 * C_c], C3);
        }
      }
      else
      {
        // Store 2 elements at a time (paralellization not possible)
        tmp[Ci] = C0[0];
        tmp[Ci + 1] = C0[1];
        tmp[Ci + C_c] = C1[0];
        tmp[Ci + C_c + 1] = C1[1];
        // Only store the bottom two rows if r is at least 4 away from the bottom of the result matrix
        if (r_do_4)
        {
          tmp[Ci + 2 * C_c] = C2[0];
          tmp[Ci + 2 * C_c + 1] = C2[1];
          tmp[Ci + 3 * C_c] = C3[0];
          tmp[Ci + 3 * C_c + 1] = C3[1];
        }
      }
    }

  // The following seems faster???
  // for (int r = 0; r < C_r; r++)
  //   for (int c = 0; c < C_c; c++)
  //     pmod_mat_set_entry(C, C_r, C_c, r, c, tmp[r * C_c + c] % MEDS_p);

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

      // Store into the result matrix.
      // Technique depends on whether c is at least 4 away from the right of the result matrix
      int result_index = r * C_c + c;
      int c_do_4 = c + 4 <= C_c;
      if (c_do_4)
        vst1_u16(&C[result_index], C_red_u16);
      else
      {
        // Store 2 elements at a time (paralellization not possible)
        C[result_index] = C_red_u16[0];
        C[result_index + 1] = C_red_u16[1];
      }
    }
}

void pmod_mat_reduce(pmod_mat_t *C, int C_r, int C_c, uint32_t *tmp)
{
  for (int i = 0; i < C_r * C_c; i++)
    C[i] = tmp[i] % MEDS_p;
}

void pmod_mat_reduce_2(pmod_mat_t *C, int C_r, int C_c, uint32_t *tmp)
{
  for (int i = 0; i < C_r * C_c; i++)
  {
    uint32_t val = tmp[i];
    uint32_t temp = (val * 2149057666) >> 43;
    C[i] = val - temp * MEDS_p;
  }
}

void pmod_mat_reduce_2_simd(pmod_mat_t *C, int C_r, int C_c, uint32_t *tmp)
{
  // Define constants
  const uint32x4_t MEDS_p_v = vdupq_n_u32(MEDS_p);
  const uint32x2_t multiplier_v = vdup_n_u32(2149057666);
  const int shift_amount = 43;

  // Loop over the matrix elements in steps of 4 (NEON register width)
  for (int i = 0; i < C_r * C_c; i += 4)
  {
    // Load 4 elements from tmp
    uint32x4_t val_v = vld1q_u32(&tmp[i]);

    // Multiply each element by the multiplier
    uint64x2_t temp_64x2_1 = vmull_u32(vget_low_u32(val_v), multiplier_v);
    temp_64x2_1 = vshrq_n_u64(temp_64x2_1, shift_amount);
    uint64x2_t temp_64x2_2 = vmull_u32(vget_high_u32(val_v), multiplier_v);
    temp_64x2_2 = vshrq_n_u64(temp_64x2_2, shift_amount);

    // Convert to 32-bit
    uint32x2_t temp_32x2_1 = vmovn_u64(temp_64x2_1);
    uint32x2_t temp_32x2_2 = vmovn_u64(temp_64x2_2);

    // Duplicate to create a 32-bit vector
    uint32x4_t temp_v = vcombine_u32(temp_32x2_1, temp_32x2_2);

    // Multiply each temp element by MEDS_p
    temp_v = vmulq_u32(temp_v, MEDS_p_v);

    // Subtract temp * MEDS_p from val
    uint32x4_t result_v = vsubq_u32(val_v, temp_v);
    uint16x4_t result_v_u16 = vqmovn_u32(result_v);

    // Store the result back to C
    vst1_u16(&C[i], result_v_u16);
  }
}

// NEON matrix multiplication with padding
void pmod_mat_mul_simd_1_pad(pmod_mat_t *C, int C_r, int C_c, pmod_mat_t *A, int A_r, int A_c, pmod_mat_t *B, int B_r, int B_c)
{
  // Add at least 4 elements to the result matrix to prevent a segmentation fault
  uint32_t tmp[C_r * C_c + 4] __attribute__((aligned(16)));

  int Ai, Bi, Ci;
  uint16x4_t B0, B1, B2, B3;
  uint32x4_t C0, C1, C2, C3;

  for (int r = 0; r < C_r; r += 4)
    for (int c = 0; c < C_c; c += 4)
    {
      // In every inner loop, we compute a 4x4 block of the result matrix
      // This 4x4 block is represented by the vectors (of size 4) C0, C1, C2, C3

      int r_dist = C_r - r; // Distance between r and the bottom of the result matrix
      int c_dist = C_c - c; // Distance between c and the right of the result matrix

      C0 = vmovq_n_u32(0);
      C1 = vmovq_n_u32(0);
      C2 = vmovq_n_u32(0);
      C3 = vmovq_n_u32(0);

      // In every iteration of the following loop, we compute a contribution to the 4x4 result block
      // Specifically, we compute the product of a 4x4 block of A and a 4x4 block of B and add it to the result block
      for (int k = 0; k < A_c; k += 4)
      {
        int k_dist = A_c - k; // Distance between k and the right of A and the bottom of B

        Ai = r * A_c + k; // Index A[r][k]
        Bi = k * B_c + c; // Index B[k][c]

        int A0 = Ai;           // A[r + 0][k]
        int A1 = Ai + A_c;     // A[r + 1][k]
        int A2 = Ai + 2 * A_c; // A[r + 2][k]
        int A3 = Ai + 3 * A_c; // A[r + 3][k]

        // Compute a 4x4 submatrix of the result
        if (k_dist > 3)
        {
          // Load the 16 elements from B.
          // In the case where c_dist < 4, we can still load the excess elements, as they do not matter.
          // However, to prevent a segmentation fault, we can't do this for B3.
          B0 = vld1_u16(&B[Bi]);
          B1 = vld1_u16(&B[Bi + B_c]);
          B2 = vld1_u16(&B[Bi + 2 * B_c]);
          if (c_dist > 3)
            B3 = vld1_u16(&B[Bi + 3 * B_c]);
          else
          {
            // c is close to the right of the result matrix, meaning we can only load a few elements.
            B3 = vmov_n_u16(0);
            if (c_dist > 0)
              B3[0] = B[Bi + 3 * B_c];
            if (c_dist > 1)
              B3[1] = B[Bi + 3 * B_c + 1];
            if (c_dist > 2)
              B3[2] = B[Bi + 3 * B_c + 2];
          }
          // An alternative solution to the above code is to make sure that B has a few extra (unallocated) elements

          C0 = vmlal_n_u16(C0, B0, A[A0 + 0]);
          C0 = vmlal_n_u16(C0, B1, A[A0 + 1]);
          C0 = vmlal_n_u16(C0, B2, A[A0 + 2]);
          C0 = vmlal_n_u16(C0, B3, A[A0 + 3]);

          if (r_dist > 1)
          {
            C1 = vmlal_n_u16(C1, B0, A[A1 + 0]);
            C1 = vmlal_n_u16(C1, B1, A[A1 + 1]);
            C1 = vmlal_n_u16(C1, B2, A[A1 + 2]);
            C1 = vmlal_n_u16(C1, B3, A[A1 + 3]);
          }

          // Only compute the bottom two rows if r is at least 4 away from the bottom of the result matrix
          if (r_dist > 2)
          {
            C2 = vmlal_n_u16(C2, B0, A[A2 + 0]);
            C2 = vmlal_n_u16(C2, B1, A[A2 + 1]);
            C2 = vmlal_n_u16(C2, B2, A[A2 + 2]);
            C2 = vmlal_n_u16(C2, B3, A[A2 + 3]);
          }

          if (r_dist > 3)
          {
            C3 = vmlal_n_u16(C3, B0, A[A3 + 0]);
            C3 = vmlal_n_u16(C3, B1, A[A3 + 1]);
            C3 = vmlal_n_u16(C3, B2, A[A3 + 2]);
            C3 = vmlal_n_u16(C3, B3, A[A3 + 3]);
          }
        }
        else if (k_dist > 2)
        {
          B0 = vld1_u16(&B[Bi]);
          B1 = vld1_u16(&B[Bi + B_c]);
          B2 = vld1_u16(&B[Bi + 2 * B_c]);

          C0 = vmlal_n_u16(C0, B0, A[A0 + 0]);
          C0 = vmlal_n_u16(C0, B1, A[A0 + 1]);
          C0 = vmlal_n_u16(C0, B2, A[A0 + 2]);

          C1 = vmlal_n_u16(C1, B0, A[A1 + 0]);
          C1 = vmlal_n_u16(C1, B1, A[A1 + 1]);
          C1 = vmlal_n_u16(C1, B2, A[A1 + 2]);

          C2 = vmlal_n_u16(C2, B0, A[A2 + 0]);
          C2 = vmlal_n_u16(C2, B1, A[A2 + 1]);
          C2 = vmlal_n_u16(C2, B2, A[A2 + 2]);

          C3 = vmlal_n_u16(C3, B0, A[A3 + 0]);
          C3 = vmlal_n_u16(C3, B1, A[A3 + 1]);
          C3 = vmlal_n_u16(C3, B2, A[A3 + 2]);
        }
        else if (k_dist > 1)
        {
          B0 = vld1_u16(&B[Bi]);
          B1 = vld1_u16(&B[Bi + B_c]);

          C0 = vmlal_n_u16(C0, B0, A[A0 + 0]);
          C0 = vmlal_n_u16(C0, B1, A[A0 + 1]);

          C1 = vmlal_n_u16(C1, B0, A[A1 + 0]);
          C1 = vmlal_n_u16(C1, B1, A[A1 + 1]);

          C2 = vmlal_n_u16(C2, B0, A[A2 + 0]);
          C2 = vmlal_n_u16(C2, B1, A[A2 + 1]);

          C3 = vmlal_n_u16(C3, B0, A[A3 + 0]);
          C3 = vmlal_n_u16(C3, B1, A[A3 + 1]);
        }
        else
        {
          B0 = vld1_u16(&B[Bi]);

          C0 = vmlal_n_u16(C0, B0, A[A0 + 0]);

          C1 = vmlal_n_u16(C1, B0, A[A1 + 0]);

          C2 = vmlal_n_u16(C2, B0, A[A2 + 0]);

          C3 = vmlal_n_u16(C3, B0, A[A3 + 0]);
        }
      }

      // Store the result block
      // Depending on whether r and c are far enough from the bottom and right of the result matrix,
      // we store the result block in different ways.
      Ci = C_c * r + c; // Index C[r][c]

      if (c_dist > 3)
      {
        // Store all 4 values of C0-C4
        if (r_dist > 0)
          vst1q_u32(&tmp[Ci], C0);
        if (r_dist > 1)
          vst1q_u32(&tmp[Ci + C_c], C1);
        if (r_dist > 2)
          vst1q_u32(&tmp[Ci + 2 * C_c], C2);
        if (r_dist > 3)
          vst1q_u32(&tmp[Ci + 3 * C_c], C3);
      }
      else
      {
        // Store some values of C0-C4
        if (r_dist > 0)
        {
          if (c_dist > 0)
            tmp[Ci] = C0[0];
          if (c_dist > 1)
            tmp[Ci + 1] = C0[1];
          if (c_dist > 2)
            tmp[Ci + 2] = C0[2];
        }
        if (r_dist > 1)
        {
          if (c_dist > 0)
            tmp[Ci + C_c] = C1[0];
          if (c_dist > 1)
            tmp[Ci + C_c + 1] = C1[1];
          if (c_dist > 2)
            tmp[Ci + C_c + 2] = C1[2];
        }
        if (r_dist > 2)
        {
          if (c_dist > 0)
            tmp[Ci + 2 * C_c] = C2[0];
          if (c_dist > 1)
            tmp[Ci + 2 * C_c + 1] = C2[1];
          if (c_dist > 2)
            tmp[Ci + 2 * C_c + 2] = C2[2];
        }
        if (r_dist > 3)
        {
          if (c_dist > 0)
            tmp[Ci + 3 * C_c] = C3[0];
          if (c_dist > 1)
            tmp[Ci + 3 * C_c + 1] = C3[1];
          if (c_dist > 2)
            tmp[Ci + 3 * C_c + 2] = C3[2];
        }
      }
    }

  // pmod_mat_reduce(C, C_r, C_c, &tmp);

  // The following seems faster???
  for (int i = 0; i < C_r * C_c; i++)
    C[i] = tmp[i] % MEDS_p;

  // Reduce the result matrix using NEON intrinsics
  // uint32x4_t C_red;
  // uint16x4_t C_red_u16;
  // uint32x4_t C_tmp;
  // uint32x4_t C_diff;
  // uint32x4_t C_mask;
  // uint32x4_t C_MEDS_p = vdupq_n_u32(MEDS_p);
  // uint32x4_t C_one = vdupq_n_u32(1);
  // for (int r = 0; r < C_r; r++)
  //   for (int c = 0; c < C_c; c += 4)
  //   {
  //     // Load 4 values from the result matrix
  //     C_red = vld1q_u32(&tmp[r * C_c + c]);

  //     // Apply two reductions
  //     C_tmp = vshrq_n_u32(C_red, GFq_bits);
  //     C_tmp = vmulq_n_u32(C_tmp, MEDS_p);
  //     C_red = vsubq_u32(C_red, C_tmp);
  //     C_tmp = vshrq_n_u32(C_red, GFq_bits);
  //     C_tmp = vmulq_n_u32(C_tmp, MEDS_p);
  //     C_red = vsubq_u32(C_red, C_tmp);

  //     // Reduce to a value between 0 and MEDS_p - 1:
  //     C_diff = vsubq_u32(C_red, C_MEDS_p);
  //     C_mask = vandq_u32(vshrq_n_u32(C_diff, 31), C_one);
  //     C_red = vaddq_u32(vmulq_u32(C_mask, C_red), vmulq_u32(vsubq_u32(C_one, C_mask), C_diff));

  //     // Convert to smaller type
  //     C_red_u16 = vqmovn_u32(C_red);

  //     // Store into the result matrix.
  //     // Technique depends on whether c is at least 4 away from the right of the result matrix
  //     int result_index = r * C_c + c;
  //     int c_dist = C_c - c;
  //     if (c_dist > 3)
  //     {
  //       vst1_u16(&C[result_index], C_red_u16);
  //     }
  //     else
  //     {
  //       // Store some values of C0-C4
  //       if (c_dist > 0)
  //         C[result_index] = C_red_u16[0];
  //       if (c_dist > 1)
  //         C[result_index + 1] = C_red_u16[1];
  //       if (c_dist > 2)
  //         C[result_index + 2] = C_red_u16[2];
  //     }
  //   }
}

// This implementation is based on the one in https://github.com/IIS-summer-2023/meds-simd-lowlevel/blob/main/ref/matrixmod.c
// It is slower than my own implementation.
void pmod_mat_mul_simd_2(pmod_mat_t *C, int C_r, int C_c, pmod_mat_t *A, int A_r, int A_c, pmod_mat_t *B, int B_r, int B_c)
{
  uint32_t tmp[C_r * C_c];

  int words_per_block = 4;
  int blocks = ceil(C_c / words_per_block);

  for (int block = 0; block < blocks; block++)
  {
    for (int i = 0; i < A_r; i++)
    {
      uint32x4_t result = vdupq_n_u32(0);
      for (int j = 0; j < A_c; j++)
      {
        uint16x4_t a = vdup_n_u16(A[i * A_c + j]);
        uint16x4_t b = vld1_u16(B + j * B_c + block * words_per_block);
        result = vmlal_u16(result, a, b);
      }
      vst1q_u32(&tmp[i * C_c + block * words_per_block], result);
    }
  }

  for (int i = 0; i < C_r * C_c; i++)
    C[i] = tmp[i] % MEDS_p;
}

#define EXECUTE_ONE_VMLAL(Cl, Ch, A, Ai, B0, B1, B2, B3, B4, B5, B6, B7) \
  ({                                                                     \
    Cl = vmlal_n_u16(Cl, vget_low_u16(B0), A[Ai + 0]);                   \
    Ch = vmlal_high_n_u16(Ch, B0, A[Ai + 0]);                            \
    Cl = vmlal_n_u16(Cl, vget_low_u16(B1), A[Ai + 1]);                   \
    Ch = vmlal_high_n_u16(Ch, B1, A[Ai + 1]);                            \
    Cl = vmlal_n_u16(Cl, vget_low_u16(B2), A[Ai + 2]);                   \
    Ch = vmlal_high_n_u16(Ch, B2, A[Ai + 2]);                            \
    Cl = vmlal_n_u16(Cl, vget_low_u16(B3), A[Ai + 3]);                   \
    Ch = vmlal_high_n_u16(Ch, B3, A[Ai + 3]);                            \
    Cl = vmlal_n_u16(Cl, vget_low_u16(B4), A[Ai + 4]);                   \
    Ch = vmlal_high_n_u16(Ch, B4, A[Ai + 4]);                            \
    Cl = vmlal_n_u16(Cl, vget_low_u16(B5), A[Ai + 5]);                   \
    Ch = vmlal_high_n_u16(Ch, B5, A[Ai + 5]);                            \
    Cl = vmlal_n_u16(Cl, vget_low_u16(B6), A[Ai + 6]);                   \
    Ch = vmlal_high_n_u16(Ch, B6, A[Ai + 6]);                            \
    Cl = vmlal_n_u16(Cl, vget_low_u16(B7), A[Ai + 7]);                   \
    Ch = vmlal_high_n_u16(Ch, B7, A[Ai + 7]);                            \
  })

#define REDUCE_ZIP(Cl, Ch, Cout, Ctmp) \
  ({                                   \
    Cout = FREEZE_REDUCE_VEC_16x4(Cl); \
    Ctmp = FREEZE_REDUCE_VEC_16x4(Ch); \
    Cout = vuzp1q_u16(Cout, Ctmp);     \
  })

void pmod_mat_mul_simd_v8(pmod_mat_t *C, int C_r, int C_c, pmod_mat_t *A, int A_r, int A_c, pmod_mat_t *B, int B_r, int B_c)
{
  int Ai, Bi, Ci;
  uint16x8_t B0, B1, B2, B3, B4, B5, B6, B7;
  uint32x4_t C0l, C0h, C1l, C1h, C2l, C2h, C3l, C3h, C4l, C4h, C5l, C5h, C6l, C6h, C7l, C7h;
  uint16x8_t C0, C1, C2, C3, C4, C5, C6, C7, Ct;

  for (int r = 0; r < C_r; r += 8)
    for (int c = 0; c < C_c; c += 8)
    {
      C0l = vmovq_n_u32(0);
      C0h = vmovq_n_u32(0);
      C1l = vmovq_n_u32(0);
      C1h = vmovq_n_u32(0);
      C2l = vmovq_n_u32(0);
      C2h = vmovq_n_u32(0);
      C3l = vmovq_n_u32(0);
      C3h = vmovq_n_u32(0);
      C4l = vmovq_n_u32(0);
      C4h = vmovq_n_u32(0);
      C5l = vmovq_n_u32(0);
      C5h = vmovq_n_u32(0);
      C6l = vmovq_n_u32(0);
      C6h = vmovq_n_u32(0);
      C7l = vmovq_n_u32(0);
      C7h = vmovq_n_u32(0);

      for (int k = 0; k < A_c; k += 8)
      {
        Ai = r * A_c + k; // Index A[r][k]
        Bi = k * B_c + c; // Index B[k][c]

        int A0 = Ai;           // A[r + 0][k]
        int A1 = Ai + A_c;     // A[r + 1][k]
        int A2 = Ai + 2 * A_c; // A[r + 2][k]
        int A3 = Ai + 3 * A_c; // A[r + 3][k]
        int A4 = Ai + 4 * A_c; // A[r + 4][k]
        int A5 = Ai + 5 * A_c; // A[r + 5][k]
        int A6 = Ai + 6 * A_c; // A[r + 6][k]
        int A7 = Ai + 7 * A_c; // A[r + 7][k]

        B0 = vld1q_u16(&B[Bi]);
        B1 = vld1q_u16(&B[Bi + B_c]);
        B2 = vld1q_u16(&B[Bi + 2 * B_c]);
        B3 = vld1q_u16(&B[Bi + 3 * B_c]);
        B4 = vld1q_u16(&B[Bi + 4 * B_c]);
        B5 = vld1q_u16(&B[Bi + 5 * B_c]);
        B6 = vld1q_u16(&B[Bi + 6 * B_c]);
        B7 = vld1q_u16(&B[Bi + 7 * B_c]);

        EXECUTE_ONE_VMLAL(C0l, C0h, A, A0, B0, B1, B2, B3, B4, B5, B6, B7);
        EXECUTE_ONE_VMLAL(C1l, C1h, A, A1, B0, B1, B2, B3, B4, B5, B6, B7);
        EXECUTE_ONE_VMLAL(C2l, C2h, A, A2, B0, B1, B2, B3, B4, B5, B6, B7);
        EXECUTE_ONE_VMLAL(C3l, C3h, A, A3, B0, B1, B2, B3, B4, B5, B6, B7);
        EXECUTE_ONE_VMLAL(C4l, C4h, A, A4, B0, B1, B2, B3, B4, B5, B6, B7);
        EXECUTE_ONE_VMLAL(C5l, C5h, A, A5, B0, B1, B2, B3, B4, B5, B6, B7);
        EXECUTE_ONE_VMLAL(C6l, C6h, A, A6, B0, B1, B2, B3, B4, B5, B6, B7);
        EXECUTE_ONE_VMLAL(C7l, C7h, A, A7, B0, B1, B2, B3, B4, B5, B6, B7);
      }

      REDUCE_ZIP(C0l, C0h, C0, Ct);
      REDUCE_ZIP(C1l, C1h, C1, Ct);
      REDUCE_ZIP(C2l, C2h, C2, Ct);
      REDUCE_ZIP(C3l, C3h, C3, Ct);
      REDUCE_ZIP(C4l, C4h, C4, Ct);
      REDUCE_ZIP(C5l, C5h, C5, Ct);
      REDUCE_ZIP(C6l, C6h, C6, Ct);
      REDUCE_ZIP(C7l, C7h, C7, Ct);

      Ci = C_c * r + c; // Index C[r][c]
      vst1q_u16(&C[Ci], C0);
      vst1q_u16(&C[Ci + C_c], C1);
      vst1q_u16(&C[Ci + 2 * C_c], C2);
      vst1q_u16(&C[Ci + 3 * C_c], C3);
      vst1q_u16(&C[Ci + 4 * C_c], C4);
      vst1q_u16(&C[Ci + 5 * C_c], C5);
      vst1q_u16(&C[Ci + 6 * C_c], C6);
      vst1q_u16(&C[Ci + 7 * C_c], C7);
    }
}

float min_cycle_bound(int m, int o, int n)
{
  // float r_loops = m / 8.0;
  // float c_loops = n / 8.0;
  // float k_loops = o / 8.0;

  // float result = 0;
  // result += r_loops;                             // R loop add and compare
  // result += r_loops * c_loops;                   // C loop add and compare
  // result += r_loops * c_loops * k_loops;         // K loop add and compare
  // result += r_loops * c_loops * k_loops * 2;     // K loop address calculation
  // result += r_loops * c_loops * k_loops * 8;         // Load A
  // result += r_loops * c_loops * k_loops * 8;         // Load B
  // result += r_loops * c_loops * k_loops * 8 * 8 * 2; // Compute C
  // result += r_loops * c_loops * 8 * 5 * 2 + 1;       // Reduce C
  // result += r_loops * c_loops * 8;                   // Store C

  // return 8 * result;

  // float result = (1 / 16.0) * m * n * (6 * o + 24);

  float result = (1 / 64.0) * m * n * (18 * o + 88);
  // result += (1 / 64.0) * m * n * 1;
  return 8 * result;

  // float result = 0;
  // result += m * n * o * 1; // Arithmetic
  // result += m * n * 5;     // Modular reduction
  // return result;
}
// float min_cycle_bound(int m, int o, int n)
// {
//   int loads = 0;
//   int stores = 0;
//   int arithmetic = 0;
//   int reduction_cost = 5;

//   for (int c = 0; c < m; c++)
//     for (int r = 0; r < n; r++)
//     {
//       for (int i = 0; i < o; i++)
//       {
//         // val = (val + (uint64_t)pmod_mat_entry(A, A_r, A_c, r, i) * (uint64_t)pmod_mat_entry(B, B_r, B_c, i, c));
//         loads += 2;
//         arithmetic += 2;
//       }

//       // pmod_mat_set_entry(C, C_r, C_c, r, c, val % MEDS_p);
//       arithmetic += reduction_cost;
//       stores++;
//     }
//   return loads + stores + arithmetic;
// }

void test_performance(char name[], int m, int n, int o, void (*function)(pmod_mat_t *, pmod_mat_t *, pmod_mat_t *))
{
  uint8_t seed[MEDS_pub_seed_bytes];

  for (int i = 0; i < MEDS_pub_seed_bytes; i++)
  {
    seed[i] = i;
  }
  keccak_state shake;
  shake256_absorb_once(&shake, seed, MEDS_pub_seed_bytes);

  __attribute__((aligned(16))) pmod_mat_t A[m * o];
  __attribute__((aligned(16))) pmod_mat_t B[o * n];
  __attribute__((aligned(16))) pmod_mat_t C1[m * n];
  __attribute__((aligned(16))) pmod_mat_t C2[m * n];
  __attribute__((aligned(16))) pmod_mat_t C3[m * n];

  for (int i = 0; i < m * o; i++)
    A[i] = rnd_GF(&shake);
  for (int i = 0; i < o * n; i++)
    B[i] = rnd_GF(&shake);

  long long matmul_cycles[ROUNDS + 1];
  long long intrinsic_matmul_cycles[ROUNDS + 1];
  long long asm_matmul_cycles[ROUNDS + 1];

  for (int round = 0; round < ROUNDS; round++)
  {
    matmul_cycles[round] = get_cyclecounter();
    pmod_mat_mul_1_prime(C1, m, n, A, m, o, B, o, n);
  }
  matmul_cycles[ROUNDS] = get_cyclecounter();

  for (int round = 0; round < ROUNDS; round++)
  {
    intrinsic_matmul_cycles[round] = get_cyclecounter();
    pmod_mat_mul_simd_1_pad(C2, m, n, A, m, o, B, o, n);
  }
  intrinsic_matmul_cycles[ROUNDS] = get_cyclecounter();

  for (int round = 0; round < ROUNDS; round++)
  {
    asm_matmul_cycles[round] = get_cyclecounter();
    function(C3, A, B);
  }
  asm_matmul_cycles[ROUNDS] = get_cyclecounter();

  // Calculate results
  double matmul_minimum_cc = min(matmul_cycles, ROUNDS + 1);
  double intrinsic_matmul_minimum_cc = min(intrinsic_matmul_cycles, ROUNDS + 1);
  double asm_matmul_minimum_cc = min(asm_matmul_cycles, ROUNDS + 1);

  double min_cycles = (double)min_cycle_bound(m, o, n);
  double matmul_cycle_multiplier = matmul_minimum_cc / (min_cycles / 8);
  double intrinsic_cycle_multiplier = intrinsic_matmul_minimum_cc / (min_cycles / 8);
  double asm_cycle_multiplier = asm_matmul_minimum_cc / (min_cycles / 8);

  double intrinsic_improvement = (intrinsic_matmul_minimum_cc - matmul_minimum_cc) / matmul_minimum_cc * 100;
  double asm_improvement = (asm_matmul_minimum_cc - matmul_minimum_cc) / matmul_minimum_cc * 100;

  int intrinsic_inequalities = 0;
  int asm_inequalities = 0;
  for (int i = 0; i < m * n; i++)
  {
    if (C1[i] != C2[i])
    {
      intrinsic_inequalities++;
    }
    if (C1[i] != C3[i])
    {
      asm_inequalities++;
    }
  }

  // Print results
  printf("-----------------------------------\n");
  printf("=== %s ===\n", name);
  printf("-----------------------------------\n");
  if (asm_inequalities == 0)
    printf("> ASM EQUAL\n");
  else
    printf("> ASM INEQUALITIES: %d/%d\n", asm_inequalities, m * n);
  if (intrinsic_inequalities == 0)
    printf("> INTRINSIC EQUAL\n");
  else
    printf("> INTRINSIC INEQUALITIES: %d/%d\n", intrinsic_inequalities, m * n);

  printf("Minimum cycle amount: %f\n", min_cycles);
  printf("Minimum cycle amount (8-way) parallel: %f\n", min_cycles / 8);
  printf("Matmul minimum: %f\t(x%f)\n", matmul_minimum_cc, matmul_cycle_multiplier);
  printf("Intrinsic minimum: %f\t(x%f)\n", intrinsic_matmul_minimum_cc, intrinsic_cycle_multiplier);
  printf("ASM minimum: %f\t(x%f)\n", asm_matmul_minimum_cc, asm_cycle_multiplier);

  printf("Improvement (intrinsic): %f%%\n", intrinsic_improvement);
  printf("Improvement (ASM): %f%%\n", asm_improvement);
  printf("\n");
}

void test_performance2(char name[], int m, int n, int o, void (*function)(pmod_mat_t *, pmod_mat_t *, pmod_mat_t *), void (*function8)(pmod_mat_t *, pmod_mat_t *, pmod_mat_t *))
{
  uint8_t seed[MEDS_pub_seed_bytes];

  for (int i = 0; i < MEDS_pub_seed_bytes; i++)
  {
    seed[i] = i;
  }
  keccak_state shake;
  shake256_absorb_once(&shake, seed, MEDS_pub_seed_bytes);

  __attribute__((aligned(16))) pmod_mat_t A[m * o];
  __attribute__((aligned(16))) pmod_mat_t B[o * n];
  __attribute__((aligned(16))) pmod_mat_t C1[m * n];
  __attribute__((aligned(16))) pmod_mat_t C2[m * n];
  __attribute__((aligned(16))) pmod_mat_t C3[m * n];

  for (int i = 0; i < m * o; i++)
    A[i] = rnd_GF(&shake);
  for (int i = 0; i < o * n; i++)
    B[i] = rnd_GF(&shake);
  for (int i = 0; i < m * n; i++)
  {
    C1[i] = 0;
    C2[i] = 0;
    C3[i] = 0;
  }

  long long matmul_cycles[ROUNDS + 1];
  long long asm_matmul8_cycles[ROUNDS + 1];
  long long asm_matmul_cycles[ROUNDS + 1];

  for (int round = 0; round < ROUNDS; round++)
  {
    matmul_cycles[round] = get_cyclecounter();
    pmod_mat_mul_1_prime(C1, m, n, A, m, o, B, o, n);
  }
  matmul_cycles[ROUNDS] = get_cyclecounter();

  for (int round = 0; round < ROUNDS; round++)
  {
    asm_matmul8_cycles[round] = get_cyclecounter();
    function8(C2, A, B);
    // pmod_mat_mul_simd_1_pad(C2, m, n, A, m, o, B, o, n);
    // pmod_mat_mul_simd_v8(C2, m, n, A, m, o, B, o, n);
  }
  asm_matmul8_cycles[ROUNDS] = get_cyclecounter();

  for (int round = 0; round < ROUNDS; round++)
  {
    asm_matmul_cycles[round] = get_cyclecounter();
    function(C3, A, B);
  }
  asm_matmul_cycles[ROUNDS] = get_cyclecounter();

  // Calculate results
  double matmul_minimum_cc = min(matmul_cycles, ROUNDS + 1);
  double asm_matmul8_minimum_cc = min(asm_matmul8_cycles, ROUNDS + 1);
  double asm_matmul_minimum_cc = min(asm_matmul_cycles, ROUNDS + 1);

  double min_cycles = (double)min_cycle_bound(m, o, n);
  double matmul_cycle_multiplier = matmul_minimum_cc / (min_cycles / 4);
  double asm8_cycle_multiplier = asm_matmul8_minimum_cc / (min_cycles / 4);
  double asm_cycle_multiplier = asm_matmul_minimum_cc / (min_cycles / 4);

  double asm8_improvement = (asm_matmul8_minimum_cc - matmul_minimum_cc) / matmul_minimum_cc * 100;
  double asm_improvement = (asm_matmul_minimum_cc - matmul_minimum_cc) / matmul_minimum_cc * 100;

  int asm8_inequalities = 0;
  int asm_inequalities = 0;
  for (int i = 0; i < m * n; i++)
  {
    if (C1[i] != C2[i])
    {
      asm8_inequalities++;
    }
    if (C1[i] != C3[i])
    {
      asm_inequalities++;
    }
  }

  // printf("C0\n");
  // pmod_mat_fprint(stdout, C1, m, n);
  // printf("C1\n");
  // pmod_mat_fprint(stdout, C2, m, n);
  // printf("C2\n");
  // pmod_mat_fprint(stdout, C3, m, n);

  // Print results
  printf("-----------------------------------\n");
  printf("=== %s ===\n", name);
  printf("-----------------------------------\n");
  if (asm_inequalities == 0)
    printf("> ASM EQUAL\n");
  else
    printf("> ASM INEQUALITIES: %d/%d\n", asm_inequalities, m * n);
  if (asm8_inequalities == 0)
    printf("> ASM-8 EQUAL\n");
  else
    printf("> ASM-8 INEQUALITIES: %d/%d\n", asm8_inequalities, m * n);

  printf("Minimum cycle amount: %f\n", min_cycles);
  printf("Minimum cycle amount (4-way) parallel: %f\n", min_cycles / 4);
  printf("Matmul minimum: %f\t(x%f)\n", matmul_minimum_cc, matmul_cycle_multiplier);
  printf("ASM-8 minimum: %f\t(x%f)\n", asm_matmul8_minimum_cc, asm8_cycle_multiplier);
  printf("ASM minimum: %f\t(x%f)\n", asm_matmul_minimum_cc, asm_cycle_multiplier);

  printf("Improvement (ASM-8): %f%%\n", asm8_improvement);
  printf("Improvement (ASM): %f%%\n", asm_improvement);
  printf("\n");
}

#define A_ROWS 30
#define A_COLS 30
#define B_ROWS A_COLS
#define B_COLS 30
#define C_ROWS A_ROWS
#define C_COLS B_COLS

/*int main(int argc, char *argv[])
{
  enable_cyclecounter();

  uint8_t seed1[MEDS_pub_seed_bytes];

  for (int i = 0; i < MEDS_pub_seed_bytes; i++)
  {
    seed1[i] = i;
  }

  __attribute__((aligned(16))) pmod_mat_t A[A_ROWS * A_COLS];
  __attribute__((aligned(16))) pmod_mat_t B[B_ROWS * B_COLS];
  __attribute__((aligned(16))) pmod_mat_t A2[A_ROWS * A_COLS];
  __attribute__((aligned(16))) pmod_mat_t B2[B_ROWS * B_COLS];
  __attribute__((aligned(16))) pmod_mat_t C1[C_ROWS * C_COLS];
  __attribute__((aligned(16))) pmod_mat_t C2[C_ROWS * C_COLS];

  keccak_state shake;
  shake256_absorb_once(&shake, seed1, MEDS_pub_seed_bytes);

  long long old_matmul_cycles[MATMUL_ROUNDS];
  long long new_matmul_cycles[MATMUL_ROUNDS];

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

  for (int round = 0; round < MATMUL_ROUNDS - 1; round++)
  {
    old_matmul_cycles[round] = get_cyclecounter();
    pmod_mat_mul_1(C1, C_ROWS, C_COLS, A, A_ROWS, A_COLS, B, B_ROWS, B_COLS);
  }
  old_matmul_cycles[MATMUL_ROUNDS - 1] = get_cyclecounter();

  for (int round = 0; round < MATMUL_ROUNDS - 1; round++)
  {
    new_matmul_cycles[round] = get_cyclecounter();
    pmod_mat_mul_asm_30_30_30(C2, A2, B2);
  }
  new_matmul_cycles[MATMUL_ROUNDS - 1] = get_cyclecounter();

  double old_matmul_minimum_cc = median_2(old_matmul_cycles, MATMUL_ROUNDS, 0);
  double new_matmul_minimum_cc = median_2(new_matmul_cycles, MATMUL_ROUNDS, 0);

  printf("Addresses of the matrices:\n");
  printf("A2: %p\n", A2);
  printf("B2: %p\n", B2);
  printf("C2: %p\n", C2);
  float cycle_bound = min_cycle_bound(C_ROWS, A_COLS, C_COLS);
  float improvement_possible = (new_matmul_minimum_cc / cycle_bound) * 100;
  // float old_matmul_std = standard_deviation(old_matmul_cycles, MATMUL_ROUNDS);
  // float new_matmul_std = standard_deviation(new_matmul_cycles, MATMUL_ROUNDS);
  double percentage = new_matmul_minimum_cc / old_matmul_minimum_cc * 100;
  double improvement = (new_matmul_minimum_cc - old_matmul_minimum_cc) / old_matmul_minimum_cc * 100;
  printf("Minimum cycle bound: %f\n", cycle_bound);
  printf("Improvement possible: -%f%% (x%f%%)\n", (improvement_possible - 100), improvement_possible / 100);
  printf("Old minimum: %f\n", old_matmul_minimum_cc);
  printf("New minimum: %f\n", new_matmul_minimum_cc);
  // printf("Old std: %f\n", old_matmul_std);
  // printf("New std: %f\n", new_matmul_std);
  printf("Percentage: %f%%\n", percentage);
  printf("Improvement: %f%%\n", improvement);

  disable_cyclecounter();

  // Compare matrices
  int exact_equalities = 0;
  int close_equalities = 0;
  int modulo_equalities = 0;
  for (int r = 0; r < C_ROWS; r++)
    for (int c = 0; c < C_COLS; c++)
    {
      GFq_t val1 = pmod_mat_entry(C1, C_ROWS, C_COLS, r, c);
      GFq_t val2 = pmod_mat_entry(C2, C_ROWS, C_COLS, r, c);
      if (val1 == val2)
        exact_equalities++;
      if (val1 % MEDS_p == val2 % MEDS_p)
        modulo_equalities++;
      val1 = val1 > MEDS_p ? val1 - MEDS_p : val1;
      val2 = val2 > MEDS_p ? val2 - MEDS_p : val2;
      if (val1 == val2)
        close_equalities++;
    }

  int expected_equalities = C_ROWS * C_COLS;

  printf("Equalities: %d / %d\n", exact_equalities, expected_equalities);
  printf("Close equalities (+/- 1 * MEDS_p): %d / %d\n", close_equalities, expected_equalities);
  printf("Modulo equalities: %d / %d\n", modulo_equalities, expected_equalities);
  if (expected_equalities == exact_equalities)
  {
    printf("Matrices are EQUAL\n");
  }
  else if (close_equalities == expected_equalities)
  {
    printf("Matrices are EQUAL with a tolerance of +/- 1 * MEDS_p\n");
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
}*/

int main(int argc, char *argv[])
{
  enable_cyclecounter();
  test_performance("pmod_mat_mul_2_k_k", 2, MEDS_k, MEDS_k, pmod_mat_mul_8_asm_2_k_k);
  test_performance("pmod_mat_mul_2_mn_k", 2, MEDS_m * MEDS_n, MEDS_k, pmod_mat_mul_8_asm_2_mn_k);
  test_performance("pmod_mat_mul_k_mn_k", MEDS_k, MEDS_m * MEDS_n, MEDS_k, pmod_mat_mul_8_asm_k_mn_k);
  test_performance("pmod_mat_mul_m_n_m", MEDS_m, MEDS_n, MEDS_m, pmod_mat_mul_8_asm_m_n_m);
  test_performance("pmod_mat_mul_m_n_n", MEDS_m, MEDS_n, MEDS_n, pmod_mat_mul_8_asm_m_n_n);
  test_performance("pmod_mat_mul_32_32_32", 32, 32, 32, pmod_mat_mul_8_asm_32_32_32);
  // test_performance2("pmod_mat_mul_24_24_24", 24, 24, 24, pmod_mat_mul_asm_24_24_24, pmod_mat_mul_8_asm_24_24_24);
  // test_performance2("pmod_mat_mul_8_8_8", 8, 8, 8, pmod_mat_mul_8_asm_8_8, pmod_mat_mul_8_asm_8_8_8);
  // test_performance2("pmod_mat_mul_13_8_8", 13, 8, 8, pmod_mat_mul_asm_13_8_8, pmod_mat_mul_8_asm_13_8_8);
  // test_performance2("pmod_mat_mul_8_8_13", 8, 8, 13, pmod_mat_mul_8_asm_8_13, pmod_mat_mul_8_asm_8_8_13);
  // test_performance2("pmod_mat_mul_13_13_13", 13, 13, 13, pmod_mat_mul_asm_13_13_13, pmod_mat_mul_8_asm_13_13_13);
  // test_performance2("pmod_mat_mul_8_13_8", 8, 13, 8, pmod_mat_mul_8_asm_13_8, pmod_mat_mul_8_asm_8_13_8);
  // test_performance2("pmod_mat_mul_13_13_13", 13, 13, 13, pmod_mat_mul_asm_13_13_13, pmod_mat_mul_8_asm_13_13_13);
  // test_performance2("pmod_mat_mul_16_16_16", 16, 16, 16, pmod_mat_mul_asm_16_16_16, pmod_mat_mul_8_asm_16_16_16);
  // test_performance2("pmod_mat_mul_24_24_24", 24, 24, 24, pmod_mat_mul_asm_24_24_24, pmod_mat_mul_8_asm_24_24_24);
  // test_performance2("pmod_mat_mul_2_k_k", 2, MEDS_k, MEDS_k, pmod_mat_mul_asm_2_k_k, pmod_mat_mul_8_asm_2_k_k);
  // test_performance2("pmod_mat_mul_2_mn_k", 2, MEDS_m * MEDS_n, MEDS_k, pmod_mat_mul_asm_2_mn_k, pmod_mat_mul_8_asm_2_mn_k);
  // test_performance2("pmod_mat_mul_k_mn_k", MEDS_k, MEDS_m * MEDS_n, MEDS_k, pmod_mat_mul_asm_k_mn_k, pmod_mat_mul_8_asm_k_mn_k);
  // test_performance2("pmod_mat_mul_m_n_m", MEDS_m, MEDS_n, MEDS_m, pmod_mat_mul_asm_m_n_m, pmod_mat_mul_8_asm_m_n_m);
  // test_performance2("pmod_mat_mul_m_n_n", MEDS_m, MEDS_n, MEDS_n, pmod_mat_mul_asm_m_n_n, pmod_mat_mul_8_asm_m_n_n);
  // test_performance2("pmod_mat_mul_24_24_24", 24, 24, 24, pmod_mat_mul_asm_24_24_24, pmod_mat_mul_8_asm_24_24_24);
  disable_cyclecounter();
}