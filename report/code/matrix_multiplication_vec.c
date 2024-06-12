void pmod_mat_mul_vec(pmod_mat_vec_t *C, int C_r, int C_c,
                      pmod_mat_vec_t *A, int A_r, int A_c,
                      pmod_mat_vec_t *B, int B_r, int B_c)
{
  for (int c = 0; c < C_c; c++)
    for (int r = 0; r < C_r; r++)
    {
      pmod_mat_vec_w_t val = ZERO_VEC_W;
      for (int i = 0; i < A_c; i++)
        val = MUL_ACC_VEC(val,
                          pmod_mat_entry(A, A_r, A_c, r, i),
                          pmod_mat_entry(B, B_r, B_c, i, c));
      C[r * C_c + c] = FREEZE_REDUCE_VEC(val);
    }
}