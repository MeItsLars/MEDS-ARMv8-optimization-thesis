#include "vec_16x4.h"

/**
 * Loads an element at a certain position from 4 matrices into a vector.
 * @param M The matrices.
 * @param M_r The number of rows in the matrices.
 * @param M_c The number of columns in the matrices.
 * @param r The row of the element.
 * @param c The column of the element.
 */
pmod_mat_vec_t load_vec(GFq_t *M[], int M_r, int M_c, int r, int c)
{
  uint16_t buf[BATCH_SIZE] = {0};
  for (int i = 0; i < BATCH_SIZE; i++)
  {
    buf[i] = (uint16_t)M[i][r * M_c + c];
  }
  return vld1_u16(buf);
}

void store_vec(GFq_t *M[], int M_r, int M_c, int r, int c, pmod_mat_vec_t val, int amount)
{
  uint16_t buf[BATCH_SIZE] = {0};
  vst1_u16(buf, val);
  for (int i = 0; i < amount; i++)
  {
    M[i][r * M_c + c] = (GFq_t)buf[i];
  }
}

void store_bitstream_12bit(uint8_t **bs_buf, int bs_index, pmod_mat_vec_t *G_vec, int batch_size)
{
  int buf_idx = 0;
  for (int r = 0; r < MEDS_k; r++)
    for (int c = MEDS_k; c < MEDS_m * MEDS_n; c += 2)
    {
      // Extract two 12-bit values. Combine them into three 8-bit values. Store these values into bs_buf.
      pmod_mat_vec_t i0 = G_vec[r * MEDS_m * MEDS_n + c];
      pmod_mat_vec_t i1 = G_vec[r * MEDS_m * MEDS_n + c + 1];

      // r0 = (i0 & 0xff);
      uint16x4_t r0_wide = vand_u16(i0, vdup_n_u16(0xff));
      // r1 = (i0 >> 8) | ((i1 & 0xf) << 4);
      uint16x4_t r1_wide = vorr_u16(vshr_n_u16(i0, 8), vshl_n_u16(vand_u16(i1, vdup_n_u16(0xf)), 4));
      // r2 = (i1 >> 4);
      uint16x4_t r2_wide = vshr_n_u16(i1, 4);

      // Convert to 8-bit
      uint8x8_t r0 = vqmovn_u16(vcombine_u16(r0_wide, vdup_n_u16(0)));
      uint8x8_t r1 = vqmovn_u16(vcombine_u16(r1_wide, vdup_n_u16(0)));
      uint8x8_t r2 = vqmovn_u16(vcombine_u16(r2_wide, vdup_n_u16(0)));

      // Store to buffer
      uint8_t r0_buf[8];
      uint8_t r1_buf[8];
      uint8_t r2_buf[8];
      vst1_u8(r0_buf, r0);
      vst1_u8(r1_buf, r1);
      vst1_u8(r2_buf, r2);
      for (int i = 0; i < batch_size; i++)
      {
        bs_buf[bs_index + i][buf_idx] = r0_buf[i];
        bs_buf[bs_index + i][buf_idx + 1] = r1_buf[i];
        bs_buf[bs_index + i][buf_idx + 2] = r2_buf[i];
      }
      buf_idx += 3;
    }
}