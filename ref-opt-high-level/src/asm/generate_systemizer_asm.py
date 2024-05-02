import os
import sys

dir_path = os.path.dirname(os.path.realpath(__file__))

R_M = "x0"
R_Mr = "x1"
R_Mc = "x2"
R_max_r = "x3"
R_loop1 = "x4"
R_loop2 = "x5"
R_loop3 = "x6"
R_Mrc_i = "x7"
R_Mr2c_i = "x8"
R_T1 = "x11"
R_T1h = "w11"

RN_ret = "v0.4h"
RN_MEDSp = "v1.4h"
RN_MEDSpw = "v1.4s"
RN_zero = "v2.4h"
RN_Mrr = "v3.4h"
RN_Mrr_d = "d3"
RN_Mrr0 = "v4.4h"
RN_Mrc = "v5.4h"
RN_Mrc_d = "d5"
RN_Mr2c = "v6.4h"
RN_Mr2c_d = "d6"
RN_T1 = "v7.4h"
RN_T1w = "v7.4s"
RN_T2_d = "d8"
RN_T2 = "v8.4h"
RN_T2w = "v8.4s"

def add(asm, indentation, s):
    asm.append("    " * indentation + s)

def add_loop(asm, loop_name, loop_var, loop_from, loop_to, direction, loop_code_fun):
    # Initialize loop
    if direction == 0:
        add(asm, 1, f"mov {loop_var}, {loop_from}")
    elif direction == 1:
        add(asm, 1, f"add {loop_var}, {loop_from}, #1")
    else:
        add(asm, 1, f"sub {loop_var}, {loop_from}, #1")
    add(asm, 0, f"{loop_name}:")
    add(asm, 1, f"cmp {loop_var}, {loop_to}")
    if direction >= 0:
        add(asm, 1, f"b.eq {loop_name}_end")
    else:
        add(asm, 1, f"b.lt {loop_name}_end")
    # Execute loop code
    loop_code_fun()
    # Increment loop variable and loop
    if direction >= 0:
        add(asm, 1, f"add {loop_var}, {loop_var}, #1")
    else:
        add(asm, 1, f"sub {loop_var}, {loop_var}, #1")
    add(asm, 1, f"b {loop_name}")
    # Loop end
    add(asm, 0, f"{loop_name}_end:")

def add_reduce(asm, rn_src, rn_tmp, rn_dst, GFq_bits):
    # Apply two reductions
    add(asm, 1, f"ushr {rn_tmp}, {rn_src}, #{GFq_bits}")
    add(asm, 1, f"mul {rn_tmp}, {rn_tmp}, {RN_MEDSpw}")
    add(asm, 1, f"sub {rn_src}, {rn_src}, {rn_tmp}")
    add(asm, 1, f"ushr {rn_tmp}, {rn_src}, #{GFq_bits}")
    add(asm, 1, f"mul {rn_tmp}, {rn_tmp}, {RN_MEDSpw}")
    add(asm, 1, f"sub {rn_src}, {rn_src}, {rn_tmp}")
    # Remove one final MEDS_p if the value in the lane is at least MEDS_p
    add(asm, 1, f"cmhs {rn_tmp}, {rn_src}, {RN_MEDSpw}")
    add(asm, 1, f"and {rn_tmp[:-3]}.16b, {rn_tmp[:-3]}.16b, {RN_MEDSpw[:-3]}.16b")
    add(asm, 1, f"sub {rn_src}, {rn_src}, {rn_tmp}")
    # Shrink to 16 bits
    add(asm, 1, f"sqxtn {rn_dst}, {rn_src}")

def add_swap(context, asm):
    return

def add_elimination_row_zero_fix_loop2(context, asm):
    # Load Mrc = M[r][c] and Mr2c = M[r2][c]
    add(asm, 1, f"ldr {RN_Mrc_d}, [{R_Mrc_i}], #8") # + #8 is done at the store in the end of this loop
    add(asm, 1, f"ldr {RN_Mr2c_d}, [{R_Mr2c_i}], #8")
    # Compute RN_T1 = Mr2c if Mrr is zero, else zero
    add(asm, 1, f"and {RN_T1[:-3]}.16b, {RN_Mrr0[:-3]}.16b, {RN_Mr2c[:-3]}.16b")
    # Compute RN_T2w = RN_T1 + Mrc
    add(asm, 1, f"uaddl {RN_T2w}, {RN_T1}, {RN_Mrc}")
    # Reduce RN_T2w
    add_reduce(asm, RN_T2w, RN_T1w, RN_T2, context.GFq_bits)
    # Store the result into M[r][c]
    add(asm, 1, f"str {RN_T2_d}, [{R_Mrc_i}], #8")

def add_elimination_row_zero_fix_loop1(context, asm):
    # Load R_Mcr2 = 8 * M_c * r2 + 8 * r
    add(asm, 1, f"madd {R_Mr2c_i}, {R_Mc}, {R_loop2}, {R_loop1}")
    add(asm, 1, f"mov {R_Mr2c_i}, {R_Mr2c_i}, lsl #3")
    # Loop
    add_loop(asm, "elimination_row_zero_fix_inner_loop", R_loop3, R_loop1, R_Mc, 0, lambda: add_elimination_row_zero_fix_loop2(context, asm))

def add_return_if_zero(context, asm):
    return

def add_compute_inverse(context, asm):
    return

def add_normalize_row(context, asm):
    return

def add_eliminate_rows(context, asm):
    return

def add_elimination_loop(context, asm):
    # Add swap code if it is needed
    if context.swap:
        add_swap(context, asm)
    
    # Add the loop that makes sure the current M[r][r] is not zero
    # First, load M[r][r], indexed by M_c * r + r
    add(asm, 1, f"madd {R_T1}, {R_Mc}, {R_loop1}, {R_loop1}")
    add(asm, 1, f"ldr {RN_Mrr_d}, [{R_M}, {R_T1}, lsl #3]")
    # Next, load R_Mcr = 8 * M_c * r + 8 * r
    add(asm, 1, f"madd {R_Mrc_i}, {R_Mc}, {R_loop1}, {R_loop1}")
    add(asm, 1, f"mov {R_Mrc_i}, {R_Mrc_i}, lsl #3")
    # Next, check if M[r][r] is zero
    add(asm, 1, f"cmeq {RN_Mrr0}, {RN_Mrr}, {RN_zero}")
    # Add the loop
    add_loop(asm, "elimination_row_zero_fix_outer_loop", R_loop2, R_loop1, R_Mr, 1, lambda: add_elimination_row_zero_fix_loop1(context, asm))

    # If, after attempting to make M[r][r] non-zero, it is still zero, move -1 into the return value
    add_return_if_zero(context, asm)

    # Compute the multiplicative inverse of M[r][r]
    add_compute_inverse(context, asm)

    # Normalize the current row of M by multiplying it by the inverse of M[r][r] (causing M[r][r] to become 1)
    add_loop(asm, "elimination_normalize_row_loop", R_loop2, R_loop1, R_Mc, 0, lambda: add_normalize_row(context, asm))

    # Eliminate the rows below the current one (so that M[r + x][c] becomes 0)
    add_loop(asm, "elimination_eliminate_rows_loop", R_loop2, R_loop1, R_Mr, 1, lambda: add_eliminate_rows(context, asm))

def add_backsub_loop(context, asm):
    return

def generate_systemizer_asm(context, fun_id):
    asm = []
    # Header and function information
    add(asm, 0, ".cpu cortex-a72")
    add(asm, 0, ".arch armv8-a")
    add(asm, 0, f".global {fun_id}")
    add(asm, 0, f"{fun_id}:")
    # Set max_r (if it is not an argument to the function)
    if not context.use_max_r:
        add(asm, 1, f"mov {R_max_r}, {R_Mr}")
    # Initialize MEDS_p into RN_MEDSp
    add(asm, 1, f"mov {R_T1h}, #{context.MEDS_p}")
    add(asm, 1, f"dup {RN_MEDSp}, {R_T1h}")
    # Initialize zero into RN_zero
    add(asm, 1, f"dup {RN_zero}, wzr")
    # Initialize the return value
    if context.swap:
        add(asm, 1, f"dup {RN_ret}, {R_Mr}")
    else:
        add(asm, 1, f"dup {RN_ret}, wzr")
    add_loop(asm, "elimination_loop", R_loop1, "#0", R_max_r, 0, lambda: add_elimination_loop(context, asm))
    if context.backsub:
        add_loop(asm, "backsub_loop", R_loop1, R_max_r, "#0", -1, lambda: add_backsub_loop(context, asm))
    add(asm, 1, "ret")
    return asm

class Context:
    def __init__(self, use_max_r, swap, backsub, MEDS_p, GFq_bits):
        self.use_max_r = use_max_r
        self.swap = swap
        self.backsub = backsub
        self.MEDS_p = MEDS_p
        self.GFq_bits = GFq_bits

def generate_systemizer_file(fun_id, context):
    asm = generate_systemizer_asm(context, fun_id)
    with open(f"{dir_path}/{fun_id}.s", "w") as f:
        f.write("\n".join(asm) + "\n")

if __name__ == "__main__":
    generate_systemizer_file("pmod_mat_syst_test", Context(0, 0, 0, 4093, 12))
    # generate_systemizer_file("pmod_mat_syst_ct_vec", Context(0, 0, 1, 4093, 12))
    # generate_systemizer_file("pmod_mat_syst_ct_partial_vec", Context(1, 0, 1, 4093, 12))
    # generate_systemizer_file("pmod_mat_rref_vec", Context(0, 1, 1, 4093, 12))
