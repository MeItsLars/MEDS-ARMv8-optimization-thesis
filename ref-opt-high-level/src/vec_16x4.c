#include "vec_16x4.h"

// // Reduces a to a value in the range [0, 2^12)
// uint16x4_t reduce_vec(uint32x4_t a)
// {
//   uint32x4_t t;
//   t = vshrq_n_u32(a, GFq_bits);
//   t = vmulq_n_u32(t, MEDS_p);
//   a = vsubq_u32(a, t);
//   t = vshrq_n_u32(a, GFq_bits);
//   t = vmulq_n_u32(t, MEDS_p);
//   a = vsubq_u32(a, t);
//   return vqmovn_u32(a);
// }

// // Freezes a to a value in the range [0, MEDS_p)
// uint16x4_t freeze_vec(uint16x4_t a)
// {
//   uint16x4_t t;
//   t = vcge_u16(a, MEDS_p_VEC);
//   t = vand_u16(t, MEDS_p_VEC);
//   return vsub_u16(a, t);
// }

/**
 * Loads an element at a certain position from 4 matrices into a vector.
 * @param M The matrices.
 * @param M_r The number of rows in the matrices.
 * @param M_c The number of columns in the matrices.
 * @param r The row of the element.
 * @param c The column of the element.
 */
pmod_mat_vec_t load_vec(uint16_t *M[], int M_r, int M_c, int r, int c)
{
  uint16_t buf[BATCH_SIZE] = {0};
  for (int i = 0; i < BATCH_SIZE; i++)
  {
    buf[i] = M[i][r * M_c + c];
  }
  return vld1_u16(buf);
}

void store_vec(uint16_t *M[], int M_r, int M_c, int r, int c, pmod_mat_vec_t val)
{
  uint16_t buf[BATCH_SIZE] = {0};
  vst1_u16(buf, val);
  for (int i = 0; i < BATCH_SIZE; i++)
  {
    M[i][r * M_c + c] = buf[i];
  }
}