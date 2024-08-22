# MEDS for ARMv8

## Matrix Equivalence Digital Signature

This repository provides three implementations of the PQC signature scheme MEDS, see [meds-pqc.org](https://www.meds-pqc.org/):
- A reference implementation in C
- A low-level optimized implementation for ARMv8 in C and assembly
- A high-level optimized implementation for ARMv8 in C

## Building and running the code

All three implementations can be compiled and ran in the same way, using CMake. The following steps are required to build and run the code:

1. Navigate to either 'ref', 'opt-low-level' or 'opt-high-level' directory
2. Create and navigate to a build directory: `mkdir build && cd build`
3. Run CMake: `cmake -DPARAM=[Level] ..` where `[Level]` is either 'level1', 'level3', or 'level5'.
4. Build the requested target: `make [target]` where `[target]` is either 'test', 'profile', or 'benchmark'.
5. Run the target: `./[Level]/[release/debug]/[target]`

The 'test' target runs a basic test of all three algorithms (key generation, signing, and verification) and prints the results. The 'profile' target runs a performance test and prints the results. The 'benchmark' target runs an extensive benchmark and prints the results.
