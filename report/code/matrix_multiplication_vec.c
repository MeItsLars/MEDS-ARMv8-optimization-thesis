void pmod_mat_mul_vec(pmod_mat_vec_t *C, int C_r, int C_c,
                      pmod_mat_vec_t *A, int A_r, int A_c,
                      pmod_mat_vec_t *B, int B_r, int B_c)
{
  for (int c = 0; c < C_c; c++)
    for (int r = 0; r < C_r; r++)
    {
      pmod_mat_vec_w_t val_low = ZERO_VEC_W;
      pmod_mat_vec_w_t val_high = ZERO_VEC_W;
      for (int i = 0; i < A_c; i++)
      {
        val_low = MUL_ACC_VEC_LOW(val_low,
                          pmod_mat_entry(A, A_r, A_c, r, i),
                          pmod_mat_entry(B, B_r, B_c, i, c));
        val_high = MUL_ACC_VEC_HIGH(val_high,
                          pmod_mat_entry(A, A_r, A_c, r, i),
                          pmod_mat_entry(B, B_r, B_c, i, c));
      }
      pmod_mat_vec_t val_low_red = REDUCE_VEC_32BIT(val_low);
      pmod_mat_vec_t val_high_red = REDUCE_VEC_32BIT(val_high);
      C[r * C_c + c] = COMBINE_VEC(val_low_red, val_high_red);
    }
}