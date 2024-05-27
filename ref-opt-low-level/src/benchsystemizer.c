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
#include "profiler.h"

profileresult profileresults[1000];
int number_of_profileresults = 0;
int profiler_enabled = 0;

#define ROUNDS 100

// extern int pmod_mat_syst_test(pmod_mat_t *M);
extern int pmod_mat_syst_5_5_5_0_0(pmod_mat_t *M);
extern int pmod_mat_syst_k_k_k_0_0(pmod_mat_t *M);
extern int pmod_mat_syst_n_2n_n_0_1(pmod_mat_t *M);
extern int pmod_mat_syst_m_2m_m_0_1(pmod_mat_t *M);
extern int pmod_mat_syst_k_2k_k_0_1(pmod_mat_t *M);
extern int pmod_mat_syst_k_2k_k_0_0(pmod_mat_t *M);
extern int pmod_mat_syst_n_2m_nr1_0_1(pmod_mat_t *M);
extern int pmod_mat_syst_mr1_m_mr1_1_1(pmod_mat_t *M);
extern int pmod_mat_syst_5_5_5_0_0_nct(pmod_mat_t *M, uint16_t *inverse_table);
extern int pmod_mat_syst_k_k_k_0_0_nct(pmod_mat_t *M, uint16_t *inverse_table);
extern int pmod_mat_syst_n_2n_n_0_1_nct(pmod_mat_t *M, uint16_t *inverse_table);
extern int pmod_mat_syst_m_2m_m_0_1_nct(pmod_mat_t *M, uint16_t *inverse_table);
extern int pmod_mat_syst_k_2k_k_0_1_nct(pmod_mat_t *M, uint16_t *inverse_table);
extern int pmod_mat_syst_k_2k_k_0_0_nct(pmod_mat_t *M, uint16_t *inverse_table);
extern int pmod_mat_syst_n_2m_nr1_0_1_nct(pmod_mat_t *M, uint16_t *inverse_table);
extern int pmod_mat_syst_mr1_m_mr1_1_1_nct(pmod_mat_t *M, uint16_t *inverse_table);

int test_pmod_mat_syst_ct_partial_swap_backsub(pmod_mat_t *M, int M_r, int M_c, int max_r, int swap, int backsub)
{
  int ret = M_r * swap;

  for (int r = 0; r < max_r; r++)
  {
    if (swap)
    {
      GFq_t z = 0;

      // compute condition for swap
      for (int r2 = r; r2 < M_r; r2++)
        z |= pmod_mat_entry(M, M_r, M_c, r2, r);

      int do_swap = GFq_eq0(z);

      // conditional swap
      {
        ret = r * do_swap + ret * (1 - do_swap);

        for (int i = 0; i < M_r; i++)
          GFq_cswap(&pmod_mat_entry(M, M_r, M_c, i, r),
                    &pmod_mat_entry(M, M_r, M_c, i, M_c - 1),
                    do_swap);
      }
    }

    for (int r2 = r + 1; r2 < M_r; r2++)
    {
      uint64_t Mrr = pmod_mat_entry(M, M_r, M_c, r, r);

      for (int c = r; c < M_c; c++)
      {
        uint64_t val = pmod_mat_entry(M, M_r, M_c, r2, c);

        uint64_t Mrc = pmod_mat_entry(M, M_r, M_c, r, c);

        pmod_mat_set_entry(M, M_r, M_c, r, c, (Mrc + val * GFq_eq0(Mrr)) % MEDS_p);
      }
    }

    uint64_t val = pmod_mat_entry(M, M_r, M_c, r, r);

    if (val == 0)
    {
      return -1;
    }

    val = GF_inv(val);

    // normalize
    for (int c = r; c < M_c; c++)
    {
      uint64_t tmp = ((uint64_t)pmod_mat_entry(M, M_r, M_c, r, c) * val) % MEDS_p;
      pmod_mat_set_entry(M, M_r, M_c, r, c, tmp);
    }

    // eliminate
    for (int r2 = r + 1; r2 < M_r; r2++)
    {
      uint64_t factor = pmod_mat_entry(M, M_r, M_c, r2, r);

      for (int c = r; c < M_c; c++)
      {
        uint64_t tmp0 = pmod_mat_entry(M, M_r, M_c, r, c);
        uint64_t tmp1 = pmod_mat_entry(M, M_r, M_c, r2, c);

        uint64_t val = (tmp0 * factor) % MEDS_p;

        val = (MEDS_p + tmp1 - val) % MEDS_p;

        pmod_mat_set_entry(M, M_r, M_c, r2, c, val);
      }
    }
  }

  if (!backsub)
  {
    return ret;
  }

  // back substitution
  for (int r = max_r - 1; r >= 0; r--)
    for (int r2 = 0; r2 < r; r2++)
    {
      uint64_t factor = pmod_mat_entry(M, M_r, M_c, r2, r);

      uint64_t tmp0 = pmod_mat_entry(M, M_r, M_c, r, r);
      uint64_t tmp1 = pmod_mat_entry(M, M_r, M_c, r2, r);

      uint64_t val = (tmp0 * factor) % MEDS_p;

      val = (MEDS_p + tmp1 - val) % MEDS_p;

      pmod_mat_set_entry(M, M_r, M_c, r2, r, val);

      for (int c = max_r; c < M_c; c++)
      {
        uint64_t tmp0 = pmod_mat_entry(M, M_r, M_c, r, c);
        uint64_t tmp1 = pmod_mat_entry(M, M_r, M_c, r2, c);

        uint64_t val = (tmp0 * factor) % MEDS_p;

        val = (MEDS_p + tmp1 - val) % MEDS_p;

        pmod_mat_set_entry(M, M_r, M_c, r2, c, val);
      }
    }
  return ret;
}

uint16x4_t reduce(uint32x4_t red, uint16x4_t MEDS_p_16x4, uint32x4_t MEDS_p_32x4)
{
  // >>> Reduce modulo MEDS_p
  // Apply two reductions
  uint32x4_t tmp = vshrq_n_u32(red, GFq_bits);
  tmp = vmulq_n_u32(tmp, MEDS_p);
  red = vsubq_u32(red, tmp);
  tmp = vshrq_n_u32(red, GFq_bits);
  tmp = vmulq_n_u32(tmp, MEDS_p);
  red = vsubq_u32(red, tmp);

  // Reduce to a value between 0 and MEDS_p - 1:
  uint32x4_t diff = vsubq_u32(red, MEDS_p_32x4);
  uint32x4_t mask = vandq_u32(vshrq_n_u32(diff, 31), vdupq_n_u32(1));
  red = vaddq_u32(vmulq_u32(mask, red), vmulq_u32(vsubq_u32(vdupq_n_u32(1), mask), diff));

  // Convert to smaller type
  return vqmovn_u32(red);
}

int test_pmod_mat_syst_ct_partial_swap_backsub_2(pmod_mat_t *M, int M_r, int M_c, int max_r, int swap, int backsub)
{
  int ret = 0;

  uint16x8_t MEDS_p_16x8 = vdupq_n_u16(MEDS_p);
  uint16x4_t MEDS_p_16x4 = vdup_n_u16(MEDS_p);
  uint32x4_t MEDS_p_32x4 = vdupq_n_u32(MEDS_p);
  // uint32x4_t one_32x4 = vdupq_n_u32(1);

  for (int r = 0; r < max_r; r++)
  {
    if (swap)
    {
      // TODO
    }

    for (int r2 = r + 1; r2 < M_r; r2++)
    {
      uint64_t Mrr = pmod_mat_entry(M, M_r, M_c, r, r);
      uint16x8_t swap = vdupq_n_u16(GFq_eq0(Mrr));

      int c;
      for (c = r; c < M_c - 8; c += 8)
      {
        // Compute result
        uint16x8_t val = vld1q_u16(&M[(M_c)*r2 + c]);
        uint16x8_t Mrc = vld1q_u16(&M[(M_c)*r + c]);
        uint16x8_t result = vaddq_u16(Mrc, vmulq_u16(val, swap));

        // Subtract MEDS_p if result is larger than MEDS_p
        uint16x8_t mask = vcgtq_u16(result, MEDS_p_16x8);
        result = vsubq_u16(result, vandq_u16(MEDS_p_16x8, mask));

        // Store result
        vst1q_u16(&M[(M_c)*r + c], result);
      }
      for (c = c; c < M_c; c++)
      {
        uint64_t val = pmod_mat_entry(M, M_r, M_c, r2, c);
        uint64_t Mrc = pmod_mat_entry(M, M_r, M_c, r, c);
        pmod_mat_set_entry(M, M_r, M_c, r, c, (Mrc + val * GFq_eq0(Mrr)) % MEDS_p);
      }
    }

    uint64_t val = pmod_mat_entry(M, M_r, M_c, r, r);

    if (val == 0)
      return -1;

    // Compute the multiplicative inverse of the element on the diagonal
    val = GF_inv(val);
    uint16x4_t val_v = vdup_n_u16(val);

    // normalize;
    int c;
    for (c = r; c < M_c - 4; c += 4)
    {
      // Compute result
      uint16x4_t vals = vld1_u16(&M[(M_c)*r + c]);
      uint32x4_t red = vmull_u16(vals, val_v);
      uint16x4_t red_16x4 = reduce(red, MEDS_p_16x4, MEDS_p_32x4);

      // Store result
      vst1_u16(&M[(M_c)*r + c], red_16x4);
    }
    for (c = c; c < M_c; c++)
    {
      uint64_t tmp = ((uint64_t)pmod_mat_entry(M, M_r, M_c, r, c) * val) % MEDS_p;
      pmod_mat_set_entry(M, M_r, M_c, r, c, tmp);
    }

    // eliminate;
    for (int r2 = r + 1; r2 < M_r; r2++)
    {
      // Calculate the factor
      GFq_t factor = pmod_mat_entry(M, M_r, M_c, r2, r);
      uint16x4_t factor_16x4 = vdup_n_u16(factor);

      int c;
      for (c = r; c < M_c - 4; c += 4)
      {
        // First, compute val (red)
        uint16x4_t tmp0 = vld1_u16(&M[(M_c)*r + c]);
        uint32x4_t val = vmull_u16(tmp0, factor_16x4);
        uint16x4_t val_16x4 = reduce(val, MEDS_p_16x4, MEDS_p_32x4);

        // Compute the new val
        uint16x4_t tmp1 = vld1_u16(&M[(M_c)*r2 + c]);
        tmp1 = vadd_u16(tmp1, MEDS_p_16x4);
        val_16x4 = vsub_u16(tmp1, val_16x4);

        // Reduce to a value between 0 and MEDS_p - 1:
        uint16x4_t diff = vsub_u16(val_16x4, MEDS_p_16x4);
        uint16x4_t mask = vand_u16(vshr_n_u16(diff, 15), vdup_n_u16(1));
        val_16x4 = vadd_u16(vmul_u16(mask, val_16x4), vmul_u16(vsub_u16(vdup_n_u16(1), mask), diff));

        // Store result
        vst1_u16(&M[(M_c)*r2 + c], val_16x4);
      }
      for (c = c; c < M_c; c++)
      {
        uint64_t tmp0 = pmod_mat_entry(M, M_r, M_c, r, c);
        uint64_t tmp1 = pmod_mat_entry(M, M_r, M_c, r2, c);

        uint64_t val = (tmp0 * factor) % MEDS_p;

        val = (MEDS_p + tmp1 - val) % MEDS_p;

        pmod_mat_set_entry(M, M_r, M_c, r2, c, val);
      }
    }
  }

  if (!backsub)
  {
    return ret;
  }

  // back substitution;
  for (int r = max_r - 1; r >= 0; r--)
    for (int r2 = 0; r2 < r; r2++)
    {
      // Compute the factor
      uint64_t factor = pmod_mat_entry(M, M_r, M_c, r2, r);
      uint16x4_t factor_16x4 = vdup_n_u16(factor);

      // Set the value in the current column
      uint64_t tmp0 = pmod_mat_entry(M, M_r, M_c, r, r);
      uint64_t tmp1 = pmod_mat_entry(M, M_r, M_c, r2, r);

      uint64_t val = (tmp0 * factor) % MEDS_p;

      val = (MEDS_p + tmp1 - val) % MEDS_p;

      pmod_mat_set_entry(M, M_r, M_c, r2, r, val);

      // Calculate and set the new values in columns right of the diagonal block.
      // The values in the diagonal block are already 0 so we don't need to do anything with them.
      int c;
      for (c = max_r; c < M_c - 4; c += 4)
      {
        // uint64_t tmp0 = pmod_mat_entry(M, M_r, M_c, r, c);
        // uint64_t tmp1 = pmod_mat_entry(M, M_r, M_c, r2, c);

        // uint64_t val = (tmp0 * factor) % MEDS_p;

        // val = (MEDS_p + tmp1 - val) % MEDS_p;

        // pmod_mat_set_entry(M, M_r, M_c, r2, c, val);

        // First, compute val (red)
        uint16x4_t tmp0 = vld1_u16(&M[(M_c)*r + c]);
        uint32x4_t val = vmull_u16(tmp0, factor_16x4);
        uint16x4_t val_16x4 = reduce(val, MEDS_p_16x4, MEDS_p_32x4);

        // Compute the new val
        uint16x4_t tmp1 = vld1_u16(&M[(M_c)*r2 + c]);
        tmp1 = vadd_u16(tmp1, MEDS_p_16x4);
        val_16x4 = vsub_u16(tmp1, val_16x4);

        // Reduce to a value between 0 and MEDS_p - 1:
        uint16x4_t diff = vsub_u16(val_16x4, MEDS_p_16x4);
        uint16x4_t mask = vand_u16(vshr_n_u16(diff, 15), vdup_n_u16(1));
        val_16x4 = vadd_u16(vmul_u16(mask, val_16x4), vmul_u16(vsub_u16(vdup_n_u16(1), mask), diff));

        // Store result
        vst1_u16(&M[(M_c)*r2 + c], val_16x4);
      }
      for (c = c; c < M_c; c++)
      {
        uint64_t tmp0 = pmod_mat_entry(M, M_r, M_c, r, c);
        uint64_t tmp1 = pmod_mat_entry(M, M_r, M_c, r2, c);

        uint64_t val = (tmp0 * factor) % MEDS_p;

        val = (MEDS_p + tmp1 - val) % MEDS_p;

        pmod_mat_set_entry(M, M_r, M_c, r2, c, val);
      }
    }
  return ret;
}

int min_cycle_bound(int M_r, int M_c, int max_r, int swap, int backsub)
{
  int reduce_cost = 10;
  int freeze_cost = 3;
  int inv_cost = 16 * (1 + reduce_cost) + freeze_cost;

  int arithmetic = 0, loads = 0, stores = 0, loop = 0;

  // int ret = M_r * swap;
  arithmetic++;

  for (int r = 0; r < max_r; r++)
  {
    loop++;
    if (swap)
    {
      // compute condition for swap
      for (int r2 = r; r2 < M_r; r2++)
      {
        loop++;
        // z |= pmod_mat_entry(M, M_r, M_c, r2, r);
        loads++;
        arithmetic++;
      }

      // int do_swap = GFq_eq0(z);
      arithmetic++;

      // conditional swap
      {
        // ret = r * do_swap + ret * (1 - do_swap);
        arithmetic += 2;

        for (int i = 0; i < M_r; i++)
        {
          loop++;
          // GFq_cswap(&pmod_mat_entry(M, M_r, M_c, i, r),
          //           &pmod_mat_entry(M, M_r, M_c, i, M_c - 1),
          //           do_swap);
          loads += 2;
          arithmetic += 2;
          stores += 2;
        }
      }
    }

    for (int r2 = r + 1; r2 < M_r; r2++)
    {
      loop++;
      // uint64_t Mrr = pmod_mat_entry(M, M_r, M_c, r, r);
      loads++;

      for (int c = r; c < M_c; c++)
      {
        loop++;
        // uint64_t val = pmod_mat_entry(M, M_r, M_c, r2, c);
        loads++;

        // uint64_t Mrc = pmod_mat_entry(M, M_r, M_c, r, c);
        loads++;

        // pmod_mat_set_entry(M, M_r, M_c, r, c, (Mrc + val * GFq_eq0(Mrr)) % MEDS_p);
        arithmetic += 2;
        arithmetic += reduce_cost;
        stores++;
      }
    }

    // uint64_t val = pmod_mat_entry(M, M_r, M_c, r, r);
    loads++;

    // val = GF_inv(val);
    arithmetic += inv_cost;

    // normalize
    for (int c = r; c < M_c; c++)
    {
      loop++;
      // uint64_t tmp = ((uint64_t)pmod_mat_entry(M, M_r, M_c, r, c) * val) % MEDS_p;
      // pmod_mat_set_entry(M, M_r, M_c, r, c, tmp);
      loads++;
      arithmetic += 1;
      arithmetic += reduce_cost;
      stores++;
    }

    // eliminate
    for (int r2 = r + 1; r2 < M_r; r2++)
    {
      loop++;
      // uint64_t factor = pmod_mat_entry(M, M_r, M_c, r2, r);
      loads++;

      for (int c = r; c < M_c; c++)
      {
        loop++;
        // uint64_t tmp0 = pmod_mat_entry(M, M_r, M_c, r, c);
        // uint64_t tmp1 = pmod_mat_entry(M, M_r, M_c, r2, c);
        loads += 2;

        // uint64_t val = (tmp0 * factor) % MEDS_p;
        arithmetic += 1;
        arithmetic += reduce_cost;

        // val = (MEDS_p + tmp1 - val) % MEDS_p;
        arithmetic += 2;
        arithmetic += freeze_cost;

        // pmod_mat_set_entry(M, M_r, M_c, r2, c, val);
        stores++;
      }
    }
  }

  if (backsub)
  {
    // back substitution
    for (int r = max_r - 1; r >= 0; r--)
    {
      loop++;
      for (int r2 = 0; r2 < r; r2++)
      {
        loop++;
        // uint64_t factor = pmod_mat_entry(M, M_r, M_c, r2, r);
        loads++;

        // uint64_t tmp0 = pmod_mat_entry(M, M_r, M_c, r, r);
        // uint64_t tmp1 = pmod_mat_entry(M, M_r, M_c, r2, r);
        loads += 2;

        // uint64_t val = (tmp0 * factor) % MEDS_p;
        arithmetic += 1;
        arithmetic += reduce_cost;

        // val = (MEDS_p + tmp1 - val) % MEDS_p;
        arithmetic += 2;
        arithmetic += freeze_cost;

        // pmod_mat_set_entry(M, M_r, M_c, r2, r, val);
        stores++;

        for (int c = max_r; c < M_c; c++)
        {
          loop++;
          // uint64_t tmp0 = pmod_mat_entry(M, M_r, M_c, r, c);
          // uint64_t tmp1 = pmod_mat_entry(M, M_r, M_c, r2, c);
          loads += 2;

          // uint64_t val = (tmp0 * factor) % MEDS_p;
          arithmetic += 1;
          arithmetic += reduce_cost;

          // val = (MEDS_p + tmp1 - val) % MEDS_p;
          arithmetic += 2;
          arithmetic += freeze_cost;

          // pmod_mat_set_entry(M, M_r, M_c, r2, c, val);
          stores++;
        }
      }
    }
  }
  return loads + stores + arithmetic + loop;
}

void test_performance(char name[], int r, int c, int max_r, int swap, int backsub, int (*function)(pmod_mat_t *), int (*function_nct)(pmod_mat_t *, uint16_t *))
{
  uint8_t seed[MEDS_pub_seed_bytes];

  for (int i = 0; i < MEDS_pub_seed_bytes; i++)
  {
    seed[i] = i;
  }
  keccak_state shake;
  shake256_absorb_once(&shake, seed, MEDS_pub_seed_bytes);

  __attribute__((aligned(16))) pmod_mat_t A1[r * c];
  __attribute__((aligned(16))) pmod_mat_t A2[r * c];
  __attribute__((aligned(16))) pmod_mat_t A3[r * c];
  __attribute__((aligned(16))) pmod_mat_t A4[r * c];

  for (int i = 0; i < r * c; i++)
  {
    GFq_t rand_el = rnd_GF(&shake);
    A1[i] = rand_el;
    A2[i] = rand_el;
    A3[i] = rand_el;
    A4[i] = rand_el;
  }

  long long systemizer_cycles[ROUNDS + 1];
  long long intrinsic_systemizer_cycles[ROUNDS + 1];
  long long asm_systemizer_cycles[ROUNDS + 1];
  long long asm_systemizer_cycles_nct[ROUNDS + 1];

  int systemizer_res;
  int intrinsic_res;
  int asm_res;
  int asm_nct_res;

  for (int round = 0; round < ROUNDS; round++)
  {
    systemizer_cycles[round] = get_cyclecounter();
    systemizer_res = test_pmod_mat_syst_ct_partial_swap_backsub(A1, r, c, max_r, swap, backsub);
  }
  systemizer_cycles[ROUNDS] = get_cyclecounter();

  for (int round = 0; round < ROUNDS; round++)
  {
    intrinsic_systemizer_cycles[round] = get_cyclecounter();
    intrinsic_res = test_pmod_mat_syst_ct_partial_swap_backsub_2(A2, r, c, max_r, swap, backsub);
  }
  intrinsic_systemizer_cycles[ROUNDS] = get_cyclecounter();

  for (int round = 0; round < ROUNDS; round++)
  {
    asm_systemizer_cycles[round] = get_cyclecounter();
    asm_res = function(A3);
  }
  asm_systemizer_cycles[ROUNDS] = get_cyclecounter();

  for (int round = 0; round < ROUNDS; round++)
  {
    asm_systemizer_cycles_nct[round] = get_cyclecounter();
    asm_nct_res = function_nct(A4, mod_inverse_table);
  }
  asm_systemizer_cycles_nct[ROUNDS] = get_cyclecounter();

  // Calculate results
  double systemizer_median_cc = median_2(systemizer_cycles, ROUNDS + 1, 0);
  double intrinsic_systemizer_median_cc = median_2(intrinsic_systemizer_cycles, ROUNDS + 1, 0);
  double asm_systemizer_median_cc = median_2(asm_systemizer_cycles, ROUNDS + 1, 0);
  double asm_systemizer_median_nct_cc = median_2(asm_systemizer_cycles_nct, ROUNDS + 1, 0);

  double min_cycles = (double)min_cycle_bound(r, c, max_r, swap, backsub);
  double systemizer_cycle_multiplier = systemizer_median_cc / (min_cycles / 4);
  double intrinsic_cycle_multiplier = intrinsic_systemizer_median_cc / (min_cycles / 4);
  double asm_cycle_multiplier = asm_systemizer_median_cc / (min_cycles / 4);
  double asm_nct_cycle_multiplier = asm_systemizer_median_nct_cc / (min_cycles / 4);

  double intrinsic_improvement = (intrinsic_systemizer_median_cc - systemizer_median_cc) / systemizer_median_cc * 100;
  double asm_improvement = (asm_systemizer_median_cc - systemizer_median_cc) / systemizer_median_cc * 100;
  double asm_nct_improvement = (asm_systemizer_median_nct_cc - systemizer_median_cc) / systemizer_median_cc * 100;

  int intrinsic_inequalities = intrinsic_res == systemizer_res ? 0 : 1;
  int asm_inequalities = asm_res == systemizer_res ? 0 : 1;
  int asm_nct_inequalities = asm_nct_res == systemizer_res ? 0 : 1;
  for (int i = 0; i < r * c; i++)
  {
    if (A1[i] != A2[i])
    {
      intrinsic_inequalities++;
    }
    if (A1[i] != A3[i])
    {
      asm_inequalities++;
    }
    if (A1[i] != A4[i])
    {
      asm_nct_inequalities++;
    }
  }

  // Print results
  printf("-----------------------------------\n");
  printf("=== %s ===\n", name);
  printf("-----------------------------------\n");
  if (asm_inequalities == 0)
    printf("> ASM EQUAL\n");
  else
    printf("> ASM INEQUALITIES: %d/%d\n", asm_inequalities, r * c + 1);
  if (asm_nct_inequalities == 0)
    printf("> ASM (NCT) EQUAL\n");
  else
    printf("> ASM (NCT) INEQUALITIES: %d/%d\n", asm_nct_inequalities, r * c + 1);
  if (intrinsic_inequalities == 0)
    printf("> INTRINSIC EQUAL\n");
  else
    printf("> INTRINSIC INEQUALITIES: %d/%d\n", intrinsic_inequalities, r * c + 1);

  printf("Minimum cycle amount: %f\n", min_cycles);
  printf("Minimum cycle amount (4-way) parallel: %f\n", min_cycles / 4);
  printf("Systemizer median: %f\t(x%f)\n", systemizer_median_cc, systemizer_cycle_multiplier);
  printf("Intrinsic median: %f\t(x%f)\n", intrinsic_systemizer_median_cc, intrinsic_cycle_multiplier);
  printf("ASM median: %f\t(x%f)\n", asm_systemizer_median_cc, asm_cycle_multiplier);
  printf("ASM (NCT) median: %f\t(x%f)\n", asm_systemizer_median_nct_cc, asm_nct_cycle_multiplier);

  printf("Improvement (intrinsic): %f%%\n", intrinsic_improvement);
  printf("Improvement (ASM): %f%%\n", asm_improvement);
  printf("Improvement (ASM NCT): %f%%\n", asm_nct_improvement);
  printf("\n");
}

/*
#define A_ROWS 5
#define A_COLS 5

int main(int argc, char *argv[])
{
  enable_cyclecounter();

  uint8_t seed1[MEDS_pub_seed_bytes];

  for (int i = 0; i < MEDS_pub_seed_bytes; i++)
  {
    seed1[i] = i;
  }

  __attribute__((aligned(16))) pmod_mat_t A1[A_ROWS * A_COLS];
  __attribute__((aligned(16))) pmod_mat_t A2[A_ROWS * A_COLS];
  __attribute__((aligned(16))) pmod_mat_t A3[A_ROWS * A_COLS];

  keccak_state shake;
  shake256_absorb_once(&shake, seed1, MEDS_pub_seed_bytes);

  long long old_systemizer_cycles[ROUNDS];
  long long intrinsic_systemizer_cycles[ROUNDS];
  long long new_systemizer_cycles[ROUNDS];

  // Fill matrices with random values
  for (int r = 0; r < A_ROWS; r++)
    for (int c = 0; c < A_COLS; c++)
    {
      GFq_t rand_el = rnd_GF(&shake);
      // Fill the second column with zeroes to test swap
      // if (c == 1)
      //   rand_el = 0;
      pmod_mat_set_entry(A1, A_ROWS, A_COLS, r, c, rand_el);
      pmod_mat_set_entry(A2, A_ROWS, A_COLS, r, c, rand_el);
      pmod_mat_set_entry(A3, A_ROWS, A_COLS, r, c, rand_el);
    }
  pmod_mat_set_entry(A1, A_ROWS, A_COLS, 0, 0, 0);
  pmod_mat_set_entry(A2, A_ROWS, A_COLS, 0, 0, 0);
  pmod_mat_set_entry(A3, A_ROWS, A_COLS, 0, 0, 0);

  int res1, res2, res3;

  printf("A:\n");
  pmod_mat_fprint(stdout, A1, A_ROWS, A_COLS);

  for (int round = 0; round < ROUNDS - 1; round++)
  {
    old_systemizer_cycles[round] = get_cyclecounter();
    res1 = test_pmod_mat_syst_ct_partial_swap_backsub(A1, A_ROWS, A_COLS, A_ROWS, 0, 0);
  }
  old_systemizer_cycles[ROUNDS - 1] = get_cyclecounter();

  for (int round = 0; round < ROUNDS - 1; round++)
  {
    intrinsic_systemizer_cycles[round] = get_cyclecounter();
    res3 = test_pmod_mat_syst_ct_partial_swap_backsub_2(A3, A_ROWS, A_COLS);
  }
  intrinsic_systemizer_cycles[ROUNDS - 1] = get_cyclecounter();

  for (int round = 0; round < ROUNDS - 1; round++)
  {
    new_systemizer_cycles[round] = get_cyclecounter();
    res2 = pmod_mat_syst_5_5_5_0_0(A2);
  }
  new_systemizer_cycles[ROUNDS - 1] = get_cyclecounter();

  printf("A1: (%d)\n", res1);
  pmod_mat_fprint(stdout, A1, A_ROWS, A_COLS);
  printf("A2: (%d)\n", res2);
  pmod_mat_fprint(stdout, A2, A_ROWS, A_COLS);

  double old_systemizer_median_cc = median_2(old_systemizer_cycles, ROUNDS, 0);
  double intrinsic_systemizer_median_cc = median_2(intrinsic_systemizer_cycles, ROUNDS, 0);
  double new_systemizer_median_cc = median_2(new_systemizer_cycles, ROUNDS, 0);

  // Print results
  double percentage = new_systemizer_median_cc / old_systemizer_median_cc * 100;
  double improvement = (new_systemizer_median_cc - old_systemizer_median_cc) / old_systemizer_median_cc * 100;
  double improvement_intrinsics = (new_systemizer_median_cc - intrinsic_systemizer_median_cc) / intrinsic_systemizer_median_cc * 100;
  printf("Old median: %f\n", old_systemizer_median_cc);
  printf("Intrinsics median: %f\n", intrinsic_systemizer_median_cc);
  printf("New median: %f\n", new_systemizer_median_cc);
  printf("Minimum cycle bound: %f\n", min_cycle_bound(A_ROWS, A_COLS, A_ROWS));
  printf("Minimum cycle bound (4-way parallel): %f\n", min_cycle_bound(A_ROWS, A_COLS, A_ROWS) / 4);
  printf("Percentage: %f%%\n", percentage);
  printf("Improvement: %f%%\n", improvement);
  printf("Improvement (intrinsics): %f%%\n", improvement_intrinsics);

  disable_cyclecounter();

  // Compare matrices
  int equalities = 0;
  for (int r = 0; r < A_ROWS; r++)
    for (int c = 0; c < A_COLS; c++)
      if (pmod_mat_entry(A1, A_ROWS, A_COLS, r, c) == pmod_mat_entry(A2, A_ROWS, A_COLS, r, c))
        equalities++;

  int expected_equalities = A_ROWS * A_COLS;

  if (expected_equalities == equalities)
  {
    printf("Matrices are EQUAL\n");
  }
  else
  {
    printf("Equalities: %d / %d\n", equalities, expected_equalities);
    printf("Matrices are NOT EQUAL\n");
    exit(-1);
  }

  return 0;
}*/

int main(int argc, char *argv[])
{
  enable_cyclecounter();
  // test_performance("pmod_mat_syst_k_k_k_0_0", MEDS_k, MEDS_k, MEDS_k, 0, 0, pmod_mat_syst_k_k_k_0_0, pmod_mat_syst_k_k_k_0_0_nct);
  // test_performance("pmod_mat_syst_n_2n_n_0_1", MEDS_n, 2 * MEDS_n, MEDS_n, 0, 1, pmod_mat_syst_n_2n_n_0_1, pmod_mat_syst_n_2n_n_0_1_nct);
  // test_performance("pmod_mat_syst_m_2m_m_0_1", MEDS_m, 2 * MEDS_m, MEDS_m, 0, 1, pmod_mat_syst_m_2m_m_0_1, pmod_mat_syst_m_2m_m_0_1_nct);
  test_performance("pmod_mat_syst_k_2k_k_0_1", MEDS_k, 2 * MEDS_k, MEDS_k, 0, 1, pmod_mat_syst_k_2k_k_0_1, pmod_mat_syst_k_2k_k_0_1_nct);
  // test_performance("pmod_mat_syst_n_2m_nr1_0_1", MEDS_n, 2 * MEDS_m, MEDS_n - 1, 0, 1, pmod_mat_syst_n_2m_nr1_0_1, pmod_mat_syst_n_2m_nr1_0_1_nct);
  // test_performance("pmod_mat_syst_mr1_m_mr1_1_1", MEDS_m - 1, MEDS_m, MEDS_m - 1, 1, 1, pmod_mat_syst_mr1_m_mr1_1_1, pmod_mat_syst_mr1_m_mr1_1_1_nct);
  // test_performance("pmod_mat_syst_k_2k_k_0_0", MEDS_k, 2 * MEDS_k, MEDS_k, 0, 0, pmod_mat_syst_k_2k_k_0_0, pmod_mat_syst_k_2k_k_0_0_nct);
  disable_cyclecounter();
}