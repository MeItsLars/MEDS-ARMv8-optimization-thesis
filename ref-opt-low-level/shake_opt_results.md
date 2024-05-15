TYPE                                | TOT. CYC   | HASH CYC  | HASH %  | Improvement
------------------------------------------------------------------------------------
Default                             | 1570859904 | 203441835 | 12.95 % | 
With keccak_f1600_x1_scalar_C       | 1543703424 | 203466389 | 13.18 % | + 24554
With keccak_f1600_x2_scalar_C       | 1532681088 | 194080469 | 12.66 % | - 9.361.366
With keccak_f1600_x4_hybrid_asm_v3p | 1515899904 | 155785329 | 10.28 % | - 47.656.506
With keccak_f1600_x5_hybrid_asm_v8  | ?          | ?         | ?       | ?

The final technique requires a different mapping into the 125-byte wide state. The current implementation is not yet ready for this.