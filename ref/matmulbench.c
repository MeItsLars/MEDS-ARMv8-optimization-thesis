#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <sys/random.h>
#include <sys/time.h>

#include "fips202.h"
#include "params.h"
#include "api.h"
#include "meds.h"
#include "matrixmod.h"
#include "benchresult.h"

benchresult benchresults[1000];
int number_of_benchresults = 0;
int benchmark_enabled = 0;

#define MATMUL_ROUNDS 1000

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

uint32_t montgomery_redc(uint32_t t)
{
  // m = ((t mod R) * N') mod R (mod R is implemented as a mask)
  uint32_t m = ((t & MG_Rmask) * MG_Nprime) & MG_Rmask;
  // res = (t + m * N) / R (/ R is implemented as a bitshift)
  uint32_t res = (t + m * MEDS_p) >> MG_Rbits;
  // if res >= MEDS_p, return res - MEDS_p, else return res (constant time)
  int32_t diff = res - MEDS_p;
  int32_t mask = (diff >> 31) & 0x1;
  return mask * res + (1 - mask) * diff;
  ;
}

uint32_t montgomery_mul(uint32_t a, uint32_t b)
{
  return montgomery_redc(a * b);
}

uint32_t montgomery_add(uint32_t a, uint32_t b)
{
  uint32_t t = a + b;
  // if t >= MEDS_p, return t - MEDS_p, else return t (constant time)
  int32_t diff = t - MEDS_p;
  int32_t mask = (diff >> 31) & 0x1;
  return mask * t + (1 - mask) * diff;
}

uint32_t montgomery_to(GFq_t a)
{
  return (((uint32_t)a) << MG_Rbits) % MEDS_p;
}

GFq_t montgomery_from(uint32_t a)
{
  return (a * MG_Rinv) % MEDS_p;
}

void pmod_mat_mul_2(pmod_mat_t *C, int C_r, int C_c, pmod_mat_t *A, int A_r, int A_c, pmod_mat_t *B, int B_r, int B_c)
{
  uint32_t tmp[C_r * C_c];

  // Convert to Montgomery domain
  for (int r = 0; r < A_r; r++)
    for (int c = 0; c < A_c; c++)
      pmod_mat_set_entry(A, A_r, A_c, r, c, montgomery_to(pmod_mat_entry(A, A_r, A_c, r, c)));
  for (int r = 0; r < B_r; r++)
    for (int c = 0; c < B_c; c++)
      pmod_mat_set_entry(B, B_r, B_c, r, c, montgomery_to(pmod_mat_entry(B, B_r, B_c, r, c)));

  // Multiply
  for (int c = 0; c < C_c; c++)
    for (int r = 0; r < C_r; r++)
    {
      uint32_t val = 0;
      for (int i = 0; i < A_c; i++)
      {
        val = montgomery_add(val, montgomery_mul(pmod_mat_entry(A, A_r, A_c, r, i), pmod_mat_entry(B, B_r, B_c, i, c)));
      }
      tmp[r * C_c + c] = val;
    }

  // Convert back from Montgomery domain
  for (int c = 0; c < C_c; c++)
    for (int r = 0; r < C_r; r++)
      pmod_mat_set_entry(C, C_r, C_c, r, c, montgomery_from(tmp[r * C_c + c]));
}

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

void pmod_mat_mul_3(pmod_mat_t *C, int C_r, int C_c, pmod_mat_t *A, int A_r, int A_c, pmod_mat_t *B, int B_r, int B_c)
{
  uint32_t tmp[C_r * C_c];

  // Multiply
  for (int c = 0; c < C_c; c++)
    for (int r = 0; r < C_r; r++)
    {
      uint32_t val = 0;
      for (int i = 0; i < A_c; i++)
      {
        val += pmod_mat_entry(A, A_r, A_c, r, i) * (uint32_t)pmod_mat_entry(B, B_r, B_c, i, c);
      }
      tmp[r * C_c + c] = val;
    }

  for (int c = 0; c < C_c; c++)
    for (int r = 0; r < C_r; r++)
      pmod_mat_set_entry(C, C_r, C_c, r, c, mod_reduce(tmp[r * C_c + c]));
}

int main(int argc, char *argv[])
{
  uint8_t seed1[MEDS_pub_seed_bytes];

  for (int i = 0; i < MEDS_pub_seed_bytes; i++)
  {
    seed1[i] = i;
  }

  pmod_mat_t A[MEDS_k * MEDS_m * MEDS_n];
  pmod_mat_t B[MEDS_m * MEDS_n * MEDS_k];
  pmod_mat_t C1[MEDS_k * MEDS_k];
  pmod_mat_t C2[MEDS_k * MEDS_k];

  keccak_state shake;
  shake256_absorb_once(&shake, seed1, MEDS_pub_seed_bytes);

  long long old_matmul_cycles[MATMUL_ROUNDS];
  long long new_matmul_cycles[MATMUL_ROUNDS];

  for (int round = 0; round < MATMUL_ROUNDS; round++)
  {
    // Fill matrices with random values
    for (int r = 0; r < MEDS_k; r++)
      for (int c = 0; c < MEDS_m * MEDS_n; c++)
      {
        pmod_mat_set_entry(A, MEDS_k, MEDS_m * MEDS_n, r, c, rnd_GF(&shake));
      }

    for (int r = 0; r < MEDS_m * MEDS_n; r++)
      for (int c = 0; c < MEDS_k; c++)
      {
        pmod_mat_set_entry(B, MEDS_m * MEDS_n, MEDS_k, r, c, rnd_GF(&shake));
      }

    long long old_matmul_cc = -cpucycles();
    pmod_mat_mul_1(C1, MEDS_k, MEDS_k, A, MEDS_k, MEDS_m * MEDS_n, B, MEDS_m * MEDS_n, MEDS_k);
    old_matmul_cc += cpucycles();
    long long new_matmul_cc = -cpucycles();
    pmod_mat_mul_3(C2, MEDS_k, MEDS_k, A, MEDS_k, MEDS_m * MEDS_n, B, MEDS_m * MEDS_n, MEDS_k);
    new_matmul_cc += cpucycles();

    old_matmul_cycles[round] = old_matmul_cc;
    new_matmul_cycles[round] = new_matmul_cc;
  }

  // Print results
  double old_matmul_median_cc = median(old_matmul_cycles, MATMUL_ROUNDS);
  double new_matmul_median_cc = median(new_matmul_cycles, MATMUL_ROUNDS);
  double percentage = new_matmul_median_cc / old_matmul_median_cc * 100;
  double improvement = (new_matmul_median_cc - old_matmul_median_cc) / old_matmul_median_cc * 100;
  printf("Old median: %f\n", old_matmul_median_cc);
  printf("New median: %f\n", new_matmul_median_cc);
  printf("Percentage: %f%%\n", percentage);
  printf("Improvement: %f%%\n", improvement);

  // Compare matrices
  int equalities = 0;
  for (int i = 0; i < MEDS_k; i++)
    for (int j = 0; j < MEDS_k; j++)
      if (pmod_mat_entry(C1, MEDS_k, MEDS_k, i, j) == pmod_mat_entry(C2, MEDS_k, MEDS_k, i, j))
      {
        equalities++;
      }

  int expected_equalities = MEDS_k * MEDS_k;

  if (expected_equalities == equalities)
    printf("Matrices are EQUAL\n");
  else
  {
    printf("Equalities: %d / %d\n", equalities, expected_equalities);
    printf("Matrices are NOT EQUAL\n");
    exit(-1);
  }

  return 0;
}