#include "vec_16x8.h"

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
  return vld1q_u16(buf);
}

void store_vec(GFq_t *M[], int M_r, int M_c, int r, int c, pmod_mat_vec_t val, int amount)
{
  uint16_t buf[BATCH_SIZE] = {0};
  vst1q_u16(buf, val);
  for (int i = 0; i < amount; i++)
  {
    M[i][r * M_c + c] = (GFq_t)buf[i];
  }
}

static inline void buf_set_12bit_pair(uint8_t **buf, int *buf_idx, int batch_size, pmod_mat_vec_t v0, pmod_mat_vec_t v1)
{
  uint16x4_t r0_wide = vand_u16(v0, vdup_n_u16(0xff));
  uint16x4_t r1_wide = vorr_u16(vshr_n_u16(v0, 8), vshl_n_u16(vand_u16(v1, vdup_n_u16(0xf)), 4));
  uint16x4_t r2_wide = vshr_n_u16(v1, 4);
  uint8x8_t r0 = vqmovn_u16(vcombine_u16(r0_wide, vdup_n_u16(0)));
  uint8x8_t r1 = vqmovn_u16(vcombine_u16(r1_wide, vdup_n_u16(0)));
  uint8x8_t r2 = vqmovn_u16(vcombine_u16(r2_wide, vdup_n_u16(0)));
  uint8_t r0_buf[3][8];
  vst1_u8(r0_buf[0], r0);
  vst1_u8(r0_buf[1], r1);
  vst1_u8(r0_buf[2], r2);
  for (int i = 0; i < batch_size; i++)
  {
    buf[i][*buf_idx] = r0_buf[0][i];
    buf[i][*buf_idx + 1] = r0_buf[1][i];
    buf[i][*buf_idx + 2] = r0_buf[2][i];
  }
  *buf_idx += 3;
}

static inline void buf_set_12bit_element(uint8_t **buf, int *buf_idx, int batch_size, pmod_mat_vec_t v0)
{
  uint16x4_t r0_wide = vand_u16(v0, vdup_n_u16(0xff));
  uint16x4_t r1_wide = vorr_u16(vshr_n_u16(v0, 8), vdup_n_u16(0));
  uint8x8_t r0 = vqmovn_u16(vcombine_u16(r0_wide, vdup_n_u16(0)));
  uint8x8_t r1 = vqmovn_u16(vcombine_u16(r1_wide, vdup_n_u16(0)));
  uint8_t r0_buf[2][8];
  vst1_u8(r0_buf[0], r0);
  vst1_u8(r0_buf[1], r1);
  for (int i = 0; i < batch_size; i++)
  {
    buf[i][*buf_idx] = r0_buf[0][i];
    buf[i][*buf_idx + 1] = r0_buf[1][i];
  }
  *buf_idx += 2;
}

void store_bitstream_12bit(uint8_t **bs_buf, int bs_index, pmod_mat_vec_t *G_vec, int batch_size)
{
  int buf_idx = 0;
  GFq_vec_t previous_element;
  int has_previous_element = 0;
  for (int r = 0; r < MEDS_k; r++)
  {
    int c = MEDS_k;
    int raw_idx = r * MEDS_m * MEDS_n;
    // If the previous row had an element left, write it with the first element of this row
    if (has_previous_element)
    {
      buf_set_12bit_pair(bs_buf + bs_index, &buf_idx, batch_size, previous_element, G_vec[raw_idx + c]);
      c++;
      has_previous_element = 0;
    }
    // Write the rest of the row in pairs of 12-bit elements
    for (; c < MEDS_m * MEDS_n - 1; c += 2)
    {
      buf_set_12bit_pair(bs_buf + bs_index, &buf_idx, batch_size, G_vec[raw_idx + c], G_vec[raw_idx + c + 1]);
    }
    // If this row has an element left, save it for the next row
    if (c == MEDS_m * MEDS_n - 1)
    {
      previous_element = G_vec[raw_idx + c];
      has_previous_element = 1;
    }
  }
  // If the final row had an element left, store it
  if (has_previous_element)
  {
    buf_set_12bit_element(bs_buf + bs_index, &buf_idx, batch_size, previous_element);
  }
}