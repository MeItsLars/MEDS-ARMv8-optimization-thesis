#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "randombytes.h"

#include "cyclecounter.h"
#include "params.h"
#include "api.h"
#include "fips202.h"
#include "util.h"
#include "meds.h"
#include "matrixmod.h"
#include "matrixmod_vec.h"

#define A_ROWS 24
#define A_COLS 24 * 24
#define B_ROWS A_COLS
#define B_COLS 24
#define C_ROWS A_ROWS
#define C_COLS B_COLS

int test_matmul()
{
  printf("----------------------------------------\n");
  printf("Testing vectorized matrix multiplication\n");
  printf("----------------------------------------\n");

  uint8_t seed[MEDS_pub_seed_bytes];

  for (int i = 0; i < MEDS_pub_seed_bytes; i++)
  {
    seed[i] = i;
  }

  pmod_mat_t A[A_ROWS * A_COLS] __attribute__((aligned(16)));
  pmod_mat_t B[B_ROWS * B_COLS] __attribute__((aligned(16)));
  pmod_mat_vec_t A2[A_ROWS * A_COLS] __attribute__((aligned(16)));
  pmod_mat_vec_t B2[B_ROWS * B_COLS] __attribute__((aligned(16)));
  pmod_mat_t C1[C_ROWS * C_COLS] __attribute__((aligned(16)));
  pmod_mat_vec_t C2[C_ROWS * C_COLS] __attribute__((aligned(16)));

  keccak_state shake;
  shake256_absorb_once(&shake, seed, MEDS_pub_seed_bytes);

  // Fill A and B with random values
  printf("Filling A and B with random values\n");
  for (int r = 0; r < A_ROWS; r++)
    for (int c = 0; c < A_COLS; c++)
    {
      uint16_t val = rnd_GF(&shake);
      uint16x4_t val_vec = {val, val, val, val};
      pmod_mat_set_entry(A, A_ROWS, A_COLS, r, c, val);
      pmod_mat_set_entry(A2, A_ROWS, A_COLS, r, c, val_vec);
    }

  for (int r = 0; r < B_ROWS; r++)
    for (int c = 0; c < B_COLS; c++)
    {
      uint16_t val = rnd_GF(&shake);
      uint16x4_t val_vec = {val, val, val, val};
      pmod_mat_set_entry(B, B_ROWS, B_COLS, r, c, val);
      pmod_mat_set_entry(B2, B_ROWS, B_COLS, r, c, val_vec);
    }

  // Perform the normal matmul
  printf("Performing normal matrix multiplication\n");
  pmod_mat_mul(C1, C_ROWS, C_COLS, A, A_ROWS, A_COLS, B, B_ROWS, B_COLS);

  // Perform the vectorized matmul
  printf("Performing vectorized matrix multiplication\n");
  pmod_mat_mul_vec(C2, C_ROWS, C_COLS, A2, A_ROWS, A_COLS, B2, B_ROWS, B_COLS);

  // Compare the results
  printf("Comparing the results\n");
  int equalities = 0;
  int inequalities = 0;
  for (int r = 0; r < C_ROWS; r++)
    for (int c = 0; c < C_COLS; c++)
    {
      uint16_t correct_val = pmod_mat_entry(C1, C_ROWS, C_COLS, r, c);
      uint16x4_t test_val = pmod_mat_entry(C2, C_ROWS, C_COLS, r, c);

      if (test_val[0] == correct_val && test_val[1] == correct_val && test_val[2] == correct_val && test_val[3] == correct_val)
        equalities++;
      else
        inequalities++;
    }
  
  printf("Equalities: %d\n", equalities);
  printf("Inequalities: %d\n", inequalities);
  if (inequalities == 0)
  {
    printf("EQUAL!\n");
  }
  else
  {
    printf("NOT EQUAL!\n");
  }
  printf("----------------------------------------\n");
  return inequalities == 0;
}

int main(int argc, char *argv[])
{
  int passed = 0;
  passed += test_matmul();

  printf("Passed %d out of 1 tests\n", passed);

  return 0;
}
