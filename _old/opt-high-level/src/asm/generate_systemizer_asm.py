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

# Preserve v8-v15
RN_ret = "v0.4h"
RN_MEDSp = "v1.4h"
RN_MEDSpw = "v13.4s"
RN_zero = "v2.4h"
RN_minus_one = "v3.4h"
RN_Mrr = "v4.4h"
RN_Mrr_d = "d4"
RN_Mrr0 = "v5.4h"
RN_Mrc = "v6.4h"
RN_Mrc_d = "d6"
RN_Mr2c = "v7.4h"
RN_Mr2c_d = "d7"
RN_T1 = "v16.4h"
RN_T1w = "v16.4s"
RN_T2_d = "d17"
RN_T2 = "v17.4h"
RN_T2w = "v17.4s"
RN_T3 = "v18.4h"
RN_T4 = "v19.4h"
RN_T4w = "v19.4s"
RN_T5w = "v20.4s"

RN_inv_curr = RN_T1
RN_inv_tmp0 = RN_Mrr
RN_inv_tmp3 = RN_T2
RN_inv_tmp5 = RN_T3
RN_inv_temp = RN_T4
RN_inv_tempw = RN_T4w
RN_inv_temp2w = RN_T5w

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
    # Shrink to 16 bits
    add(asm, 1, f"sqxtn {rn_dst}, {rn_src}")

def add_freeze(asm, rn_src, rn_tmp, rn_dst):
    # Remove one final MEDS_p if the value in the lane is at least MEDS_p
    add(asm, 1, f"cmhs {rn_tmp}, {rn_src}, {RN_MEDSp}")
    add(asm, 1, f"and {rn_tmp[:-3]}.16b, {rn_tmp[:-3]}.16b, {RN_MEDSp[:-3]}.16b")
    add(asm, 1, f"sub {rn_dst}, {rn_src}, {rn_tmp}")

def add_freeze_reduce(asm, rn_src, rn_tmp, rn_tmph, rn_dst, GFq_bits):
    add_reduce(asm, rn_src, rn_tmp, rn_dst, GFq_bits)
    add_freeze(asm, rn_dst, rn_tmph, rn_dst)

def add_swap(context, asm):
    return

def add_elimination_row_zero_fix_loop2(context, asm):
    # Load Mrc = M[r][c] and Mr2c = M[r2][c]
    add(asm, 1, f"ldr {RN_Mrc_d}, [{R_Mrc_i}]") # + #8 is done at the store in the end of this loop
    add(asm, 1, f"ldr {RN_Mr2c_d}, [{R_Mr2c_i}], #8")
    # Compute RN_T1 = Mr2c if Mrr is zero, else zero
    add(asm, 1, f"and {RN_T1[:-3]}.16b, {RN_Mrr0[:-3]}.16b, {RN_Mr2c[:-3]}.16b")
    # Compute RN_T2w = RN_T1 + Mrc
    add(asm, 1, f"uaddl {RN_T2w}, {RN_T1}, {RN_Mrc}")
    # Reduce RN_T2w
    add_freeze_reduce(asm, RN_T2w, RN_T1w, RN_T1, RN_T2, context.GFq_bits)
    # Store the result into M[r][c]
    add(asm, 1, f"str {RN_T2_d}, [{R_Mrc_i}], #8")
    return

def add_elimination_row_zero_fix_loop1(context, asm):
    # Load R_Mcr2_i = M[r2][r] = M + 8 * M_c * r2 + 8 * r
    add(asm, 1, f"madd {R_Mr2c_i}, {R_Mc}, {R_loop2}, {R_loop1}")
    add(asm, 1, f"add {R_Mr2c_i}, {R_M}, {R_Mr2c_i}, lsl #3")
    # Loop
    add_loop(asm, "elimination_row_zero_fix_inner_loop", R_loop3, R_loop1, R_Mc, 0, lambda: add_elimination_row_zero_fix_loop2(context, asm))

def add_return_if_zero(context, asm):
    # If the current value of lane i of RN_Mrr is zero, set lane i of RN_ret to -1. Otherwise, leave lane i of RN_ret as it is.
    add(asm, 1, f"cmeq {RN_T1}, {RN_Mrr}, #0")
    add(asm, 1, f"and {RN_T2[:-3]}.16b, {RN_T1[:-3]}.16b, {RN_minus_one[:-3]}.16b")
    add(asm, 1, f"mvn {RN_T1[:-3]}.16b, {RN_T1[:-3]}.16b")
    add(asm, 1, f"and {RN_T1[:-3]}.16b, {RN_T1[:-3]}.16b, {RN_ret[:-3]}.16b")
    add(asm, 1, f"orr {RN_ret[:-3]}.16b, {RN_T1[:-3]}.16b, {RN_T2[:-3]}.16b")

def add_compute_inverse_line(context, asm, rn_dst, rn_src1, rn_src2):
    # Compute RN_dst = RN_src1 * RN_src2
    add(asm, 1, f"umull {RN_inv_tempw}, {rn_src1}, {rn_src2}")
    # Reduce RN_dst
    add_reduce(asm, RN_inv_tempw, RN_inv_temp2w, rn_dst, context.GFq_bits)

def add_compute_inverse(context, asm):
    # Compute the multiplicative inverse of RN_Mrr and store it in RN_Mrr
    add_compute_inverse_line(context, asm, RN_inv_curr, RN_inv_tmp0, RN_inv_tmp0) # 2
    add_compute_inverse_line(context, asm, RN_inv_curr, RN_inv_curr, RN_inv_curr) # 4
    add_compute_inverse_line(context, asm, RN_inv_tmp3, RN_inv_curr, RN_inv_tmp0) # 5
    add_compute_inverse_line(context, asm, RN_inv_curr, RN_inv_tmp3, RN_inv_tmp3) # 10
    add_compute_inverse_line(context, asm, RN_inv_tmp5, RN_inv_curr, RN_inv_tmp3) # 15
    add_compute_inverse_line(context, asm, RN_inv_curr, RN_inv_tmp5, RN_inv_tmp5) # 30
    add_compute_inverse_line(context, asm, RN_inv_curr, RN_inv_curr, RN_inv_curr) # 60
    add_compute_inverse_line(context, asm, RN_inv_curr, RN_inv_curr, RN_inv_curr) # 120
    add_compute_inverse_line(context, asm, RN_inv_curr, RN_inv_curr, RN_inv_curr) # 240
    add_compute_inverse_line(context, asm, RN_inv_curr, RN_inv_curr, RN_inv_tmp5) # 255
    add_compute_inverse_line(context, asm, RN_inv_curr, RN_inv_curr, RN_inv_curr) # 510
    add_compute_inverse_line(context, asm, RN_inv_curr, RN_inv_curr, RN_inv_curr) # 1020
    add_compute_inverse_line(context, asm, RN_inv_curr, RN_inv_curr, RN_inv_curr) # 2040
    add_compute_inverse_line(context, asm, RN_inv_curr, RN_inv_curr, RN_inv_tmp3) # 2045
    add_compute_inverse_line(context, asm, RN_inv_curr, RN_inv_curr, RN_inv_curr) # 4090
    add_compute_inverse_line(context, asm, RN_inv_curr, RN_inv_curr, RN_inv_tmp0) # 4091
    add_freeze(asm, RN_inv_curr, RN_inv_temp, RN_Mrr)

def add_normalize_row(context, asm):
    # Load M[r][c] into RN_Mrc
    add(asm, 1, f"ldr {RN_Mrc_d}, [{R_Mrc_i}]") # + #8 is done at the store in the end of this loop
    # Compute RN_T1 = Mrc * Mrr
    add(asm, 1, f"umull {RN_T1w}, {RN_Mrc}, {RN_Mrr}")
    # Reduce RN_T1
    add_freeze_reduce(asm, RN_T1w, RN_T2w, RN_T2, RN_Mrc, context.GFq_bits)
    # Store the result into M[r][c]
    add(asm, 1, f"str {RN_Mrc_d}, [{R_Mrc_i}], #8")

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
    # Next, load R_Mcr_i = M[r][r] = M + 8 * M_c * r + 8 * r
    add(asm, 1, f"madd {R_Mrc_i}, {R_Mc}, {R_loop1}, {R_loop1}")
    add(asm, 1, f"add {R_Mrc_i}, {R_M}, {R_Mrc_i}, lsl #3")
    # Next, check if M[r][r] is zero
    add(asm, 1, f"cmeq {RN_Mrr0}, {RN_Mrr}, {RN_zero}")
    # Add the loop
    add_loop(asm, "elimination_row_zero_fix_outer_loop", R_loop2, R_loop1, R_Mr, 1, lambda: add_elimination_row_zero_fix_loop1(context, asm))

    # If, after attempting to make M[r][r] non-zero, it is still zero, move -1 into the return value
    add_return_if_zero(context, asm)

    # Compute the multiplicative inverse of M[r][r]
    add_compute_inverse(context, asm)

    # Normalize the current row of M by multiplying it by the inverse of M[r][r] (causing M[r][r] to become 1)
    # First, load R_Mcr_i = M[r][r] = M + 8 * M_c * r + 8 * r
    add(asm, 1, f"madd {R_Mrc_i}, {R_Mc}, {R_loop1}, {R_loop1}")
    add(asm, 1, f"add {R_Mrc_i}, {R_M}, {R_Mrc_i}, lsl #3")
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
    # Initialize MEDS_p into RN_MEDSpw
    add(asm, 1, f"dup {RN_MEDSpw}, {R_T1h}")
    # Initialize zero into RN_zero
    add(asm, 1, f"dup {RN_zero}, wzr")
    # Initialize minus one into RN_minus_one
    add(asm, 1, f"mov {R_T1h}, #-1")
    add(asm, 1, f"dup {RN_minus_one}, {R_T1h}")
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
