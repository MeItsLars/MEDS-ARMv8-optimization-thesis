CASE 1: OPTIMIZED HASH STRUCTURE:

TYPE                                | TOT. CYC   | HASH CYC  | HASH %  | Improvement
------------------------------------------------------------------------------------
Default                             | 1570859904 | 203441835 | 12.95 % | 
With keccak_f1600_x1_scalar_C       | 1543703424 | 203466389 | 13.18 % | + 24554
With keccak_f1600_x2_scalar_C       | 1532681088 | 194080469 | 12.66 % | - 9.361.366
With keccak_f1600_x4_hybrid_asm_v3p | 1515899904 | 155785329 | 10.28 % | - 47.656.506
With keccak_f1600_x5_hybrid_asm_v8  | 1512470400 | 168985177 | 11.17 % | - 34.456.658

The best option is the 4-way parallel implementation.

CASE 2: DEFAULT HASH STRUCTURE:
TYPE                                | TOT. CYC   | HASH CYC  | HASH %  | Improvement
------------------------------------------------------------------------------------
Default                             | 1544273536 | 203463431 | 13.18 % | 
With keccak_f1600_x1_scalar_C       | 1542335360 | 203365592 | 13.19 % | - 97839

Although these measurements suggest a small difference, over multiple runs, there does not seem to be a significant difference between the two implementations.