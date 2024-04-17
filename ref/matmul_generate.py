import os

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
R_T2h = "w11"

RN_A0 = "v0.4h"
RN_A1 = "v1.4h"
RN_A2 = "v2.4h"
RN_A3 = "v3.4h"
RN_B0 = "v4.4h"
RN_B1 = "v5.4h"
RN_B2 = "v6.4h"
RN_B3 = "v7.4h"
RN_C0 = "v8.4s"
RN_C1 = "v9.4s"
RN_C2 = "v10.4s"
RN_C3 = "v11.4s"
RN_C0h = "v8.4h"
RN_C1h = "v9.4h"
RN_C2h = "v10.4h"
RN_C3h = "v11.4h"
RN_C0T = "v12.4s"
RN_C1T = "v13.4s"
RN_C2T = "v14.4s"
RN_C3T = "v15.4s"
RN_MEDSp = "v16.4s"

def add(asm, indentation, s):
    asm.append("    " * indentation + s)

def add_load_n_full_rows(asm, rn1, rn2, rn3, rn4, rni, rni2, rninc, ramount):
    # If we only need to load 1 row, be done with it
    if ramount == 1:
        add(asm, 1, f"ld1 {{{rn1}}}, [{rni}]")
        return

    # Otherwise, check the rni/rni2 difference
    if rni == rni2:
        add(asm, 1, f"ld1 {{{rn1}}}, [{rni}], {rninc}")
    else:
        add(asm, 1, f"ld1 {{{rn1}}}, [{rni}]")
        add(asm, 1, f"add {rni2}, {rni}, {rninc}")
    if ramount > 1:
        add(asm, 1, f"ld1 {{{rn2}}}, [{rni2}], {rninc}")
    if ramount > 2:
        add(asm, 1, f"ld1 {{{rn3}}}, [{rni2}], {rninc}")
    if ramount > 3:
        add(asm, 1, f"ld1 {{{rn4}}}, [{rni2}]")

def add_load_n_full_cols(asm, rn1, rn2, rn3, rn4, rni, rni2, rninc, camount):
    # This is a bit harder, because we can't use a full NEON load instruction
    # Instead, we need to 'cut off' the loading at 'camount'
    # We can ignore this for the first 'ramount-1' rows, because those memory addresses will definitely be valid
    # even though we will load data that we don't need. The 4th row will need to be cut off

    # Load first row of 'camount' columns
    if rni == rni2:
        add(asm, 1, f"ld1 {{{rn1}}}, [{rni}], {rninc}")
    else:
        add(asm, 1, f"ld1 {{{rn1}}}, [{rni}]")
        add(asm, 1, f"add {rni2}, {rni}, {rninc}")
    # Load second row of 'camount' columns
    add(asm, 1, f"ld1 {{{rn2}}}, [{rni2}], {rninc}")
    # Load third row of 'camount' columns
    add(asm, 1, f"ld1 {{{rn3}}}, [{rni2}], {rninc}")
    # Load fourth row of 'camount' columns without a direct NEON instruction
    if camount > 3:
        add(asm, 1, f"ld1 {{{rn4}}}, [{rni2}]")
        return

    # Load the elements one by one
    # 1. Clear rn4 so that it becomes 0
    add(asm, 1, f"dup {rn4}, wzr")
    # 2. Load 'camount' columns into the lanes of rn4
    rn4_p = rn4[:-3]
    if camount > 0:
        add(asm, 1, f"ldrh {R_T2h}, [{rni2}]")
        add(asm, 1, f"ins {rn4_p}.h[0], {R_T2h}")
    if camount > 1:
        add(asm, 1, f"ldrh {R_T2h}, [{rni2}, #2]")
        add(asm, 1, f"ins {rn4_p}.h[1], {R_T2h}")
    if camount > 2:
        add(asm, 1, f"ldrh {R_T2h}, [{rni2}, #4]")
        add(asm, 1, f"ins {rn4_p}.h[2], {R_T2h}")

def add_store_row_n_cols(asm, rn, rni, rninc, camount):
    inc = f", {rninc}" if rninc != None else ""
    if camount > 3:
        add(asm, 1, f"st1 {{{rn}}}, [{rni}]{inc}")
        return

    for i in range(camount):
        # Extract the i-th lane of rn   
        rn_p = rn[:-3]
        add(asm, 1, f"mov {R_T2h}, {rn_p}.s[{i}]")
        # Store the i-th lane of rn
        add(asm, 1, f"strh {R_T2h}, [{rni}, #{i * 2}]")
    
    # Increment the memory address
    if rninc:
        add(asm, 1, f"add {rni}, {rni}, {rninc}")

def add_store(asm, rn1, rn2, rn3, rn4, rni, rni2, rninc, ramount, camount):
    if ramount > 0:
        if rni == rni2:
            # add(asm, 1, f"st1 {{{rn1}}}, [{rni}], {rninc}")
            add_store_row_n_cols(asm, rn1, rni, None if ramount == 1 else rninc, camount)
        else:
            # add(asm, 1, f"st1 {{{rn1}}}, [{rni}]")
            add_store_row_n_cols(asm, rn1, rni, None, camount)
            if ramount > 1:
                add(asm, 1, f"add {rni2}, {rni}, {rninc}")
    if ramount > 1:
        # add(asm, 1, f"st1 {{{rn2}}}, [{rni2}], {rninc}")
        add_store_row_n_cols(asm, rn2, rni2, None if ramount == 2 else rninc, camount)
    if ramount > 2:
        # add(asm, 1, f"st1 {{{rn3}}}, [{rni2}], {rninc}")
        add_store_row_n_cols(asm, rn3, rni2, None if ramount == 3 else rninc, camount)
    if ramount > 3:
        # add(asm, 1, f"st1 {{{rn4}}}, [{rni2}]")
        add_store_row_n_cols(asm, rn4, rni2, None, camount)

def add_reduce(asm, rn_src, rn_tmp, rn_dst, GFq_bits, final_shrink):
    add(asm, 1, f"ushr {rn_tmp}, {rn_src}, #{GFq_bits}")
    add(asm, 1, f"mul {rn_tmp}, {rn_tmp}, {RN_MEDSp}")
    add(asm, 1, f"sub {rn_src}, {rn_src}, {rn_tmp}")
    add(asm, 1, f"ushr {rn_tmp}, {rn_src}, #{GFq_bits}")
    add(asm, 1, f"mul {rn_tmp}, {rn_tmp}, {RN_MEDSp}")
    add(asm, 1, f"sub {rn_src}, {rn_src}, {rn_tmp}")
    if final_shrink:
        add(asm, 1, f"sqxtn {rn_dst}, {rn_src}")

def add_mult(asm, initial, rn_C, rn_A):
    ins = "umull" if initial else "umlal"
    add(asm, 1, f"{ins} {rn_C}, {RN_B0}, {rn_A}[0]")
    add(asm, 1, f"umlal {rn_C}, {RN_B1}, {rn_A}[1]")
    add(asm, 1, f"umlal {rn_C}, {RN_B2}, {rn_A}[2]")
    add(asm, 1, f"umlal {rn_C}, {RN_B3}, {rn_A}[3]")

def add_load_and_mult(asm, context, initial, pad_r, pad_c):
    # Load A
    add_load_n_full_rows(asm, RN_A0, RN_A1, RN_A2, RN_A3, R_Ai, R_T1, R_2o, context.r_pad_dist if pad_r else 4)
    # Load B
    add_load_n_full_cols(asm, RN_B0, RN_B1, RN_B2, RN_B3, R_Bi, R_T1, R_2n, context.c_pad_dist if pad_c else 4)
    # Add multiplications
    if not pad_r or context.r_pad_dist > 0:
        add_mult(asm, initial, RN_C0, RN_A0)
    if not pad_r or context.r_pad_dist > 1:
        add_mult(asm, initial, RN_C1, RN_A1)
    if not pad_r or context.r_pad_dist > 2:
        add_mult(asm, initial, RN_C2, RN_A2)
    if not pad_r or context.r_pad_dist > 3:
        add_mult(asm, initial, RN_C3, RN_A3)

def add_k_loop(asm, context, pad_r, pad_c):
    loop_id = f"k_loop{'_pr' if pad_r else ''}{'_pc' if pad_c else ''}"

    # Incrment k
    add(asm, 1, f"mov {R_k}, #0")

    add(asm, 0, f"{loop_id}_1:")
    # Compute Ai = A + 2 * r * A_c + 2k (k = 0, so we can ignore that)
    add(asm, 1, f"madd {R_Ai}, {R_2o}, {R_r}, {R_A}")
    # Compute Bi = B + 2c + 2k * B_c    (k = 0, so we can ignore that)
    add(asm, 1, f"add {R_Bi}, {R_B}, {R_c}, lsl #1")
    # Multiply
    add_load_and_mult(asm, context, True, pad_r, pad_c)
    # Branch to end of k loop
    add(asm, 1, f"b {loop_id}_end")
    
    add(asm, 0, f"{loop_id}_2:")
    # Add 2k to Ai
    add(asm, 1, f"add {R_Ai}, {R_Ai}, #8")
    # Add 2kB_c to Bi
    add(asm, 1, f"add {R_Bi}, {R_Bi}, {R_2n}, lsl #2")
    # Multiply
    add_load_and_mult(asm, context, False, pad_r, pad_c)

    add(asm, 0, f"{loop_id}_end:")
    add(asm, 1, f"add {R_k}, {R_k}, #4")
    add(asm, 1, f"cmp {R_k}, #{context.k_size}")
    add(asm, 1, f"blt {loop_id}_2")
    # Reduce C modulo MEDS_p
    if not pad_r or context.r_pad_dist > 0:
        add_reduce(asm, RN_C0, RN_C0T, RN_C0h, context.GFq_bits, not pad_c)
    if not pad_r or context.r_pad_dist > 1:
        add_reduce(asm, RN_C1, RN_C1T, RN_C1h, context.GFq_bits, not pad_c)
    if not pad_r or context.r_pad_dist > 2:
        add_reduce(asm, RN_C2, RN_C2T, RN_C2h, context.GFq_bits, not pad_c)
    if not pad_r or context.r_pad_dist > 3:
        add_reduce(asm, RN_C3, RN_C3T, RN_C3h, context.GFq_bits, not pad_c)
    # Store C back into memory
    # From C + 2rC_c + 2c, compute Ci = C + 2c (C is incremented with 2rC_c in each iteration)
    add(asm, 1, f"add {R_T1}, {R_C}, {R_c}, lsl #1")
    add_store(asm, RN_C0h, RN_C1h, RN_C2h, RN_C3h, R_T1, R_T1, R_2n, 
              context.r_pad_dist if pad_r else 4,
              context.c_pad_dist if pad_c else 4)

def add_c_loop(asm, context, pad_r):
    loop_id = f"c_loop{'_pr' if pad_r else ''}"
    add(asm, 1, f"mov {R_c}, #0")
    if context.c_size > 0:
        # Loop for while there are at least 4 columns remaining
        add(asm, 0, f"{loop_id}:")
        add_k_loop(asm, context, pad_r, False)
        add(asm, 0, f"{loop_id}_end:")
        add(asm, 1, f"add {R_c}, {R_c}, #4")
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
        add(asm, 1, f"add {R_r}, {R_r}, #4")
        # We need Ci = C + 2rC_c; therefore add 2rC_c to C
        add(asm, 1, f"add {R_C}, {R_C}, {R_2n}, lsl #2")
        add(asm, 1, f"cmp {R_r}, #{context.r_size}")
        add(asm, 1, f"blt {loop_id}")
    if context.r_pad_dist > 0:
        # Loop for the remaining rows if they exist
        add(asm, 0, f"{loop_id}_pr:")
        add_c_loop(asm, context, True)
        add(asm, 0, f"{loop_id}_pr_end:")
    

def generate_mat_mul_asm(context):
    asm = []
    # Header and function information
    add(asm, 0, ".cpu cortex-a72")
    add(asm, 0, ".arch armv8-a")
    fun_id = f"pmod_mat_mul_asm_{context.r}_{context.k}_{context.c}"
    add(asm, 0, f".global {fun_id}")
    add(asm, 0, f"{fun_id}:")
    # Initialize MEDS_p into v16.4s
    add(asm, 1, f"mov {R_T1h}, #{context.MEDS_p}")
    add(asm, 1, f"dup {RN_MEDSp}, {R_T1h}")
    # Store widened versions of m, o, and n
    add(asm, 1, f"mov {R_2o}, #{context.k * 2}")
    add(asm, 1, f"mov {R_2n}, #{context.c * 2}")
    add_r_loop(asm, context)
    add(asm, 1, "ret")
    return (fun_id, asm)

# Get the directory that this python file is in
dir_path = os.path.dirname(os.path.realpath(__file__))

class Context:
    def __init__(self, r, c, k, MEDS_p, GFq_bits):
        self.r = r
        self.c = c
        self.k = k
        self.r_size = 4 * (r // 4)
        self.c_size = 4 * (c // 4)
        self.k_size = 4 * (k // 4)
        self.r_pad_dist = r - self.r_size
        self.c_pad_dist = c - self.c_size
        self.k_pad_dist = k - self.k_size
        self.MEDS_p = MEDS_p
        self.GFq_bits = GFq_bits

    def __str__(self):
        return f"r: {self.r}, c: {self.c}, k: {self.k}, r_size: {self.r_size}, c_size: {self.c_size}, k_size: {self.k_size}, r_pad_dist: {self.r_pad_dist}, c_pad_dist: {self.c_pad_dist}, k_pad_dist: {self.k_pad_dist}, MEDS_p: {self.MEDS_p}, GFq_bits: {self.GFq_bits}"

# for r in range(1, 8):
#     fun_id, asm = generate_mat_mul_asm(Context(r, 4, 4, 4093, 12))
#     with open(f"{dir_path}/{fun_id}.s", "w") as f:
#         for line in asm:
#             f.write(line + "\n")

#     print(f"Generated {fun_id}.s")
        
# for c in range(1, 8):
#     fun_id, asm = generate_mat_mul_asm(Context(4, c, 4, 4093, 12))
#     with open(f"{dir_path}/{fun_id}.s", "w") as f:
#         for line in asm:
#             f.write(line + "\n")

#     print(f"Generated {fun_id}.s")

fun_id, asm = generate_mat_mul_asm(Context(24, 67, 4, 4093, 12))
with open(f"{dir_path}/{fun_id}.s", "w") as f:
    for line in asm:
        f.write(line + "\n")

print(f"Generated {fun_id}.s")

fun_id, asm = generate_mat_mul_asm(Context(4, 4, 4, 4093, 12))
with open(f"{dir_path}/{fun_id}.s", "w") as f:
    for line in asm:
        f.write(line + "\n")

print(f"Generated {fun_id}.s")