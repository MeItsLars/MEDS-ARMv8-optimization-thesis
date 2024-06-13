void pmod_mat_mul(pmod_mat_t *C, int C_r, int C_c, 
                  pmod_mat_t *A, int A_r, int A_c, 
                  pmod_mat_t *B, int B_r, int B_c)
{
  for (int c = 0; c < C_c; c++)
    for (int r = 0; r < C_r; r++)
    {
      uint64_t val = 0;
      for (int i = 0; i < A_c; i++)
        val = val + 
              (uint64_t)pmod_mat_entry(A, A_r, A_c, r, i) *
              (uint64_t)pmod_mat_entry(B, B_r, B_c, i, c);
      C[r * C_c + c] = val % MEDS_p;
    }
}