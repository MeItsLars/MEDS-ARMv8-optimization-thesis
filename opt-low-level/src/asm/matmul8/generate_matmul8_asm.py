import os
import sys
from typing import List

dir_path = os.path.dirname(os.path.realpath(__file__))

R_C = "x0"
R_A = "x1"
R_B = "x2"
R_2o = "x3"
R_2n = "x4"
R_r = "x5"
R_c = "x6"
R_k = "x7"
R_Ai = "x8"
R_Bi = "x9"
R_T1 = "x10"
R_T1h = "w10"
R_T2 = "x11"
R_T2h = "w11"
R_T3 = "x12"
R_T3h = "w12"

class ARMNeonRegister:
    registers = []

    def __init__(self, index):
        self.id = None
        self.available = True
        self.index = index

    def claim(self, id):
        self.id = id
        self.available = False
    
    def release(self):
        self.id = None
        self.available = True
    
    def _h(self):
        return f"v{self.index}.h"
    
    def _2d(self):
        return f"v{self.index}.2d"
    
    def _2s(self):
        return f"v{self.index}.2s"

    def _4s(self):
        return f"v{self.index}.4s"
    
    def _8h(self):
        return f"v{self.index}.8h"

    def _4h(self):
        return f"v{self.index}.4h"

    @staticmethod
    def request(id):
        for register in ARMNeonRegister.registers:
            if register.available:
                register.claim(id)
                return register
        raise ValueError(f"No NEON registers available for request with id {id}")

    @staticmethod
    def get(id):
        for register in ARMNeonRegister.registers:
            if register.id == id:
                return register
        raise ValueError(f"No NEON register with id {id}")

for i in range(0, 32):
    ARMNeonRegister.registers.append(ARMNeonRegister(i))

def add(asm, indentation, s):
    asm.append("    " * indentation + s)

def add_nop(asm, indentation, s):
    # asm.append("    " * indentation + "# " + s)
    # asm.append("    " * indentation + "nop")
    add(asm, indentation, s)

def load_row_with_n_cols(asm, rn: ARMNeonRegister, rni, rni2, rninc, overflow, camount):
    """
    Adds assembly that loads a single matrix row consisting of 0 < n <= 8 elements into a NEON register.

    Parameters:
    - asm: The list of assembly instructions to add to
    - rn: The name of the NEON register to load the row into
    - rni: The name of the memory address to load the row from
    - rni2: The name of the memory address to increment to after loading the row
    - rninc: The amount to increment the memory address by after loading the row
             or None if the memory address should not be incremented
    - overflow: Whether it is okay to load 8 elements from memory, even if n < 8
    - camount: The number of elements in the row to load (n)
    """
    if camount > 7 or overflow:
        if rninc:
            if rni == rni2:
                add_nop(asm, 1, f"ld1 {{{rn._8h()}}}, [{rni}], {rninc}")
            else:
                add_nop(asm, 1, f"ld1 {{{rn._8h()}}}, [{rni}]")
                add_nop(asm, 1, f"add {rni2}, {rni}, {rninc}")
        else:
            add_nop(asm, 1, f"ld1 {{{rn._8h()}}}, [{rni}]")
        return

    # Fill the register with 0 first
    add_nop(asm, 1, f"dup {rn._8h()}, wzr")

    start = 0
    if camount > 3:
        # Load the first 4 elements in parallel
        add_nop(asm, 1, f"ld1 {{{rn._4h()}}}, [{rni}]")
        start = 4
    
    # Load the remaining elements one by one
    for i in range(start, camount):
        # Load the i-th lane of rn
        rn_p = rn._8h()[:-3]
        add_nop(asm, 1, f"ldrh {R_T3h}, [{rni}, #{i * 2}]")
        add_nop(asm, 1, f"ins {rn_p}.h[{i}], {R_T3h}")
    
    # Increment the memory address
    if rninc:
        add_nop(asm, 1, f"add {rni2}, {rni}, {rninc}")

def add_store_row_with_n_cols(asm, rn: ARMNeonRegister, rni, rni2, rninc, camount):
    """
    Adds assembly that stores a single matrix row consisting of 0 < n <= 8 elements from a NEON register into memory.

    Parameters:
    - asm: The list of assembly instructions to add to
    - rn: The name of the NEON register to store the row from
    - rni: The name of the memory address to store the row into
    - rni2: The name of the memory address to increment to after storing the row
    - rninc: The amount to increment the memory address by after storing the row
             or None if the memory address should not be incremented
    - camount: The number of elements in the row to store (n)
    """
    if camount > 7:
        if rninc:
            if rni == rni2:
                add_nop(asm, 1, f"st1 {{{rn._8h()}}}, [{rni}], {rninc}")
            else:
                add_nop(asm, 1, f"st1 {{{rn._8h()}}}, [{rni}]")
                add_nop(asm, 1, f"add {rni2}, {rni}, {rninc}")
        else:
            add_nop(asm, 1, f"st1 {{{rn._8h()}}}, [{rni}]")
        return
    
    start = 0
    if camount > 3:
        # Store the first 4 elements in parallel
        add_nop(asm, 1, f"st1 {{{rn._4h()}}}, [{rni}]")
        start = 4

    for i in range(start, camount):
        # Extract the i-th lane of rn   
        rn_p = rn._8h()[:-3]
        add_nop(asm, 1, f"umov {R_T3h}, {rn_p}.h[{i}]")
        # Store the i-th lane of rn
        add_nop(asm, 1, f"strh {R_T3h}, [{rni}, #{i * 2}]")
    
    # Increment the memory address
    if rninc:
        add_nop(asm, 1, f"add {rni}, {rni}, {rninc}")

def add_load_row(asm, id, i, neon_register, rni, rni2, rninc, ramount, camount):
    if i == 0:
        load_row_with_n_cols(asm, neon_register, rni, rni2, None if ramount == i + 1 else rninc, ramount > i + 1, camount)
    elif i == 7:
        load_row_with_n_cols(asm, neon_register, rni2, rni2, None, False, camount)
    else:
        load_row_with_n_cols(asm, neon_register, rni2, rni2, None if ramount == i + 1 else rninc, ramount > i + 1, camount)

def add_load_r_rows_c_cols(asm, id, rni, rni2, rninc, ramount, camount):
    used_registers = []
    for i in range(0, 8):
        if ramount <= i:
            break
        
        neon_register = ARMNeonRegister.request(f"{id}{i}")
        used_registers.append(neon_register)
        add_load_row(asm, id, i, neon_register, rni, rni2, rninc, ramount, camount)
        
    return used_registers

def add_mult(asm, initial, rn_Cl: ARMNeonRegister, rn_Ch: ARMNeonRegister, rn_A: ARMNeonRegister, B: List[ARMNeonRegister], kamount):
    for i in range(0, 8):
        if kamount <= i:
            break
        
        if i == 0 and initial:
            add_nop(asm, 1, f"umull {rn_Cl._4s()}, {B[i]._4h()}, {rn_A._h()}[{i}]")
            add_nop(asm, 1, f"umull2 {rn_Ch._4s()}, {B[i]._8h()}, {rn_A._h()}[{i}]")
        else:
            add_nop(asm, 1, f"umlal {rn_Cl._4s()}, {B[i]._4h()}, {rn_A._h()}[{i}]")
            add_nop(asm, 1, f"umlal2 {rn_Ch._4s()}, {B[i]._8h()}, {rn_A._h()}[{i}]")

def add_load_and_mult(asm, context, initial, Cl: List[ARMNeonRegister], Ch: List[ARMNeonRegister], pad_r, pad_c, pad_k):
    # Load B
    B = add_load_r_rows_c_cols(asm, "B", R_Bi, R_T2, R_2n,
                           context.k_pad_dist if pad_k else 8, 
                           context.c_pad_dist if pad_c else 8)
    # Add multiplications
    rn_Ai = ARMNeonRegister.get("Ai")
    for i in range(0, 8):
        if not pad_r or context.r_pad_dist > i:
            add_load_row(asm, "A", i, rn_Ai, R_Ai, R_T1, R_2o,
                              context.r_pad_dist if pad_r else 8,
                              context.k_pad_dist if pad_k else 8)
            add_mult(asm, initial, Cl[i], Ch[i], rn_Ai, B, context.k_pad_dist if pad_k else 8)
    
    # Release the B registers (A is released during mult)
    for register in B:
        register.release()

def add_store(asm, rn: List[ARMNeonRegister], rni, rni2, rninc, ramount, camount):
    for i in range(0, 8):
        if ramount <= i:
            break
        
        if i == 0:
            add_store_row_with_n_cols(asm, rn[i], rni, rni2, None if ramount == i + 1 else rninc, camount)
        elif i == 7:
            add_store_row_with_n_cols(asm, rn[i], rni2, rni2, None, camount)
        else:
            add_store_row_with_n_cols(asm, rn[i], rni2, rni2, None if ramount == i + 1 else rninc, camount)
    
    for i in range(len(rn)):
        rn[i].release()

def add_freeze_reduce_neon_16x4(asm, rn_src: ARMNeonRegister, rn_tmp1: ARMNeonRegister, rn_tmp2: ARMNeonRegister, shrink=False):
    # Compute val * magic
    rn_magic = ARMNeonRegister.get("magic")
    rn_medsp = ARMNeonRegister.get("medsp")
    add_nop(asm, 1, f"umull {rn_tmp1._2d()}, {rn_src._2s()}, {rn_magic._2s()}")
    add_nop(asm, 1, f"umull2 {rn_tmp2._2d()}, {rn_src._4s()}, {rn_magic._4s()}")

    # zip the results back into one register
    add_nop(asm, 1, f"uzp2 {rn_tmp1._4s()}, {rn_tmp1._4s()}, {rn_tmp2._4s()}")

    # shift right by 43 % 16 = 11 bits
    add_nop(asm, 1, f"ushr {rn_tmp1._4s()}, {rn_tmp1._4s()}, 11")

    # subtract
    add_nop(asm, 1, f"mls {rn_src._4s()}, {rn_tmp1._4s()}, {rn_medsp._4s()}")

    # shrink to 16 bits if necessary
    if shrink:
        add_nop(asm, 1, f"xtn {rn_src._4h()}, {rn_src._4s()}")

def add_reduce_combine(asm, rn_C, rn_Cl, rn_Ch):
    rn_T1 = ARMNeonRegister.request("T1")
    rn_T2 = ARMNeonRegister.request("T2")

    add_freeze_reduce_neon_16x4(asm, rn_Cl, rn_T1, rn_T2, False)
    add_freeze_reduce_neon_16x4(asm, rn_Ch, rn_T1, rn_T2, False)
    add_nop(asm, 1, f"uzp1 {rn_C._8h()}, {rn_Cl._8h()}, {rn_Ch._8h()}")

    rn_T1.release()
    rn_T2.release()

def add_k_loop(asm, context, pad_r, pad_c):
    loop_id = f"k_loop{'_pr' if pad_r else ''}{'_pc' if pad_c else ''}"

    # Initialize k
    add(asm, 1, f"mov {R_k}, #0")

    Cl = [ARMNeonRegister.request(f"C{i}l") for i in range(8)]
    Ch = [ARMNeonRegister.request(f"C{i}h") for i in range(8)]

    # This should always be true
    if context.k_size > 0:
        add(asm, 0, f"{loop_id}_1:")
        # Compute Ai = A + 2 * r * A_c + 2k (k = 0, so we can ignore that)
        add(asm, 1, f"madd {R_Ai}, {R_2o}, {R_r}, {R_A}")
        # Compute Bi = B + 2c + 2k * B_c    (k = 0, so we can ignore that)
        add(asm, 1, f"add {R_Bi}, {R_B}, {R_c}, lsl #1")
        # Multiply
        add_load_and_mult(asm, context, True, Cl, Ch, pad_r, pad_c, False)

        # Branch to end of k loop, but only if this is not the last iteration
        if context.k_size > 1:
            add(asm, 1, f"b {loop_id}_end")
    
    # If we have more than 1 iteration of k, we need to loop
    if context.k_size > 1:
        add(asm, 0, f"{loop_id}_2:")
        # Add 2k to Ai
        add(asm, 1, f"add {R_Ai}, {R_Ai}, #16")
        # Add 2kB_c to Bi
        add(asm, 1, f"add {R_Bi}, {R_Bi}, {R_2n}, lsl #3")
        # Multiply
        add_load_and_mult(asm, context, False, Cl, Ch, pad_r, pad_c, False)

        # Check if we are done. Otherwise, jump back to the beginning of the loop
        add(asm, 0, f"{loop_id}_end:")
        add(asm, 1, f"add {R_k}, {R_k}, #8")
        add(asm, 1, f"cmp {R_k}, #{context.k_size}")
        add(asm, 1, f"blt {loop_id}_2")
    
    # Padding time! If we need to pad k, do it here!
    if context.k_pad_dist > 0:
        add(asm, 0, f"{loop_id}_pk:")
        # Add 2k to Ai
        add(asm, 1, f"add {R_Ai}, {R_Ai}, #16")
        # Add 2kB_c to Bi
        add(asm, 1, f"add {R_Bi}, {R_Bi}, {R_2n}, lsl #3")
        # Multiply
        add_load_and_mult(asm, context, False, Cl, Ch, pad_r, pad_c, True)

    # Reduce Cl and Ch modulo MEDS_p, combine into C
    C = [ARMNeonRegister.request(f"C{i}") for i in range(8)]
    for i in range(0, 8):
        if not pad_r or context.r_pad_dist > i:
            add_reduce_combine(asm, C[i], Cl[i], Ch[i])

    # Store C back into memory
    # From C + 2rC_c + 2c, compute Ci = C + 2c (C is incremented with 2rC_c in each iteration)
    add(asm, 1, f"add {R_T1}, {R_C}, {R_c}, lsl #1")
    add_store(asm, C, R_T1, R_T1, R_2n, 
              context.r_pad_dist if pad_r else 8,
              context.c_pad_dist if pad_c else 8)
    
    # Release the registers
    for register in Cl:
        register.release()
    for register in Ch:
        register.release()

def add_c_loop(asm, context, pad_r):
    loop_id = f"c_loop{'_pr' if pad_r else ''}"
    add(asm, 1, f"mov {R_c}, #0")
    if context.c_size > 0:
        # Loop for while there are at least 4 columns remaining
        add(asm, 0, f"{loop_id}:")
        add_k_loop(asm, context, pad_r, False)
        add(asm, 0, f"{loop_id}_end:")
        add(asm, 1, f"add {R_c}, {R_c}, #8")
        add(asm, 1, f"cmp {R_c}, #{context.c_size}")
        add(asm, 1, f"blt {loop_id}")
    if context.c_pad_dist > 0:
        # Loop for the remaining columns if they exist
        add(asm, 0, f"{loop_id}_pc:")
        add_k_loop(asm, context, pad_r, True)
    

def add_r_loop(asm, context):
    loop_id = 'r_loop'
    add(asm, 1, f"mov {R_r}, #0")
    if context.r_size > 0:
        # Loop for while there are at least 4 rows remaining
        add(asm, 0, f"{loop_id}:")
        add_c_loop(asm, context, False)
        add(asm, 0, f"{loop_id}_end:")
        add(asm, 1, f"add {R_r}, {R_r}, #8")
        # We need Ci = C + 2C_c; therefore add 2rC_c to C (r is incremented by 8, so lsl #3)
        add(asm, 1, f"add {R_C}, {R_C}, {R_2n}, lsl #3")
        add(asm, 1, f"cmp {R_r}, #{context.r_size}")
        add(asm, 1, f"blt {loop_id}")
    if context.r_pad_dist > 0:
        # Loop for the remaining rows if they exist
        add(asm, 0, f"{loop_id}_pr:")
        add_c_loop(asm, context, True)
        add(asm, 0, f"{loop_id}_pr_end:")
    

def generate_mat_mul_asm(context, fun_id):
    if context.k_size == 0:
        raise ValueError("k must be at least 4")

    asm = []
    # Header and function information
    add(asm, 0, ".cpu cortex-a72")
    add(asm, 0, ".arch armv8-a")
    add(asm, 0, f".global {fun_id}")
    add(asm, 0, f"{fun_id}:")
    # Store v8-v15
    # Preserve v8-v15? Does not seem to be necessary
    # add(asm, 1, "stp q8, q9, [sp, #-32]!")
    # add(asm, 1, "stp q10, q11, [sp, #-32]!")
    # add(asm, 1, "stp q12, q13, [sp, #-32]!")
    # add(asm, 1, "stp q14, q15, [sp, #-32]!")
    # Initialize magic value (0x8018_0481 = -2145909631) into RN_MAGIC registers
    rn_magic = ARMNeonRegister.request("magic")
    add(asm, 1, f"mov {R_T1h}, 0x0481")
    add(asm, 1, f"movk {R_T1h}, 0x8018, lsl #16")
    add(asm, 1, f"dup {rn_magic._4s()}, {R_T1h}")
    # Initialize MEDS_p into RN_MEDSp
    rn_medsp = ARMNeonRegister.request("medsp")
    add(asm, 1, f"mov {R_T1h}, #{context.MEDS_p}")
    add(asm, 1, f"dup {rn_medsp._4s()}, {R_T1h}")
    # Claim a neon register below 16 for loading values of A[i]. This is required by the assembler.
    rn_Ai = ARMNeonRegister.request("Ai")
    # Store widened versions of m, o, and n
    add(asm, 1, f"mov {R_2o}, #{context.k * 2}")
    add(asm, 1, f"mov {R_2n}, #{context.c * 2}")
    add_r_loop(asm, context)
    # Restore v8-v15
    # add(asm, 1, "ldp q14, q15, [sp], #32")
    # add(asm, 1, "ldp q12, q13, [sp], #32")
    # add(asm, 1, "ldp q10, q11, [sp], #32")
    # add(asm, 1, "ldp q8, q9, [sp], #32")
    # Return
    add(asm, 1, "ret")
    rn_Ai.release()
    rn_medsp.release()
    rn_magic.release()
    return asm

# Get the directory that this python file is in
dir_path = os.path.dirname(os.path.realpath(__file__))

class Context:
    def __init__(self, r, c, k, MEDS_p, GFq_bits):
        self.r = r
        self.c = c
        self.k = k
        self.r_size = 8 * (r // 8)
        self.c_size = 8 * (c // 8)
        self.k_size = 8 * (k // 8)
        self.r_pad_dist = r - self.r_size
        self.c_pad_dist = c - self.c_size
        self.k_pad_dist = k - self.k_size
        self.MEDS_p = MEDS_p
        self.GFq_bits = GFq_bits

    def __str__(self):
        return f"r: {self.r}, c: {self.c}, k: {self.k}, r_size: {self.r_size}, c_size: {self.c_size}, k_size: {self.k_size}, r_pad_dist: {self.r_pad_dist}, c_pad_dist: {self.c_pad_dist}, k_pad_dist: {self.k_pad_dist}, MEDS_p: {self.MEDS_p}, GFq_bits: {self.GFq_bits}"

def generate_matmul_file(name, r, c, k, MEDS_p, GFq_bits, fun_id):
    asm = generate_mat_mul_asm(Context(r, c, k, MEDS_p, GFq_bits), fun_id)
    path = f"{dir_path}/{name}/{fun_id}.s"
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, "w") as f:
        for line in asm:
            f.write(line + "\n")
    print(f'Generated {fun_id}.s ({r}, {c}, {k}, {MEDS_p}, {GFq_bits})')
    for register in ARMNeonRegister.registers:
        if not register.available:
            print(f"Warning: NEON register {register.index} is still in use")

def generate_matmul_manually():
    generate_matmul_file('level3', 8, 8, 8, 4093, 12, 'pmod_mat_mul_8_asm_8_8_8')
    generate_matmul_file('level3', 13, 8, 8, 4093, 12, 'pmod_mat_mul_8_asm_13_8_8')
    generate_matmul_file('level3', 8, 13, 8, 4093, 12, 'pmod_mat_mul_8_asm_8_13_8')
    generate_matmul_file('level3', 8, 8, 13, 4093, 12, 'pmod_mat_mul_8_asm_8_8_13')
    generate_matmul_file('level3', 13, 13, 13, 4093, 12, 'pmod_mat_mul_8_asm_13_13_13')
    generate_matmul_file('level3', 16, 16, 16, 4093, 12, 'pmod_mat_mul_8_asm_16_16_16')
    generate_matmul_file('level3', 24, 24, 24, 4093, 12, 'pmod_mat_mul_8_asm_24_24_24')

def parse_params_h(path):
    with open(path, "r") as f:
        lines = f.readlines()
        for line in lines:
            if line.startswith('#define MEDS_name '):
                name = line.split('#define MEDS_name ')[1].rstrip('\n')[1:-1]
            if line.startswith('#define MEDS_m '):
                m = int(line.split('#define MEDS_m ')[1])
            if line.startswith('#define MEDS_n '):
                n = int(line.split('#define MEDS_n ')[1])
            if line.startswith('#define MEDS_k '):
                k = int(line.split('#define MEDS_k ')[1])
            if line.startswith('#define MEDS_p '):
                MEDS_p = int(line.split('#define MEDS_p ')[1])
            if line.startswith('#define GFq_bits '):
                GFq_bits = int(line.split('#define GFq_bits ')[1])
    return (name, m, n, k, MEDS_p, GFq_bits)

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python generate_matmul_asm.py <params.h | 'none'>")
        sys.exit(1)
    
    params_h_path = sys.argv[1]

    if params_h_path == "none":
        generate_matmul_manually()
    else:
        name, m, n, k, MEDS_p, GFq_bits = parse_params_h(params_h_path)
        # Generate the 6 different multiplications required for MEDS
        generate_matmul_file(name, k, m*n, k, MEDS_p, GFq_bits, f'pmod_mat_mul_8_asm_k_mn_k')
        generate_matmul_file(name, 2, m*n, k, MEDS_p, GFq_bits, f'pmod_mat_mul_8_asm_2_mn_k')
        generate_matmul_file(name, 2, k, k, MEDS_p, GFq_bits, f'pmod_mat_mul_8_asm_2_k_k')
        generate_matmul_file(name, m, n, m, MEDS_p, GFq_bits, f'pmod_mat_mul_8_asm_m_n_m')
        generate_matmul_file(name, m, n, n, MEDS_p, GFq_bits, f'pmod_mat_mul_8_asm_m_n_n')
    