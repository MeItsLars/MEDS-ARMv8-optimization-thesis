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

void pmod_mat_mul_2(pmod_mat_t *C, int C_r, int C_c, pmod_mat_t *A, int A_r, int A_c, pmod_mat_t *B, int B_r, int B_c)
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
    pmod_mat_mul_2(C2, MEDS_k, MEDS_k, A, MEDS_k, MEDS_m * MEDS_n, B, MEDS_m * MEDS_n, MEDS_k);
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
  int eq = 1;
  for (int i = 0; i < MEDS_k; i++)
    for (int j = 0; j < MEDS_k; j++)
      if (pmod_mat_entry(C1, MEDS_k, MEDS_k, i, j) != pmod_mat_entry(C2, MEDS_k, MEDS_k, i, j))
      {
        eq = 0;
        break;
      }

  if (eq)
    printf("Matrices are equal\n");
  else
  {
    printf("Matrices are NOT EQUAL\n");
    exit(-1);
  }


  return 0;
}