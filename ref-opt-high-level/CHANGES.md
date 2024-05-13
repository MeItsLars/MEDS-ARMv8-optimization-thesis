List of things that were changed as a high-level optimization compared to the original implementation:
- Created a 4-way parallel method for all required underlying operations:
    * pmod_mat_mul (matrix multiplication)
    * pmod_mat_syst_ct_partial_swap_backsub_vec (matrix systemization)
    * GF_inv_vec (Galois Field inversion)
    * pmod_mat_inv_vec (matrix inversion)
    * solve_vec (matrix system solving)
    * pi_vec (applying the pi function)
    * SF_vec (applying the SF function)
- Created a 4-way parallel implementation for the challenges in signing and verification
- Created a specific and faster implementation for copying commitment data into the buffer that will be hashed (bs_fill)