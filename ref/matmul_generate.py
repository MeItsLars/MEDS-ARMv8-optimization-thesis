import os

# R_C = "x0"
# R_A = "x1"
# R_B = "x2"
# R_2m = "x3"
# R_2o = "x4"
# R_2n = "x5"
# R_r = "x6"
# R_c = "x7"
# R_k = "x8"
# R_Ai = "x9"
# R_Bi = "x10"
# R_T1 = "x11"
# R_T1h = "w11"

# RN_A0 = "v0.4h"
# RN_A1 = "v1.4h"
# RN_A2 = "v2.4h"
# RN_A3 = "v3.4h"
# RN_B0 = "v4.4h"
# RN_B1 = "v5.4h"
# RN_B2 = "v6.4h"
# RN_B3 = "v7.4h"
# RN_C0 = "v8.4s"
# RN_C1 = "v9.4s"
# RN_C2 = "v10.4s"
# RN_C3 = "v11.4s"
# RN_C0h = "v8.4h"
# RN_C1h = "v9.4h"
# RN_C2h = "v10.4h"
# RN_C3h = "v11.4h"
# RN_C0T = "v12.4s"
# RN_C1T = "v13.4s"
# RN_C2T = "v14.4s"
# RN_C3T = "v15.4s"
# RN_MEDSp = "v16.4s"

R_C = "x0"
R_A = "x1"
R_B = "x2"
R_2m = "x3"
R_2o = "x4"
R_2n = "x5"
R_r = "x9"
R_c = "x10"
R_k = "x11"
R_Ai = "x12"
R_Bi = "x13"
R_T1 = "x15"
R_T1h = "w15"

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

def add_load_store(asm, ins, rn1, rn2, rn3, rn4, rni, rni2, rninc):
    if rni == rni2:
        add(asm, 1, f"{ins} {{{rn1}}}, [{rni}], {rninc}")
    else:
        add(asm, 1, f"{ins} {{{rn1}}}, [{rni}]")
        add(asm, 1, f"add {rni2}, {rni}, {rninc}")
    add(asm, 1, f"{ins} {{{rn2}}}, [{rni2}], {rninc}")
    add(asm, 1, f"{ins} {{{rn3}}}, [{rni2}], {rninc}")
    add(asm, 1, f"{ins} {{{rn4}}}, [{rni2}]")

def add_load(asm, rn1, rn2, rn3, rn4, rni, rni2, rninc):
    add_load_store(asm, "ld1", rn1, rn2, rn3, rn4, rni, rni2, rninc)

def add_store(asm, rn1, rn2, rn3, rn4, rni, rni2, rninc):
    add_load_store(asm, "st1", rn1, rn2, rn3, rn4, rni, rni2, rninc)

def add_reduce(asm, rn_src, rn_tmp, rn_dst, GFq_bits):
    add(asm, 1, f"ushr {rn_tmp}, {rn_src}, #{GFq_bits}")
    add(asm, 1, f"mul {rn_tmp}, {rn_tmp}, {RN_MEDSp}")
    add(asm, 1, f"sub {rn_src}, {rn_src}, {rn_tmp}")
    add(asm, 1, f"ushr {rn_tmp}, {rn_src}, #{GFq_bits}")
    add(asm, 1, f"mul {rn_tmp}, {rn_tmp}, {RN_MEDSp}")
    add(asm, 1, f"sub {rn_src}, {rn_src}, {rn_tmp}")
    add(asm, 1, f"sqxtn {rn_dst}, {rn_src}")

def add_mult(asm, initial, rn_C, rn_A):
    ins = "umull" if initial else "umlal"
    add(asm, 1, f"{ins} {rn_C}, {RN_B0}, {rn_A}[0]")
    add(asm, 1, f"umlal {rn_C}, {RN_B1}, {rn_A}[1]")
    add(asm, 1, f"umlal {rn_C}, {RN_B2}, {rn_A}[2]")
    add(asm, 1, f"umlal {rn_C}, {RN_B3}, {rn_A}[3]")

def add_load_and_mult(asm, initial):
    # Load A
    add_load(asm, RN_A0, RN_A1, RN_A2, RN_A3, R_Ai, R_T1, R_2o)
    # Load B
    add_load(asm, RN_B0, RN_B1, RN_B2, RN_B3, R_Bi, R_T1, R_2n)
    # Add multiplications
    add_mult(asm, initial, RN_C0, RN_A0)
    add_mult(asm, initial, RN_C1, RN_A1)
    add_mult(asm, initial, RN_C2, RN_A2)
    add_mult(asm, initial, RN_C3, RN_A3)
    return

def add_k_loop(asm, m, o, n, MEDS_p, GFq_bits):
    add(asm, 1, f"mov {R_k}, #0")

    add(asm, 0, f"k_loop_1:")
    # Compute Ai = A + 2 * r * A_c + 2k (k = 0, so we can ignore that)
    add(asm, 1, f"madd {R_Ai}, {R_2o}, {R_r}, {R_A}")
    # Compute Bi = B + 2c + 2k * B_c    (k = 0, so we can ignore that)
    add(asm, 1, f"add {R_Bi}, {R_B}, {R_c}, lsl #1")
    # Multiply
    add_load_and_mult(asm, True)
    # Branch to end of k loop
    add(asm, 1, f"b k_loop_end")
    
    add(asm, 0, f"k_loop_2:")
    # Add 2k to Ai
    add(asm, 1, f"add {R_Ai}, {R_Ai}, #8")
    # Add 2kB_c to Bi
    add(asm, 1, f"add {R_Bi}, {R_Bi}, {R_2n}, lsl #2")
    # Multiply
    add_load_and_mult(asm, False)

    add(asm, 0, f"k_loop_end:")
    add(asm, 1, f"add {R_k}, {R_k}, #4")
    add(asm, 1, f"cmp {R_k}, #{o}")
    add(asm, 1, f"blt k_loop_2")
    # Reduce C modulo MEDS_p
    add_reduce(asm, RN_C0, RN_C0T, RN_C0h, GFq_bits)
    add_reduce(asm, RN_C1, RN_C1T, RN_C1h, GFq_bits)
    add_reduce(asm, RN_C2, RN_C2T, RN_C2h, GFq_bits)
    add_reduce(asm, RN_C3, RN_C3T, RN_C3h, GFq_bits)
    # Store C back into memory
    # From C + 2rC_c + 2c, compute Ci = C + 2c (C is incremented with 2rC_c in each iteration)
    add(asm, 1, f"add {R_T1}, {R_C}, {R_c}, lsl #1")
    add_store(asm, RN_C0h, RN_C1h, RN_C2h, RN_C3h, R_T1, R_T1, R_2n)

def add_c_loop(asm, m, o, n, MEDS_p, GFq_bits):
    add(asm, 1, f"mov {R_c}, #0")
    add(asm, 0, f"c_loop:")
    add_k_loop(asm, m, o, n, MEDS_p, GFq_bits)
    add(asm, 0, f"c_loop_end:")
    add(asm, 1, f"add {R_c}, {R_c}, #4")
    add(asm, 1, f"cmp {R_c}, #{n}")
    add(asm, 1, f"blt c_loop")

def add_r_loop(asm, m, o, n, MEDS_p, GFq_bits):
    add(asm, 1, f"mov {R_r}, #0")
    add(asm, 0, f"r_loop:")
    add_c_loop(asm, m, o, n, MEDS_p, GFq_bits)
    add(asm, 0, f"r_loop_end:")
    add(asm, 1, f"add {R_r}, {R_r}, #4")
    # We need Ci = C + 2rC_c; therefore add 2rC_c to C
    add(asm, 1, f"add {R_C}, {R_C}, {R_2n}, lsl #2")
    add(asm, 1, f"cmp {R_r}, #{m}")
    add(asm, 1, f"blt r_loop")

def generate_mat_mul_asm(m, o, n, MEDS_p, GFq_bits):
    asm = []
    # Header and function information
    add(asm, 0, ".cpu cortex-a72")
    add(asm, 0, ".arch armv8-a")
    fun_id = f"pmod_mat_mul_asm_{m}_{o}_{n}"
    add(asm, 0, f".global {fun_id}")
    add(asm, 0, f"{fun_id}:")
    # Initialize MEDS_p into v16.4s
    add(asm, 1, f"mov {R_T1h}, #{MEDS_p}")
    add(asm, 1, f"dup {RN_MEDSp}, {R_T1h}")
    # Store widened versions of m, o, and n
    add(asm, 1, f"mov {R_2m}, #{m * 2}")
    add(asm, 1, f"mov {R_2o}, #{o * 2}")
    add(asm, 1, f"mov {R_2n}, #{n * 2}")
    add_r_loop(asm, m, o, n, MEDS_p, GFq_bits)
    add(asm, 1, "ret")
    return (fun_id, asm)

# Get the directory that this python file is in
dir_path = os.path.dirname(os.path.realpath(__file__))

fun_id, asm = generate_mat_mul_asm(24, 24*24, 24, 4093, 12)
with open(f"{dir_path}/{fun_id}.s", "w") as f:
    for line in asm:
        f.write(line + "\n")