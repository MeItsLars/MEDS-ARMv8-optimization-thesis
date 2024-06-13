for (int b = MEDS_m - 3; b >= 0; b--)
  for (int c = MEDS_m - 1; c >= 0; c--)
    for (int r = 0; r < MEDS_m; r++)
    {
      uint64_t tmp1 = pmod_mat_entry(N, MEDS_n-1,MEDS_m, r, c);
      uint64_t tmp2 = sol[(MEDS_m+1)*MEDS_n+b*MEDS_m+MEDS_m+c];
      uint64_t prod = (tmp1 * tmp2) % MEDS_p;
      uint64_t val = sol[(MEDS_m+1) * MEDS_n + b * MEDS_m + r];
      val = ((MEDS_p + val) - prod) % MEDS_p;
      sol[(MEDS_m + 1) * MEDS_n + b * MEDS_m + r] = val;
    }