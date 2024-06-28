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

#define ROUNDS 128

int solve_opt_part(pmod_mat_t *N, GFq_t *sol, pmod_mat_t *P00nt)
{
  _Static_assert(MEDS_n == MEDS_m + 1, "solve_opt requires MEDS_n == MEDS_m+1");

  for (int b = MEDS_m - 3; b >= 0; b--)
    for (int c = MEDS_m - 1; c >= 0; c--)
    {
      uint16x8_t tmp2_16x8 = vdupq_n_u16(sol[(MEDS_m + 1) * MEDS_n + b * MEDS_m + MEDS_m + c]);
      uint16x4_t tmp2_16x4 = vdup_n_u16(sol[(MEDS_m + 1) * MEDS_n + b * MEDS_m + MEDS_m + c]);
      uint16_t tmp2 = sol[(MEDS_m + 1) * MEDS_n + b * MEDS_m + MEDS_m + c];
      int r = 0;
      for (; r < MEDS_m - 8; r += 8)
      {
        // uint64_t tmp1 = pmod_mat_entry(N, MEDS_n - 1, MEDS_m, r, c);
        uint16_t tmp1_0 = pmod_mat_entry(N, MEDS_n - 1, MEDS_m, r, c);
        uint16_t tmp1_1 = pmod_mat_entry(N, MEDS_n - 1, MEDS_m, r + 1, c);
        uint16_t tmp1_2 = pmod_mat_entry(N, MEDS_n - 1, MEDS_m, r + 2, c);
        uint16_t tmp1_3 = pmod_mat_entry(N, MEDS_n - 1, MEDS_m, r + 3, c);
        uint16_t tmp1_4 = pmod_mat_entry(N, MEDS_n - 1, MEDS_m, r + 4, c);
        uint16_t tmp1_5 = pmod_mat_entry(N, MEDS_n - 1, MEDS_m, r + 5, c);
        uint16_t tmp1_6 = pmod_mat_entry(N, MEDS_n - 1, MEDS_m, r + 6, c);
        uint16_t tmp1_7 = pmod_mat_entry(N, MEDS_n - 1, MEDS_m, r + 7, c);

        uint16x8_t tmp1 = {tmp1_0, tmp1_1, tmp1_2, tmp1_3, tmp1_4, tmp1_5, tmp1_6, tmp1_7};

        uint32x4_t prod_low = vmull_u16(vget_low_u16(tmp1), vget_low_u16(tmp2_16x8));
        uint32x4_t prod_high = vmull_high_u16(tmp1, tmp2_16x8);
        uint16x8_t prod_low_red = (uint16x8_t)FREEZE_REDUCE_VEC_16x4(prod_low);
        uint16x8_t prod_high_red = (uint16x8_t)FREEZE_REDUCE_VEC_16x4(prod_high);
        uint16x8_t prod = vuzp1q_u16(prod_low_red, prod_high_red);

        // uint64_t val = sol[(MEDS_m + 1) * MEDS_n + b * MEDS_m + r];
        uint16x8_t val = vld1q_u16((uint16_t *)&sol[(MEDS_m + 1) * MEDS_n + b * MEDS_m + r]);

        // val = ((MEDS_p + val) - prod) % MEDS_p;
        val = vsubq_u16(vaddq_u16(val, MEDS_p_VEC_16x8), prod);
        val = FREEZE_VEC_16x8(val);

        // sol[(MEDS_m + 1) * MEDS_n + b * MEDS_m + r] = val;
        vst1q_u16((uint16_t *)&sol[(MEDS_m + 1) * MEDS_n + b * MEDS_m + r], val);
      }
      for (; r < MEDS_m - 4; r += 4)
      {
        // uint64_t tmp1 = pmod_mat_entry(N, MEDS_n - 1, MEDS_m, r, c);
        uint16_t tmp1_0 = pmod_mat_entry(N, MEDS_n - 1, MEDS_m, r, c);
        uint16_t tmp1_1 = pmod_mat_entry(N, MEDS_n - 1, MEDS_m, r + 1, c);
        uint16_t tmp1_2 = pmod_mat_entry(N, MEDS_n - 1, MEDS_m, r + 2, c);
        uint16_t tmp1_3 = pmod_mat_entry(N, MEDS_n - 1, MEDS_m, r + 3, c);
        uint16x4_t tmp1 = {tmp1_0, tmp1_1, tmp1_2, tmp1_3};

        // uint64_t prod = (tmp1 * tmp2) % MEDS_p;
        uint32x4_t prod = vmull_u16(tmp1, tmp2_16x4);
        uint16x4_t prod_red = FREEZE_REDUCE_VEC_16x4_SHRINK(prod);

        // uint64_t val = sol[(MEDS_m + 1) * MEDS_n + b * MEDS_m + r];
        uint16x4_t val = vld1_u16((uint16_t *)&sol[(MEDS_m + 1) * MEDS_n + b * MEDS_m + r]);

        // val = ((MEDS_p + val) - prod) % MEDS_p;
        val = vsub_u16(vadd_u16(val, MEDS_p_VEC_16x4), prod_red);
        val = FREEZE_VEC_16x4(val);

        // sol[(MEDS_m + 1) * MEDS_n + b * MEDS_m + r] = val;
        vst1_u16((uint16_t *)&sol[(MEDS_m + 1) * MEDS_n + b * MEDS_m + r], val);
      }
      for (; r < MEDS_m; r++)
      {
        uint64_t tmp1 = pmod_mat_entry(N, MEDS_n - 1, MEDS_m, r, c);

        uint64_t prod = (tmp1 * tmp2) % MEDS_p;

        uint64_t val = sol[(MEDS_m + 1) * MEDS_n + b * MEDS_m + r];

        val = ((MEDS_p + val) - prod) % MEDS_p;

        sol[(MEDS_m + 1) * MEDS_n + b * MEDS_m + r] = val;
      }
    }

  for (int b = MEDS_m - 2; b >= 0; b--)
    for (int c = MEDS_m - 1; c >= 0; c--)
    {
      // uint64_t tmp2 = sol[2 * MEDS_m * MEDS_n - (MEDS_m - 1) - (MEDS_m - 1 - b) * MEDS_m + c];
      uint16x8_t tmp2_16x8 = vdupq_n_u16(sol[2 * MEDS_m * MEDS_n - (MEDS_m - 1) - (MEDS_m - 1 - b) * MEDS_m + c]);
      uint16x4_t tmp2_16x4 = vdup_n_u16(sol[2 * MEDS_m * MEDS_n - (MEDS_m - 1) - (MEDS_m - 1 - b) * MEDS_m + c]);
      uint16_t tmp2 = sol[2 * MEDS_m * MEDS_n - (MEDS_m - 1) - (MEDS_m - 1 - b) * MEDS_m + c];
      int r = 0;
      for (; r < MEDS_n - 8; r += 8)
      {
        // uint64_t tmp1 = pmod_mat_entry(P00nt, MEDS_n, MEDS_m, r, c);
        uint16_t tmp1_0 = pmod_mat_entry(P00nt, MEDS_n, MEDS_m, r, c);
        uint16_t tmp1_1 = pmod_mat_entry(P00nt, MEDS_n, MEDS_m, r + 1, c);
        uint16_t tmp1_2 = pmod_mat_entry(P00nt, MEDS_n, MEDS_m, r + 2, c);
        uint16_t tmp1_3 = pmod_mat_entry(P00nt, MEDS_n, MEDS_m, r + 3, c);
        uint16_t tmp1_4 = pmod_mat_entry(P00nt, MEDS_n, MEDS_m, r + 4, c);
        uint16_t tmp1_5 = pmod_mat_entry(P00nt, MEDS_n, MEDS_m, r + 5, c);
        uint16_t tmp1_6 = pmod_mat_entry(P00nt, MEDS_n, MEDS_m, r + 6, c);
        uint16_t tmp1_7 = pmod_mat_entry(P00nt, MEDS_n, MEDS_m, r + 7, c);
        uint16x8_t tmp1 = {tmp1_0, tmp1_1, tmp1_2, tmp1_3, tmp1_4, tmp1_5, tmp1_6, tmp1_7};

        // uint64_t prod = (tmp1 * tmp2) % MEDS_p;
        uint32x4_t prod_low = vmull_u16(vget_low_u16(tmp1), vget_low_u16(tmp2_16x8));
        uint32x4_t prod_high = vmull_high_u16(tmp1, tmp2_16x8);
        uint16x8_t prod_low_red = (uint16x8_t)FREEZE_REDUCE_VEC_16x4(prod_low);
        uint16x8_t prod_high_red = (uint16x8_t)FREEZE_REDUCE_VEC_16x4(prod_high);
        uint16x8_t prod = vuzp1q_u16(prod_low_red, prod_high_red);

        // uint64_t val = sol[b * MEDS_n + r];
        uint16x8_t val = vld1q_u16((uint16_t *)&sol[b * MEDS_n + r]);

        // val = ((MEDS_p + val) - prod) % MEDS_p;
        val = vsubq_u16(vaddq_u16(val, MEDS_p_VEC_16x8), prod);
        val = FREEZE_VEC_16x8(val);

        // sol[b * MEDS_n + r] = val;
        vst1q_u16((uint16_t *)&sol[b * MEDS_n + r], val);
      }
      for (; r < MEDS_n - 4; r += 4)
      {
        // uint64_t tmp1 = pmod_mat_entry(P00nt, MEDS_n, MEDS_m, r, c);
        uint16_t tmp1_0 = pmod_mat_entry(P00nt, MEDS_n, MEDS_m, r, c);
        uint16_t tmp1_1 = pmod_mat_entry(P00nt, MEDS_n, MEDS_m, r + 1, c);
        uint16_t tmp1_2 = pmod_mat_entry(P00nt, MEDS_n, MEDS_m, r + 2, c);
        uint16_t tmp1_3 = pmod_mat_entry(P00nt, MEDS_n, MEDS_m, r + 3, c);
        uint16x4_t tmp1 = {tmp1_0, tmp1_1, tmp1_2, tmp1_3};

        // uint64_t prod = (tmp1 * tmp2) % MEDS_p;
        uint32x4_t prod = vmull_u16(tmp1, tmp2_16x4);
        uint16x4_t prod_red = FREEZE_REDUCE_VEC_16x4_SHRINK(prod);

        // uint64_t val = sol[b * MEDS_n + r];
        uint16x4_t val = vld1_u16((uint16_t *)&sol[b * MEDS_n + r]);

        // val = ((MEDS_p + val) - prod) % MEDS_p;
        val = vsub_u16(vadd_u16(val, MEDS_p_VEC_16x4), prod_red);
        val = FREEZE_VEC_16x4(val);

        // sol[b * MEDS_n + r] = val;
        vst1_u16((uint16_t *)&sol[b * MEDS_n + r], val);
      }
      for (; r < MEDS_n; r++)
      {
        uint64_t tmp1 = pmod_mat_entry(P00nt, MEDS_n, MEDS_m, r, c);

        uint64_t prod = (tmp1 * tmp2) % MEDS_p;

        uint64_t val = sol[b * MEDS_n + r];

        val = ((MEDS_p + val) - prod) % MEDS_p;

        sol[b * MEDS_n + r] = val;
      }
    }
  return 0;
}

float min_cycle_bound_crude()
{
  int operations = 0;
  int reduce_cost = 5;
  int freeze_cost = 3;

  for (int b = MEDS_m - 3; b >= 0; b--)
    for (int c = MEDS_m - 1; c >= 0; c--)
    {
      operations++;
      for (int r = 0; r < MEDS_m; r += 8)
      {
        operations += 8;
        operations += 2 + 2 * reduce_cost + 1;
        operations++;
        operations += 2 + freeze_cost;
        operations++;
      }
    }

  for (int b = MEDS_m - 2; b >= 0; b--)
    for (int c = MEDS_m - 1; c >= 0; c--)
    {
      operations++;
      for (int r = 0; r < MEDS_n; r += 8)
      {
        operations += 8;
        operations += 2 + 2 * reduce_cost + 1;
        operations++;
        operations += 2 + freeze_cost;
        operations++;
      }
    }

  return operations * 8;
}

float min_cycle_bound()
{
  float result_vec8 = (1 / 8.0) * (MEDS_m - 3) * (MEDS_m - 1) * (28 * MEDS_m + 28 * MEDS_n + 16);
  return result_vec8 * 8;
}

void test_performance(char name[])
{
  // For the benchmarking, we can set all these values to 0, as the code runs in constant itme anyway.
  pmod_mat_t N[MEDS_n * (2 * MEDS_m)] = {0};
  GFq_t sol[MEDS_m * MEDS_m + MEDS_n * MEDS_n] = {0};
  pmod_mat_t P00nt[MEDS_n * MEDS_m] = {0};

  long long solve_cycles[ROUNDS + 1];

  for (int round = 0; round < ROUNDS; round++)
  {
    solve_cycles[round] = get_cyclecounter();
    solve_opt_part(N, sol, P00nt);
  }
  solve_cycles[ROUNDS] = get_cyclecounter();

  // Calculate results
  double solve_median_cc = median_2(solve_cycles, ROUNDS + 1, 0);

  double min_cycles = (double)min_cycle_bound();
  double solve_cycle_multiplier = solve_median_cc / (min_cycles / 8);

  // Print results
  printf("-----------------------------------\n");
  printf("=== %s ===\n", name);
  printf("-----------------------------------\n");
  printf("Minimum cycle amount: %f\n", min_cycles);
  printf("Minimum cycle amount (8-way) parallel: %f\n", min_cycles / 8);
  printf("Solve median: %f\t(x%f)\n", solve_median_cc, solve_cycle_multiplier);
  printf("\n");
}

int main(int argc, char *argv[])
{
  enable_cyclecounter();
  test_performance("solve_opt_part");
  disable_cyclecounter();
}