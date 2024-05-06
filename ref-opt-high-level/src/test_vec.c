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

#define ROUNDS 100

#define A_ROWS 24
#define A_COLS 24*24
#define B_ROWS A_COLS
#define B_COLS 24
#define C_ROWS A_ROWS
#define C_COLS B_COLS

float median_2(long long arr[], int n)
{
  long long differences[n - 1];
  for (int i = 0; i < n - 1; i++)
    differences[i] = arr[i + 1] - arr[i];
  qsort(differences, n - 1, sizeof(long long), compare);
  if (n % 2 == 0)
    return (differences[n / 2 - 1] + differences[n / 2]) / 2.0;
  else
    return differences[n / 2];
}

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

int compare_matrices(pmod_mat_t *A1, pmod_mat_vec_t *A2, int cols, int rows)
{
  int equalities = 0;
  int inequalities = 0;
  for (int r = 0; r < rows; r++)
    for (int c = 0; c < cols; c++)
    {
      uint16_t correct_val = pmod_mat_entry(A1, rows, cols, r, c);
      uint16x4_t test_val = pmod_mat_entry(A2, rows, cols, r, c);

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

int test_reduce()
{
  printf("----------------------------------------\n");
  printf("Testing vectorized reduction\n");
  printf("----------------------------------------\n");

  uint32_t val = 2 << 30;
  GFq_vec_w_t val_vec = {val, val, val, val};

  GFq_t res;
  GFq_vec_t res_vec;

  // Measurement arrays
  long long old_cycles[ROUNDS];
  long long new_cycles[ROUNDS];

  for (int i = 0; i < ROUNDS - 1; i++)
  {
    old_cycles[i] = get_cyclecounter();
    res = REDUCE(val);
  }
  old_cycles[ROUNDS - 1] = get_cyclecounter();
  double old_median = median_2(old_cycles, ROUNDS);
  printf("Median cycles (normal): %f\n", old_median);

  for (int i = 0; i < ROUNDS - 1; i++)
  {
    new_cycles[i] = get_cyclecounter();
    res_vec = FREEZE_REDUCE_VEC(val_vec);
  }
  new_cycles[ROUNDS - 1] = get_cyclecounter();
  double new_median = median_2(new_cycles, ROUNDS);
  printf("Median cycles (vectorized): %f (x%f %%)\n", new_median, new_median / old_median);

  if (res == res_vec[0] && res == res_vec[1] && res == res_vec[2] && res == res_vec[3])
  {
    printf("EQUAL!\n");
    return 1;
  }
  else
  {
    printf("NOT EQUAL!\n");
    return 0;
  }
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

  // Measurement arrays
  long long old_cycles[ROUNDS];
  long long new_cycles[ROUNDS];

  // Perform the normal matmul
  printf("Performing normal matrix multiplication\n");
  for (int i = 0; i < ROUNDS - 1; i++)
  {
    old_cycles[i] = get_cyclecounter();
    pmod_mat_mul(C1, C_ROWS, C_COLS, A, A_ROWS, A_COLS, B, B_ROWS, B_COLS);
  }
  old_cycles[ROUNDS - 1] = get_cyclecounter();
  double old_median = median_2(old_cycles, ROUNDS);
  printf("Median cycles (normal): %f\n", old_median);

  // Perform the vectorized matmul
  printf("Performing vectorized matrix multiplication\n");
  for (int i = 0; i < ROUNDS - 1; i++)
  {
    new_cycles[i] = get_cyclecounter();
    pmod_mat_mul_vec(C2, C_ROWS, C_COLS, A2, A_ROWS, A_COLS, B2, B_ROWS, B_COLS);
  }
  new_cycles[ROUNDS - 1] = get_cyclecounter();
  double new_median = median_2(new_cycles, ROUNDS);
  printf("Median cycles (vectorized): %f (x%f %%)\n", new_median, new_median / old_median);

  // Compare the results
  printf("Comparing the results\n");
  return compare_matrices(C1, C2, C_COLS, C_ROWS);
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

  GFq_t val = rnd_GF(&state);
  GFq_vec_t val_vec = {val, val, val, val};

  // Measurement arrays
  long long old_cycles[ROUNDS];
  long long new_cycles[ROUNDS];

  GFq_t inv_val;
  GFq_vec_t inv_val_vec;

  for (int i = 0; i < ROUNDS - 1; i++)
  {
    old_cycles[i] = get_cyclecounter();
    inv_val = GF_inv(val);
  }
  old_cycles[ROUNDS - 1] = get_cyclecounter();
  double old_median = median_2(old_cycles, ROUNDS);
  printf("Median cycles (normal): %f\n", old_median);

  for (int i = 0; i < ROUNDS - 1; i++)
  {
    new_cycles[i] = get_cyclecounter();
    inv_val_vec = GF_inv_vec(val_vec);
  }
  new_cycles[ROUNDS - 1] = get_cyclecounter();
  double new_median = median_2(new_cycles, ROUNDS);
  printf("Median cycles (vectorized): %f (x%f %%)\n", new_median, new_median / old_median);

  if (inv_val == inv_val_vec[0] && inv_val == inv_val_vec[1] && inv_val == inv_val_vec[2] && inv_val == inv_val_vec[3])
    equalities++;
  else
    inequalities++;

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

extern pmod_mat_s_vec_t pmod_mat_syst_test(pmod_mat_vec_t *M, int M_r, int M_c);

void pmod_mat_vec_t_print(pmod_mat_vec_t *M, int M_r, int M_c)
{
  for (int r = 0; r < M_r; r++)
  {
    for (int c = 0; c < M_c; c++)
    {
      printf("%d ", M[r * M_c + c][0]);
    }
    printf("\n");
  }
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

  int res;
  pmod_mat_s_vec_t res_vec;

  // Measurement arrays
  long long old_cycles[ROUNDS];
  long long new_cycles[ROUNDS];

  // Perform the normal pmod_mat_syst_ct
  int res_wrong = pmod_mat_syst_ct_partial_swap_backsub(A_wrong, A_ROWS, A_COLS, A_ROWS + maxr_change, swap, backsub);
  for (int i = 0; i < ROUNDS - 1; i++)
  {
    old_cycles[i] = get_cyclecounter();
    res = pmod_mat_syst_ct_partial_swap_backsub(A, A_ROWS, A_COLS, A_ROWS + maxr_change, swap, backsub);
  }
  old_cycles[ROUNDS - 1] = get_cyclecounter();
  double old_median = median_2(old_cycles, ROUNDS);
  printf("Median cycles (normal): %f\n", old_median);

  // Perform the vectorized pmod_mat_syst_ct
  for (int i = 0; i < ROUNDS - 1; i++)
  {
    new_cycles[i] = get_cyclecounter();
    res_vec = pmod_mat_syst_ct_partial_swap_backsub_vec(A2, A_ROWS, A_COLS, A_ROWS + maxr_change, swap, backsub);
    // res_vec = pmod_mat_syst_test(A2, A_ROWS, A_COLS);
  }
  new_cycles[ROUNDS - 1] = get_cyclecounter();
  double new_median = median_2(new_cycles, ROUNDS);
  printf("Median cycles (vectorized): %f (x%f %%)\n", new_median, new_median / old_median);

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
  return compare_matrices(A, A2, A_COLS, A_ROWS);
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

  int res;
  pmod_mat_s_vec_t res_vec;

  // Measurement arrays
  long long old_cycles[ROUNDS];
  long long new_cycles[ROUNDS];

  // Perform the normal solve
  printf("Performing normal solve\n");
  int res_wrong = solve_opt(A_wrong, B_inv_wrong, C_wrong);
  for (int i = 0; i < ROUNDS - 1; i++)
  {
    old_cycles[i] = get_cyclecounter();
    res = solve_opt(A, B_inv, C);
  }
  old_cycles[ROUNDS - 1] = get_cyclecounter();
  double old_median = median_2(old_cycles, ROUNDS);
  printf("Median cycles (normal): %f\n", old_median);

  // Perform the vectorized solve
  printf("Performing vectorized solve\n");
  for (int i = 0; i < ROUNDS - 1; i++)
  {
    new_cycles[i] = get_cyclecounter();
    res_vec = solve_vec(A_vec, B_inv_vec, C_vec);
  }
  new_cycles[ROUNDS - 1] = get_cyclecounter();
  double new_median = median_2(new_cycles, ROUNDS);
  printf("Median cycles (vectorized): %f (x%f %%)\n", new_median, new_median / old_median);

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
  int equal_A = compare_matrices(A, A_vec, MEDS_m, MEDS_m);

  printf("Comparing the results (matrix B_inv)\n");
  int equal_B_inv = compare_matrices(B_inv, B_inv_vec, MEDS_n, MEDS_n);

  return equal_A && equal_B_inv;
}

int test_pmod_mat_inv()
{
  printf("----------------------------------------\n");
  printf("Testing pmod_mat_inv\n");
  printf("----------------------------------------\n");

  keccak_state shake;
  create_keccak_seed(&shake, 50);

  pmod_mat_t A[MEDS_m * MEDS_m] __attribute__((aligned(16)));
  pmod_mat_vec_t A_vec[MEDS_m * MEDS_m] __attribute__((aligned(16)));
  pmod_mat_t A_inv[MEDS_m * MEDS_m] __attribute__((aligned(16)));
  pmod_mat_vec_t A_inv_vec[MEDS_m * MEDS_m] __attribute__((aligned(16)));
  pmod_mat_t A_wrong[MEDS_m * MEDS_m] __attribute__((aligned(16)));
  pmod_mat_t A_inv_wrong[MEDS_m * MEDS_m] __attribute__((aligned(16)));

  // Fill A with random values
  printf("Filling A with random values\n");
  generate_random_matrices(A, A_vec, MEDS_m, MEDS_m, &shake);
  generate_non_invertible_matrix(A_wrong, MEDS_m, MEDS_m, &shake);

  // Insert A_wrong into the last lane of A_vec
  for (int r = 0; r < MEDS_m; r++)
    for (int c = 0; c < MEDS_m; c++)
    {
      pmod_mat_vec_t val = pmod_mat_entry(A_vec, MEDS_m, MEDS_m, r, c);
      val[3] = pmod_mat_entry(A_wrong, MEDS_m, MEDS_m, r, c);
      pmod_mat_set_entry(A_vec, MEDS_m, MEDS_m, r, c, val);
    }

  int res;
  pmod_mat_s_vec_t res_vec;

  // Measurement arrays
  long long old_cycles[ROUNDS];
  long long new_cycles[ROUNDS];

  // Perform the normal inversion
  printf("Performing normal inversion\n");
  int res_wrong = pmod_mat_inv(A_inv_wrong, A_wrong, MEDS_m, MEDS_m);
  for (int i = 0; i < ROUNDS - 1; i++)
  {
    old_cycles[i] = get_cyclecounter();
    res = pmod_mat_inv(A_inv, A, MEDS_m, MEDS_m);
  }
  old_cycles[ROUNDS - 1] = get_cyclecounter();
  double old_median = median_2(old_cycles, ROUNDS);
  printf("Median cycles (normal): %f\n", old_median);

  // Perform the vectorized inversion
  printf("Performing vectorized inversion\n");
  for (int i = 0; i < ROUNDS - 1; i++)
  {
    new_cycles[i] = get_cyclecounter();
    res_vec = pmod_mat_inv_vec(A_inv_vec, A_vec, MEDS_m, MEDS_m);
  }
  new_cycles[ROUNDS - 1] = get_cyclecounter();
  double new_median = median_2(new_cycles, ROUNDS);
  printf("Median cycles (vectorized): %f (x%f %%)\n", new_median, new_median / old_median);

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
  return compare_matrices(A_inv, A_inv_vec, MEDS_m, MEDS_m);
}

int test_pi()
{
  printf("----------------------------------------\n");
  printf("Testing pi\n");
  printf("----------------------------------------\n");

  keccak_state shake;
  create_keccak_seed(&shake, 60);

  pmod_mat_t A[MEDS_m * MEDS_m] __attribute__((aligned(16)));
  pmod_mat_vec_t A_vec[MEDS_m * MEDS_m] __attribute__((aligned(16)));
  pmod_mat_t B[MEDS_n * MEDS_n] __attribute__((aligned(16)));
  pmod_mat_vec_t B_vec[MEDS_n * MEDS_n] __attribute__((aligned(16)));
  pmod_mat_t G[MEDS_k * MEDS_m * MEDS_n] __attribute__((aligned(16)));
  pmod_mat_vec_t G_vec[MEDS_k * MEDS_m * MEDS_n] __attribute__((aligned(16)));
  pmod_mat_t Gout[MEDS_k * MEDS_m * MEDS_n] __attribute__((aligned(16)));
  pmod_mat_vec_t Gout_vec[MEDS_k * MEDS_m * MEDS_n] __attribute__((aligned(16)));

  // Fill A, B and G with random values
  printf("Filling A, B and G with random values\n");
  generate_random_matrices(A, A_vec, MEDS_m, MEDS_m, &shake);
  generate_random_matrices(B, B_vec, MEDS_n, MEDS_n, &shake);
  generate_random_matrices(G, G_vec, MEDS_k * MEDS_m, MEDS_n, &shake);

  // Measurement arrays
  long long old_cycles[ROUNDS];
  long long new_cycles[ROUNDS];

  // Perform the normal pi
  printf("Performing normal pi\n");
  for (int i = 0; i < ROUNDS - 1; i++)
  {
    old_cycles[i] = get_cyclecounter();
    pi(Gout, A, B, G);
  }
  old_cycles[ROUNDS - 1] = get_cyclecounter();
  double old_median = median_2(old_cycles, ROUNDS);
  printf("Median cycles (normal): %f\n", old_median);

  // Perform the vectorized pi
  printf("Performing vectorized pi\n");
  for (int i = 0; i < ROUNDS - 1; i++)
  {
    new_cycles[i] = get_cyclecounter();
    pi_vec(Gout_vec, A_vec, B_vec, G_vec);
  }
  new_cycles[ROUNDS - 1] = get_cyclecounter();
  double new_median = median_2(new_cycles, ROUNDS);
  printf("Median cycles (vectorized): %f (x%f %%)\n", new_median, new_median / old_median);

  // Compare the results
  return compare_matrices(Gout, Gout_vec, MEDS_m * MEDS_n, MEDS_k);
}

int test_SF()
{
  printf("----------------------------------------\n");
  printf("Testing SF\n");
  printf("----------------------------------------\n");

  keccak_state shake;
  create_keccak_seed(&shake, 70);

  pmod_mat_t G[MEDS_k * MEDS_m * MEDS_n] __attribute__((aligned(16)));
  pmod_mat_t G_wrong[MEDS_k * MEDS_m * MEDS_n] __attribute__((aligned(16)));
  pmod_mat_vec_t G_vec[MEDS_k * MEDS_m * MEDS_n] __attribute__((aligned(16)));

  // Fill G with random values
  printf("Filling G with random values\n");
  generate_random_matrices(G, G_vec, MEDS_k * MEDS_m, MEDS_n, &shake);
  generate_non_invertible_matrix(G_wrong, MEDS_k * MEDS_m, MEDS_n, &shake);

  // Insert G_wrong into the last lane of G_vec
  for (int r = 0; r < MEDS_k * MEDS_m; r++)
    for (int c = 0; c < MEDS_n; c++)
    {
      pmod_mat_vec_t val = pmod_mat_entry(G_vec, MEDS_k * MEDS_m, MEDS_n, r, c);
      val[3] = pmod_mat_entry(G_wrong, MEDS_k * MEDS_m, MEDS_n, r, c);
      pmod_mat_set_entry(G_vec, MEDS_k * MEDS_m, MEDS_n, r, c, val);
    }
  
  int res;
  pmod_mat_s_vec_t res_vec;

  // Measurement arrays
  long long old_cycles[ROUNDS];
  long long new_cycles[ROUNDS];

  // Perform the normal SF
  printf("Performing normal SF\n");
  int res_wrong = SF(G_wrong, G_wrong);
  for (int i = 0; i < ROUNDS - 1; i++)
  {
    old_cycles[i] = get_cyclecounter();
    res = SF(G, G);
  }
  old_cycles[ROUNDS - 1] = get_cyclecounter();
  double old_median = median_2(old_cycles, ROUNDS);
  printf("Median cycles (normal): %f\n", old_median);

  // Perform the vectorized SF
  printf("Performing vectorized SF\n");
  for (int i = 0; i < ROUNDS - 1; i++)
  {
    new_cycles[i] = get_cyclecounter();
    res_vec = SF_vec(G_vec, G_vec);
  }
  new_cycles[ROUNDS - 1] = get_cyclecounter();
  double new_median = median_2(new_cycles, ROUNDS);
  printf("Median cycles (vectorized): %f (x%f %%)\n", new_median, new_median / old_median);

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
  return compare_matrices(G, G_vec, MEDS_m * MEDS_n, MEDS_k);
}

int main(int argc, char *argv[])
{
  enable_cyclecounter();

  int test_count = 14;
  int passed = 0;
  passed += test_cmov();
  passed += test_cswap();
  passed += test_reduce();
  passed += test_matmul();
  passed += test_GF_inv();
  passed += test_pmod_mat_syst_ct(0, 1, 1);
  passed += test_pmod_mat_syst_ct(0, 0, 1);
  passed += test_pmod_mat_syst_ct(-1, 0, 1);
  passed += test_pmod_mat_syst_ct(-1, 0, 0);
  passed += test_pmod_mat_syst_ct(0, 0, 0);
  passed += test_solve();
  passed += test_pmod_mat_inv();
  passed += test_pi();
  passed += test_SF();

  printf("----------------------------------------\n\n");
  printf("Passed %d out of %d tests\n", passed, test_count);
  if (passed == test_count)
    printf("SUCCESS!\n");
  else
    printf("FAILED!\n");

  disable_cyclecounter();
  return 0;
}
