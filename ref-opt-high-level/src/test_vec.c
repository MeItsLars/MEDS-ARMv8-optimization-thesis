#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "randombytes.h"

#include "cyclecounter.h"
#include "params.h"
#include "api.h"
#include "fips202.h"
#include "util.h"
#include "util_vec.h"
#include "meds.h"
#include "matrixmod.h"
#include "matrixmod_vec.h"

#define A_ROWS 24
#define A_COLS 24 * 24
#define B_ROWS A_COLS
#define B_COLS 24
#define C_ROWS A_ROWS
#define C_COLS B_COLS

void create_keccak_seed(keccak_state *shake, uint8_t input_seed)
{
  uint8_t seed[MEDS_pub_seed_bytes];

  for (int i = 0; i < MEDS_pub_seed_bytes; i++)
  {
    seed[i] = input_seed;
  }

  shake256_absorb_once(shake, seed, MEDS_pub_seed_bytes);
}

void generate_random_matrices(pmod_mat_t *A, pmod_mat_vec_t *A_vec, int rows, int cols, keccak_state *shake)
{
  for (int r = 0; r < rows; r++)
    for (int c = 0; c < cols; c++)
    {
      uint16_t val = rnd_GF(shake);
      uint16x4_t val_vec = {val, val, val, val};
      pmod_mat_set_entry(A, rows, cols, r, c, val);
      pmod_mat_set_entry(A_vec, rows, cols, r, c, val_vec);
    }
}

void generate_non_invertible_matrix(pmod_mat_t *A, int rows, int cols, keccak_state *shake)
{
  for (int r = 0; r < rows; r++)
    for (int c = 0; c < cols; c++)
    {
      uint16_t val = rnd_GF(shake);
      pmod_mat_set_entry(A, rows, cols, r, c, val);
    }

  // Make the first and last row all zeros
  for (int c = 0; c < cols; c++)
  {
    pmod_mat_set_entry(A, rows, cols, 0, c, 0);
    pmod_mat_set_entry(A, rows, cols, rows - 1, c, 0);
  }
}

int test_cmov()
{
  printf("----------------------------------------\n");
  printf("Testing vectorized conditional move\n");
  printf("----------------------------------------\n");

  pmod_mat_s_vec_t a = {1, 2, 3, 4};
  pmod_mat_s_vec_t b = {5, 6, 7, 8};
  pmod_mat_s_vec_t cond_0 = {0, 0, 0, 0};
  pmod_mat_s_vec_t cond_1 = {65535, 65535, 65535, 65535};

  pmod_mat_s_vec_cmov(&a, &b, cond_0);
  int success_1 = a[0] == 1 && a[1] == 2 && a[2] == 3 && a[3] == 4 && b[0] == 5 && b[1] == 6 && b[2] == 7 && b[3] == 8;
  printf("cmov (false): success = %d\n", success_1);

  pmod_mat_s_vec_cmov(&a, &b, cond_1);
  int success_2 = a[0] == 5 && a[1] == 6 && a[2] == 7 && a[3] == 8 && b[0] == 5 && b[1] == 6 && b[2] == 7 && b[3] == 8;
  printf("cmov (true): success = %d\n", success_2);

  return success_1 && success_2;
}

int test_cswap()
{
  printf("----------------------------------------\n");
  printf("Testing vectorized conditional swap\n");
  printf("----------------------------------------\n");

  pmod_mat_vec_t a = {1, 2, 3, 4};
  pmod_mat_vec_t b = {5, 6, 7, 8};
  pmod_mat_s_vec_t cond_0 = {0, 0, 0, 0};
  pmod_mat_s_vec_t cond_1 = {65535, 65535, 65535, 65535};

  pmod_mat_vec_cswap_s_cond(&a, &b, cond_0);
  int success_1 = a[0] == 1 && a[1] == 2 && a[2] == 3 && a[3] == 4 && b[0] == 5 && b[1] == 6 && b[2] == 7 && b[3] == 8;
  printf("cswap (false): success = %d\n", success_1);

  pmod_mat_vec_cswap_s_cond(&a, &b, cond_1);
  int success_2 = a[0] == 5 && a[1] == 6 && a[2] == 7 && a[3] == 8 && b[0] == 1 && b[1] == 2 && b[2] == 3 && b[3] == 4;
  printf("cswap (true): success = %d\n", success_2);

  return success_1 && success_2;
}

int test_matmul()
{
  printf("----------------------------------------\n");
  printf("Testing vectorized matrix multiplication\n");
  printf("----------------------------------------\n");

  keccak_state shake;
  create_keccak_seed(&shake, 10);

  pmod_mat_t A[A_ROWS * A_COLS] __attribute__((aligned(16)));
  pmod_mat_t B[B_ROWS * B_COLS] __attribute__((aligned(16)));
  pmod_mat_vec_t A2[A_ROWS * A_COLS] __attribute__((aligned(16)));
  pmod_mat_vec_t B2[B_ROWS * B_COLS] __attribute__((aligned(16)));
  pmod_mat_t C1[C_ROWS * C_COLS] __attribute__((aligned(16)));
  pmod_mat_vec_t C2[C_ROWS * C_COLS] __attribute__((aligned(16)));

  // Fill A and B with random values
  printf("Filling A and B with random values\n");
  generate_random_matrices(A, A2, A_ROWS, A_COLS, &shake);
  generate_random_matrices(B, B2, B_ROWS, B_COLS, &shake);

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
  return inequalities == 0;
}

int test_GF_inv()
{
  printf("----------------------------------------\n");
  printf("Testing finite field inversion\n");
  printf("----------------------------------------\n");

  int equalities = 0;
  int inequalities = 0;
  keccak_state state;
  create_keccak_seed(&state, 20);
  for (int i = 0; i < 100; i++)
  {
    GFq_t val = rnd_GF(&state);
    GFq_vec_t val_vec = {val, val, val, val};
    GFq_t inv_val = GF_inv(val);
    GFq_vec_t inv_val_vec = GF_inv_vec(val_vec);
    if (inv_val == inv_val_vec[0] && inv_val == inv_val_vec[1] && inv_val == inv_val_vec[2] && inv_val == inv_val_vec[3])
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
  return inequalities == 0;
}

int test_pmod_mat_syst_ct(int maxr_change, int swap, int backsub)
{
  printf("----------------------------------------\n");
  printf("Testing pmod_mat_syst_ct (maxr_change = %d, swap = %d, backsub = %d)\n", maxr_change, swap, backsub);
  printf("----------------------------------------\n");

  keccak_state shake;
  create_keccak_seed(&shake, 30);

  pmod_mat_t A[A_ROWS * A_COLS] __attribute__((aligned(16)));
  pmod_mat_vec_t A2[A_ROWS * A_COLS] __attribute__((aligned(16)));
  pmod_mat_t A_wrong[A_ROWS * A_COLS] __attribute__((aligned(16)));

  // Fill A with random values
  printf("Filling A and A_wrong with random values\n");
  generate_random_matrices(A, A2, A_ROWS, A_COLS, &shake);
  generate_non_invertible_matrix(A_wrong, A_ROWS, A_COLS, &shake);

  // Insert A_wrong into the last lane of A2
  for (int r = 0; r < A_ROWS; r++)
    for (int c = 0; c < A_COLS; c++)
    {
      pmod_mat_vec_t val = pmod_mat_entry(A2, A_ROWS, A_COLS, r, c);
      val[3] = pmod_mat_entry(A_wrong, A_ROWS, A_COLS, r, c);
      pmod_mat_set_entry(A2, A_ROWS, A_COLS, r, c, val);
    }

  // Perform the normal pmod_mat_syst_ct
  int res = pmod_mat_syst_ct_partial_swap_backsub(A, A_ROWS, A_COLS, A_ROWS + maxr_change, swap, backsub);
  int res_wrong = pmod_mat_syst_ct_partial_swap_backsub(A_wrong, A_ROWS, A_COLS, A_ROWS + maxr_change, swap, backsub);

  // Perform the vectorized pmod_mat_syst_ct
  pmod_mat_s_vec_t res_vec = pmod_mat_syst_ct_partial_swap_backsub_vec(A2, A_ROWS, A_COLS, A_ROWS + maxr_change, swap, backsub);

  printf("Normal result: %d\n", res);
  printf("Vectorized result: %d, %d, %d\n", res_vec[0], res_vec[1], res_vec[2]);
  if (res != res_vec[0] || res != res_vec[1] || res != res_vec[2])
  {
    printf("NOT EQUAL!\n");
    return 0;
  }

  if (res == -1)
  {
    printf("NOT INVERTIBLE!\n");
    return 1;
  }

  printf("Normal result (wrong): %d\n", res_wrong);
  printf("Vectorized result (wrong): %d\n", res_vec[3]);
  if (res_wrong != res_vec[3])
  {
    printf("NOT EQUAL!\n");
    return 0;
  }

  // Compare the results
  printf("Comparing the results\n");
  int equalities = 0;
  int inequalities = 0;
  for (int r = 0; r < A_ROWS; r++)
    for (int c = 0; c < A_COLS; c++)
    {
      uint16_t correct_val = pmod_mat_entry(A, A_ROWS, A_COLS, r, c);
      uint16x4_t test_val = pmod_mat_entry(A2, A_ROWS, A_COLS, r, c);

      if (test_val[0] == correct_val && test_val[1] == correct_val && test_val[2] == correct_val)
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
  return inequalities == 0;
}

int test_solve()
{
  printf("----------------------------------------\n");
  printf("Testing pmod_mat_solve\n");
  printf("----------------------------------------\n");

  keccak_state shake;
  create_keccak_seed(&shake, 40);

  pmod_mat_t C[2 * MEDS_m * MEDS_n] __attribute__((aligned(16)));
  pmod_mat_vec_t C_vec[2 * MEDS_m * MEDS_n] __attribute__((aligned(16)));
  pmod_mat_t C_wrong[2 * MEDS_m * MEDS_n] __attribute__((aligned(16)));

  pmod_mat_t A[MEDS_m * MEDS_m] __attribute__((aligned(16)));
  pmod_mat_vec_t A_vec[MEDS_m * MEDS_m] __attribute__((aligned(16)));
  pmod_mat_t B_inv[MEDS_n * MEDS_n] __attribute__((aligned(16)));
  pmod_mat_vec_t B_inv_vec[MEDS_n * MEDS_n] __attribute__((aligned(16)));
  pmod_mat_t A_wrong[MEDS_m * MEDS_m] __attribute__((aligned(16)));
  pmod_mat_t B_inv_wrong[MEDS_n * MEDS_n] __attribute__((aligned(16)));

  // Fill C with random values
  printf("Filling C with random values\n");
  generate_random_matrices(C, C_vec, 2 * MEDS_m, MEDS_n, &shake);
  generate_non_invertible_matrix(C_wrong, 2 * MEDS_m, MEDS_n, &shake);

  // Insert C_wrong into the last lane of C_vec
  for (int r = 0; r < 2 * MEDS_m; r++)
    for (int c = 0; c < MEDS_n; c++)
    {
      pmod_mat_vec_t val = pmod_mat_entry(C_vec, 2 * MEDS_m, MEDS_n, r, c);
      val[3] = pmod_mat_entry(C_wrong, 2 * MEDS_m, MEDS_n, r, c);
      pmod_mat_set_entry(C_vec, 2 * MEDS_m, MEDS_n, r, c, val);
    }

  // Perform the normal solve
  printf("Performing normal solve\n");
  int res = solve_opt(A, B_inv, C);
  int res_wrong = solve_opt(A_wrong, B_inv_wrong, C_wrong);

  // Perform the vectorized solve
  printf("Performing vectorized solve\n");
  pmod_mat_s_vec_t res_vec = solve_vec(A_vec, B_inv_vec, C_vec);

  printf("Normal result: %d\n", res);
  printf("Vectorized result: %d, %d, %d\n", res_vec[0], res_vec[1], res_vec[2]);
  if (res != res_vec[0] || res != res_vec[1] || res != res_vec[2])
  {
    printf("NOT EQUAL!\n");
    return 0;
  }

  if (res == -1)
  {
    printf("NOT INVERTIBLE!\n");
    return 1;
  }

  printf("Normal result (wrong): %d\n", res_wrong);
  printf("Vectorized result (wrong): %d\n", res_vec[3]);
  if (res_wrong != res_vec[3])
  {
    printf("NOT EQUAL!\n");
    return 0;
  }

  // Compare the results
  printf("Comparing the results (matrix A)\n");
  int equalities_A = 0;
  int inequalities_A = 0;
  for (int r = 0; r < MEDS_m; r++)
    for (int c = 0; c < MEDS_m; c++)
    {
      uint16_t correct_val = pmod_mat_entry(A, MEDS_m, MEDS_m, r, c);
      uint16x4_t test_val = pmod_mat_entry(A_vec, MEDS_m, MEDS_m, r, c);

      if (test_val[0] == correct_val && test_val[1] == correct_val && test_val[2] == correct_val)
        equalities_A++;
      else
        inequalities_A++;
    }

  printf("Equalities: %d\n", equalities_A);
  printf("Inequalities: %d\n", inequalities_A);

  printf("Comparing the results (matrix B_inv)\n");
  int equalities_B_inv = 0;
  int inequalities_B_inv = 0;
  for (int r = 0; r < MEDS_n; r++)
    for (int c = 0; c < MEDS_n; c++)
    {
      uint16_t correct_val = pmod_mat_entry(B_inv, MEDS_n, MEDS_n, r, c);
      uint16x4_t test_val = pmod_mat_entry(B_inv_vec, MEDS_n, MEDS_n, r, c);

      if (test_val[0] == correct_val && test_val[1] == correct_val && test_val[2] == correct_val)
        equalities_B_inv++;
      else
        inequalities_B_inv++;
    }

  printf("Equalities: %d\n", equalities_B_inv);
  printf("Inequalities: %d\n", inequalities_B_inv);

  if (inequalities_A == 0 && inequalities_B_inv == 0)
  {
    printf("EQUAL!\n");
  }
  else
  {
    printf("NOT EQUAL!\n");
  }

  return inequalities_A == 0 && inequalities_B_inv == 0;
}

int main(int argc, char *argv[])
{
  int test_count = 10;
  int passed = 0;
  passed += test_cmov();
  passed += test_cswap();
  passed += test_matmul();
  passed += test_GF_inv();
  passed += test_pmod_mat_syst_ct(0, 1, 1);
  passed += test_pmod_mat_syst_ct(0, 0, 1);
  passed += test_pmod_mat_syst_ct(-1, 0, 1);
  passed += test_pmod_mat_syst_ct(-1, 0, 0);
  passed += test_pmod_mat_syst_ct(0, 0, 0);
  passed += test_solve();

  printf("----------------------------------------\n\n");
  printf("Passed %d out of %d tests\n", passed, test_count);
  if (passed == test_count)
    printf("SUCCESS!\n");
  else
    printf("FAILED!\n");

  return 0;
}
