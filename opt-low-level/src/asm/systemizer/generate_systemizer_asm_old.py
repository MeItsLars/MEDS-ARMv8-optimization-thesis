import os
import sys

dir_path = os.path.dirname(os.path.realpath(__file__))

# POSSIBLE CALLS:
# pmod_mat_syst_ct_partial_swap_backsub(M, M_r, M_c, M_r, 0, 1);
# > PMOD_MAT_INV - pmod_mat_syst_ct(M, A_r, A_c*2);
# > n n n, m m m, k k k

# pmod_mat_syst_ct_partial_swap_backsub(M, M_r, M_c, max_r, 0, 1);
# > SOLVE_OPT 
# > n 2m n-1

# pmod_mat_syst_ct_partial_swap_backsub(M, M_r, M_c, M_r, 1, 1);
# > SOLVE_OPT
# > m-1 m m-1
# > 2mn mm+nn 2mn

# > n n n 0 1
# > m m m 0 1
# > k k k 0 1
# > n 2m n-1 0 1
# > m-1 m m-1 1 1
# > 2mn mm+nn 2mn 1 1

# Preserve from x15 
R_M = "x0"
R_Mc = "x1"
R_ret = "x2"
R_MEDSp = "x3"
R_MEDSp_h = "w3"
R_loop1 = "x4"
R_loop2 = "x5"
R_loop3 = "x6"
R_Mrr_i = "x7"
R_Mrc_l = "x8"
R_Mr2c_l = "x9"
R_Mrr = "x10"
R_Mrr_h = "w10"
R_Mrc = "x11"
R_Mrc_h = "w11"
R_Mr2c = "x12"
R_Mr2c_h = "w12"
R_Mr2r = "x13"
R_Mr2r_h = "w13"
R_T1 = "x14"
R_T1_h = "w14"
R_T2 = "x15"
R_T2_h = "w15"

R_Mir = R_Mrc
R_Mir_h = R_Mrc_h
R_MiMc = R_Mr2c
R_MiMc_h = R_Mr2c_h
R_Mir_l = R_Mrc_l
R_MiMc_l = R_Mr2c_l
R_2Mc = R_Mrr

R_inv_curr = R_T1
R_inv_tmp0 = R_Mrr
R_inv_tmp3 = R_T2
R_inv_tmp5 = R_loop2
R_inv_temp = R_loop3

# Preserve v8-v15
RN_Mrr = "v1.4h"
RN_Mrr_8h = "v2.8h"
RN_Mrc = "v3.4h"
RN_Mrc_8h = "v3.8h"
RN_Mr2c = "v4.4h"
RN_Mr2c_8h = "v4.8h"
RN_Mrr0 = "v5.4h"
RN_Mrr0_8h = "v6.8h"
RN_Mr2r = "v7.4h"

RN_MEDSp = "v16.4h"
RN_MEDSp_8h = "v17.8h"
RN_MEDSp_w = "v18.4s"
RN_T1 = "v19.4h"
RN_T1_8h = "v19.8h"
RN_T1_w = "v19.4s"
RN_T2 = "v20.4h"
RN_T2_8h = "v20.8h"
RN_T2_w = "v20.4s"

def add(asm, indentation, s):
    asm.append("    " * indentation + s)

def add_loop(asm, loop_name, loop_var, loop_from, loop_to, direction, loop_code_fun):
    # Initialize loop
    if direction == 0 or direction == -2:
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

#############################################################################
# MODULAR REDUCTION
#############################################################################
########## NORMAL ##########
def add_reduce(asm, rn_src, rn_tmp, rn_dst, GFq_bits):
    # Apply two reductions
    add(asm, 1, f"lsr {rn_tmp}, {rn_src}, #{GFq_bits}")
    add(asm, 1, f"mul {rn_tmp}, {rn_tmp}, {R_MEDSp}")
    add(asm, 1, f"sub {rn_src}, {rn_src}, {rn_tmp}")
    add(asm, 1, f"lsr {rn_tmp}, {rn_src}, #{GFq_bits}")
    add(asm, 1, f"mul {rn_tmp}, {rn_tmp}, {R_MEDSp}")
    add(asm, 1, f"sub {rn_dst}, {rn_src}, {rn_tmp}")

def add_freeze(asm, r_src, r_tmp, r_dst, MEDS_p):
    # Remove one final MEDS_p if the value in the lane is at least MEDS_p'
    add(asm, 1, f"cmp {r_src}, #{MEDS_p}")
    add(asm, 1, f"csel {r_tmp}, {R_MEDSp}, xzr, ge")
    add(asm, 1, f"sub {r_dst}, {r_src}, {r_tmp}")

def add_freeze_reduce(asm, rn_src, rn_tmp, rn_dst, MEDS_p, GFq_bits):
    add_reduce(asm, rn_src, rn_tmp, rn_dst, GFq_bits)
    add_freeze(asm, rn_dst, rn_tmp, rn_dst, MEDS_p)

########## NEON 16x4 ##########
def add_reduce_neon_16x4(asm, rn_src, rn_tmp, rn_dst, GFq_bits):
    # Apply two reductions
    add(asm, 1, f"ushr {rn_tmp}, {rn_src}, #{GFq_bits}")
    add(asm, 1, f"mul {rn_tmp}, {rn_tmp}, {RN_MEDSp_w}")
    add(asm, 1, f"sub {rn_src}, {rn_src}, {rn_tmp}")
    add(asm, 1, f"ushr {rn_tmp}, {rn_src}, #{GFq_bits}")
    add(asm, 1, f"mul {rn_tmp}, {rn_tmp}, {RN_MEDSp_w}")
    add(asm, 1, f"sub {rn_src}, {rn_src}, {rn_tmp}")
    # Shrink to 16 bits
    add(asm, 1, f"sqxtn {rn_dst}, {rn_src}")

def add_freeze_neon_16x4(asm, rn_src, rn_tmp, rn_dst):
    # Remove one final MEDS_p if the value in the lane is at least MEDS_p
    add(asm, 1, f"cmhs {rn_tmp}, {rn_src}, {RN_MEDSp}")
    add(asm, 1, f"and {rn_tmp[:-3]}.16b, {rn_tmp[:-3]}.16b, {RN_MEDSp[:-3]}.16b")
    add(asm, 1, f"sub {rn_dst}, {rn_src}, {rn_tmp}")

def add_freeze_reduce_neon_16x4(asm, rn_src, rn_tmp, rn_tmph, rn_dst, GFq_bits):
    add_reduce_neon_16x4(asm, rn_src, rn_tmp, rn_dst, GFq_bits)
    add_freeze_neon_16x4(asm, rn_dst, rn_tmph, rn_dst)

########## NEON 16x8 ##########
def add_freeze_neon_16x8(asm, rn_src, rn_tmp, rn_dst):
    # Remove one final MEDS_p if the value in the lane is at least MEDS_p
    add(asm, 1, f"cmhs {rn_tmp}, {rn_src}, {RN_MEDSp_8h}")
    add(asm, 1, f"and {rn_tmp[:-3]}.16b, {rn_tmp[:-3]}.16b, {RN_MEDSp_8h[:-3]}.16b")
    add(asm, 1, f"sub {rn_dst}, {rn_src}, {rn_tmp}")

#############################################################################
# ELIMINATION LOOP
#############################################################################
def add_swap(context, asm):
    # Store context.Mc * 2 into Mrr (we're not using Mrr anyway right now)
    add(asm, 1, f"mov {R_2Mc}, #{context.Mc * 2}")
    # If Mrr == 0 and all rows below have Mr2r == 0, swap the current column with the last one
    add(asm, 1, f"add {R_loop2}, {R_M}, {R_Mrr_i}, lsl #1")
    # Initialize z = 0
    add(asm, 1, f"mov {R_T1}, #0")
    # Loop over all rows below the current one and or the values of Mrc with z
    add(asm, 1, f"mov {R_loop3}, {R_loop1}")
    add(asm, 0, "elimination_swap_or_loop:")
    add(asm, 1, f"cmp {R_loop3}, #{context.Mr}")
    add(asm, 1, f"b.ge elimination_swap_or_loop_end")
    if context.Mc * 2 < 256:
        add(asm, 1, f"ldrh {R_Mr2r_h}, [{R_loop2}], #{context.Mc * 2}")
    else:
        add(asm, 1, f"ldrh {R_Mr2r_h}, [{R_loop2}]")
        add(asm, 1, f"add {R_loop2}, {R_loop2}, {R_2Mc}")
    add(asm, 1, f"orr {R_T1}, {R_T1}, {R_Mr2r}")
    add(asm, 1, f"add {R_loop3}, {R_loop3}, #1")
    add(asm, 1, "b elimination_swap_or_loop")
    add(asm, 0, "elimination_swap_or_loop_end:")
    # If z == 0, ret = r. If z != 0, ret = ret
    add(asm, 1, f"cmp {R_T1}, #0")
    add(asm, 1, f"csel {R_ret}, {R_ret}, {R_loop1}, ne")
    # If T1 == TRUE, swap the current ('r') and last columns
    add(asm, 1, f"add {R_Mir_l}, {R_M}, {R_loop1}, lsl #1")
    if context.Mc * 2 < 256:
        add(asm, 1, f"add {R_MiMc_l}, {R_M}, {(context.Mc - 1) * 2}")
    else:
        add(asm, 1, f"add {R_MiMc_l}, {R_M}, {R_2Mc}")
        add(asm, 1, f"sub {R_MiMc_l}, {R_MiMc_l}, #2")
    add(asm, 1, f"mov {R_loop2}, #0")
    add(asm, 0, "elimination_swap_loop:")
    add(asm, 1, f"cmp {R_loop2}, #{context.Mr}")
    add(asm, 1, f"b.ge elimination_swap_loop_end")
    # Load M[i][r] and M[i][Mc - 1] into R_Mir_h and R_MiMc_h
    add(asm, 1, f"ldrh {R_Mir_h}, [{R_Mir_l}]") # Increment at store
    add(asm, 1, f"ldrh {R_MiMc_h}, [{R_MiMc_l}]") # Increment at store
    # If T1 == TRUE, swap R_Mir and R_MiMc. Otherwise, do nothing
    add(asm, 1, f"cmp {R_T1}, #0")
    add(asm, 1, f"csel {R_T2}, {R_Mir}, {R_MiMc}, ne")
    add(asm, 1, f"csel {R_MiMc_h}, {R_MiMc_h}, {R_Mir_h}, ne")
    add(asm, 1, f"mov {R_Mir}, {R_T2}")
    # Store the results back
    if context.Mc * 2 < 256:
        add(asm, 1, f"strh {R_Mir_h}, [{R_Mir_l}], #{context.Mc * 2}")
        add(asm, 1, f"strh {R_MiMc_h}, [{R_MiMc_l}], #{context.Mc * 2}")
    else:
        add(asm, 1, f"strh {R_Mir_h}, [{R_Mir_l}]")
        add(asm, 1, f"strh {R_MiMc_h}, [{R_MiMc_l}]")
        add(asm, 1, f"add {R_Mir_l}, {R_Mir_l}, {R_2Mc}")
        add(asm, 1, f"add {R_MiMc_l}, {R_MiMc_l}, {R_2Mc}")
    add(asm, 1, f"add {R_loop2}, {R_loop2}, #1")
    add(asm, 1, f"b elimination_swap_loop")
    add(asm, 0, "elimination_swap_loop_end:")

def add_elimination_row_zero_fix_loop2_neon_16x8(context, asm):
    # Load Mrc = M[r][c] and Mr2c = M[r2][c]
    add(asm, 1, f"ld1 {{{RN_Mrc_8h}}}, [{R_Mrc_l}]") # + #16 is done at the store in the end of this loop
    add(asm, 1, f"ld1 {{{RN_Mr2c_8h}}}, [{R_Mr2c_l}], #16")
    # Compute RN_T1 = Mr2c if Mrr is zero, else zero
    add(asm, 1, f"and {RN_T1_8h[:-3]}.16b, {RN_Mrr0_8h[:-3]}.16b, {RN_Mr2c_8h[:-3]}.16b")
    # Compute RN_T2w = RN_T1 + Mrc
    add(asm, 1, f"add {RN_T2_8h}, {RN_T1_8h}, {RN_Mrc_8h}")
    # Reduce RN_T2w
    add_freeze_neon_16x8(asm, RN_T2_8h, RN_T1_8h, RN_T2_8h)
    # Store the result into M[r][c]
    add(asm, 1, f"st1 {{{RN_T2_8h}}}, [{R_Mrc_l}], #16")

def add_elimination_row_zero_fix_loop2_neon_16x4(context, asm):
    # Load Mrc = M[r][c] and Mr2c = M[r2][c]
    add(asm, 1, f"ld1 {{{RN_Mrc}}}, [{R_Mrc_l}]") # + #8 is done at the store in the end of this loop
    add(asm, 1, f"ld1 {{{RN_Mr2c}}}, [{R_Mr2c_l}], #8")
    # Compute RN_T1 = Mr2c if Mrr is zero, else zero
    add(asm, 1, f"and {RN_T1[:-3]}.16b, {RN_Mrr0[:-3]}.16b, {RN_Mr2c[:-3]}.16b")
    # Compute RN_T2w = RN_T1 + Mrc
    add(asm, 1, f"add {RN_T2}, {RN_T1}, {RN_Mrc}")
    # Reduce RN_T2w
    add_freeze_neon_16x4(asm, RN_T2, RN_T1, RN_T2)
    # Store the result into M[r][c]
    add(asm, 1, f"st1 {{{RN_T2}}}, [{R_Mrc_l}], #8")

def add_elimination_row_zero_fix_loop2_scalar(context, asm):
    # Load Mrc = M[r][c] and Mr2c = M[r2][c]
    add(asm, 1, f"ldrh {R_Mrc_h}, [{R_Mrc_l}]") # + #2 is done at the store in the end of this loop
    add(asm, 1, f"ldrh {R_Mr2c_h}, [{R_Mr2c_l}], #2")
    # Compute RN_T1 = Mr2c if Mrr is zero, else zero
    add(asm, 1, f"cmp {R_Mrr}, #0")
    add(asm, 1, f"csel {R_T1}, {R_Mr2c}, xzr, eq")
    # Compute RN_T2w = RN_T1 + Mrc
    add(asm, 1, f"add {R_T1}, {R_T1}, {R_Mrc}")
    # Reduce RN_T2w
    add_freeze(asm, R_T1, R_Mrc, R_T1, context.MEDS_p)
    # Store the result into M[r][c]
    add(asm, 1, f"strh {R_T1_h}, [{R_Mrc_l}], #2")

def add_elimination_row_zero_fix_loop1(context, asm):
    ## INITIALIZE VARIABLES
    # Load R_Mcr2_i = M[r2][r] = M + 2 * M_c * r2 + 2 * r
    add(asm, 1, f"madd {R_Mr2c_l}, {R_Mc}, {R_loop2}, {R_loop1}")
    add(asm, 1, f"add {R_Mr2c_l}, {R_M}, {R_Mr2c_l}, lsl #1")
    # Load R_Mrr = M[r][r] = M + 8 * M_c * r + 8 * r
    add(asm, 1, f"ldrh {R_Mrr_h}, [{R_M}, {R_Mrr_i}, lsl #1]")
    # Next, load R_Mcr_i = M[r][r] = M + 8 * M_c * r + 8 * r
    add(asm, 1, f"add {R_Mrc_l}, {R_M}, {R_Mrr_i}, lsl #1")
    # Load Mrr into all lanes of RN_Mrr
    add(asm, 1, f"dup {RN_Mrr}, {R_Mrr_h}")
    add(asm, 1, f"dup {RN_Mrr_8h}, {R_Mrr_h}")
    add(asm, 1, f"cmeq {RN_Mrr0}, {RN_Mrr}, #0")
    add(asm, 1, f"cmeq {RN_Mrr0_8h}, {RN_Mrr_8h}, #0")

    ## PERFORM LOOP
    add(asm, 1, f"mov {R_loop3}, {R_loop1}")
    # Step 1: Do 16x8 NEON loops while there are at least 8 lanes left
    add(asm, 0, f"elimination_row_zero_fix_inner_loop_neon_16x8:")
    add(asm, 1, f"sub {R_T1}, {R_Mc}, {R_loop3}")
    add(asm, 1, f"cmp {R_T1}, #8")
    add(asm, 1, f"b.lt elimination_row_zero_fix_inner_loop_neon_16x4")
    add_elimination_row_zero_fix_loop2_neon_16x8(context, asm)
    add(asm, 1, f"add {R_loop3}, {R_loop3}, #8")
    add(asm, 1, f"b elimination_row_zero_fix_inner_loop_neon_16x8")
    # Step 2: Do 16x4 NEON if there are at least 4 lanes left
    add(asm, 0, f"elimination_row_zero_fix_inner_loop_neon_16x4:")
    add(asm, 1, f"sub {R_T1}, {R_Mc}, {R_loop3}")
    add(asm, 1, f"cmp {R_T1}, #4")
    add(asm, 1, f"b.lt elimination_row_zero_fix_inner_loop_scalar")
    add_elimination_row_zero_fix_loop2_neon_16x4(context, asm)
    add(asm, 1, f"add {R_loop3}, {R_loop3}, #4")
    # Step 3: Do scalar loops while there are less than 4 lanes left
    add(asm, 0, f"elimination_row_zero_fix_inner_loop_scalar:")
    add(asm, 1, f"cmp {R_loop3}, {R_Mc}")
    add(asm, 1, f"b.ge elimination_row_zero_fix_inner_loop_end")
    add_elimination_row_zero_fix_loop2_scalar(context, asm)
    add(asm, 1, f"add {R_loop3}, {R_loop3}, #1")
    add(asm, 1, f"b elimination_row_zero_fix_inner_loop_scalar")
    # Step 4: End
    add(asm, 0, f"elimination_row_zero_fix_inner_loop_end:")

def add_compute_inverse_line(context, asm, rn_dst, rn_src1, rn_src2):
    # Compute RN_dst = RN_src1 * RN_src2
    add(asm, 1, f"mul {rn_dst}, {rn_src1}, {rn_src2}")
    # Reduce RN_dst
    add_reduce(asm, rn_dst, R_inv_temp, rn_dst, context.GFq_bits)

def add_compute_inverse(context, asm):
    # Compute the multiplicative inverse of RN_Mrr and store it in RN_Mrr
    add_compute_inverse_line(context, asm, R_inv_curr, R_inv_tmp0, R_inv_tmp0) # 2
    add_compute_inverse_line(context, asm, R_inv_curr, R_inv_curr, R_inv_curr) # 4
    add_compute_inverse_line(context, asm, R_inv_tmp3, R_inv_curr, R_inv_tmp0) # 5
    add_compute_inverse_line(context, asm, R_inv_curr, R_inv_tmp3, R_inv_tmp3) # 10
    add_compute_inverse_line(context, asm, R_inv_tmp5, R_inv_curr, R_inv_tmp3) # 15
    add_compute_inverse_line(context, asm, R_inv_curr, R_inv_tmp5, R_inv_tmp5) # 30
    add_compute_inverse_line(context, asm, R_inv_curr, R_inv_curr, R_inv_curr) # 60
    add_compute_inverse_line(context, asm, R_inv_curr, R_inv_curr, R_inv_curr) # 120
    add_compute_inverse_line(context, asm, R_inv_curr, R_inv_curr, R_inv_curr) # 240
    add_compute_inverse_line(context, asm, R_inv_curr, R_inv_curr, R_inv_tmp5) # 255
    add_compute_inverse_line(context, asm, R_inv_curr, R_inv_curr, R_inv_curr) # 510
    add_compute_inverse_line(context, asm, R_inv_curr, R_inv_curr, R_inv_curr) # 1020
    add_compute_inverse_line(context, asm, R_inv_curr, R_inv_curr, R_inv_curr) # 2040
    add_compute_inverse_line(context, asm, R_inv_curr, R_inv_curr, R_inv_tmp3) # 2045
    add_compute_inverse_line(context, asm, R_inv_curr, R_inv_curr, R_inv_curr) # 4090
    add_compute_inverse_line(context, asm, R_inv_curr, R_inv_curr, R_inv_tmp0) # 4091
    add_freeze(asm, R_inv_curr, R_inv_temp, R_Mrr, context.MEDS_p)

def add_normalize_row_neon_16x4(context, asm):
    # Load M[r][c] into RN_Mrc
    add(asm, 1, f"ld1 {{{RN_Mrc}}}, [{R_Mrc_l}]") # + #8 is done at the store in the end of this loop
    # Compute RN_T1 = Mrc * Mrr
    add(asm, 1, f"umull {RN_T1_w}, {RN_Mrc}, {RN_Mrr}")
    # Reduce RN_T1
    add_freeze_reduce_neon_16x4(asm, RN_T1_w, RN_T2_w, RN_T2, RN_Mrc, context.GFq_bits)
    # Store the result into M[r][c]
    add(asm, 1, f"st1 {{{RN_Mrc}}}, [{R_Mrc_l}], #8")

def add_normalize_row(context, asm):
    # Load M[r][c] into RN_Mrc
    add(asm, 1, f"ldrh {R_Mrc_h}, [{R_Mrc_l}]") # + #2 is done at the store in the end of this loop
    # Compute RN_T1 = Mrc * Mrr
    add(asm, 1, f"mul {R_T1}, {R_Mrc}, {R_Mrr}")
    # Reduce RN_T1
    add_freeze_reduce(asm, R_T1, R_T2, R_Mrc, context.MEDS_p, context.GFq_bits)
    # Store the result into M[r][c]
    add(asm, 1, f"strh {R_Mrc_h}, [{R_Mrc_l}], #2")

def add_normalize_loop(context, asm):
    # First, load R_Mcr_i = M[r][r] = M + 8 * M_c * r + 8 * r
    add(asm, 1, f"add {R_Mrc_l}, {R_M}, {R_Mrr_i}, lsl #1")

    add(asm, 1, f"mov {R_loop3}, {R_loop1}")
    add(asm, 0, "elimination_normalize_row_loop_neon_16x4:")
    add(asm, 1, f"sub {R_T1}, {R_Mc}, {R_loop3}")
    add(asm, 1, f"cmp {R_T1}, #4")
    add(asm, 1, f"b.lt elimination_normalize_row_loop_scalar")
    add_normalize_row_neon_16x4(context, asm)
    add(asm, 1, f"add {R_loop3}, {R_loop3}, #4")
    add(asm, 1, f"b elimination_normalize_row_loop_neon_16x4")
    add(asm, 0, "elimination_normalize_row_loop_scalar:")
    add(asm, 1, f"cmp {R_loop3}, {R_Mc}")
    add(asm, 1, f"b.ge elimination_normalize_row_loop_end")
    add_normalize_row(context, asm)
    add(asm, 1, f"add {R_loop3}, {R_loop3}, #1")
    add(asm, 1, f"b elimination_normalize_row_loop_scalar")
    add(asm, 0, "elimination_normalize_row_loop_end:")

def add_elimination_row_eliminate_inner_loop_neon_16x4(context, asm):
    # Load Mrc = M[r][c] and Mr2c = M[r2][c]
    add(asm, 1, f"ld1 {{{RN_Mrc}}}, [{R_Mrc_l}], #8")
    add(asm, 1, f"ld1 {{{RN_Mr2c}}}, [{R_Mr2c_l}]") # + #8 is done at the store in the end of this loop
    # Compute T1 = Mrc * Mr2r
    add(asm, 1, f"umull {RN_T1_w}, {RN_Mrc}, {RN_Mr2r}")
    # Reduce T1
    add_freeze_reduce_neon_16x4(asm, RN_T1_w, RN_T2_w, RN_T2, RN_T1, context.GFq_bits)
    # Compute val=(MEDS_p + tmp1 - val) % MEDS_p;
    add(asm, 1, f"add {RN_T2}, {RN_Mr2c}, {RN_MEDSp}")
    add(asm, 1, f"sub {RN_T2}, {RN_T2}, {RN_T1}")
    add_freeze_neon_16x4(asm, RN_T2, RN_T1, RN_Mr2c)
    # Store the result into M[r][c]
    add(asm, 1, f"st1 {{{RN_Mr2c}}}, [{R_Mr2c_l}], #8")

def add_elimination_row_eliminate_inner_loop_scalar(context, asm):
    # Load Mrc = M[r][c] and Mr2c = M[r2][c]
    add(asm, 1, f"ldrh {R_Mrc_h}, [{R_Mrc_l}], #2")
    add(asm, 1, f"ldrh {R_Mr2c_h}, [{R_Mr2c_l}]") # + #2 is done at the store in the end of this loop
    # Compute T1 = Mrc * Mr2r
    add(asm, 1, f"mul {R_T1}, {R_Mrc}, {R_Mr2r}")
    # Reduce T1
    add_freeze_reduce(asm, R_T1, R_T2, R_T1, context.MEDS_p, context.GFq_bits)
    # Compute val=(MEDS_p + tmp1 - val) % MEDS_p;
    add(asm, 1, f"add {R_T2}, {R_Mr2c}, #{context.MEDS_p}")
    add(asm, 1, f"sub {R_T2}, {R_T2}, {R_T1}")
    add_freeze(asm, R_T2, R_T1, R_Mr2c, context.MEDS_p)
    # Store the result into M[r][c]
    add(asm, 1, f"strh {R_Mr2c_h}, [{R_Mr2c_l}], #2")

def add_elimination_row_eliminate_outer_loop(context, asm):
    ## INITIALIZE VARIABLES
    # Load R_Mcr2_i = M[r2][r] = M + 2 * M_c * r2 + 2 * r
    add(asm, 1, f"madd {R_Mr2c_l}, {R_Mc}, {R_loop2}, {R_loop1}")
    add(asm, 1, f"add {R_Mr2c_l}, {R_M}, {R_Mr2c_l}, lsl #1")
    # Load R_Mr2r = M[r2][r] = M + 2 * M_c * r2 + 2 * r
    add(asm, 1, f"ldrh {R_Mr2r_h}, [{R_Mr2c_l}]")
    # Load R_Mcr_i = M[r][r] = M + 8 * M_c * r + 8 * r
    add(asm, 1, f"add {R_Mrc_l}, {R_M}, {R_Mrr_i}, lsl #1")
    # Load Mrr and Mr2r into NEON lanes
    add(asm, 1, f"dup {RN_Mrr}, {R_Mrr_h}")
    add(asm, 1, f"dup {RN_Mr2r}, {R_Mr2r_h}")

    ## PERFORM LOOP
    add(asm, 1, f"mov {R_loop3}, {R_loop1}")
    # Step 1: Do 16x4 NEON if there are at least 4 lanes left
    add(asm, 0, f"elimination_row_eliminate_inner_loop_neon_16x4:")
    add(asm, 1, f"sub {R_T1}, {R_Mc}, {R_loop3}")
    add(asm, 1, f"cmp {R_T1}, #4")
    add(asm, 1, f"b.lt elimination_row_eliminate_inner_loop_scalar")
    add_elimination_row_eliminate_inner_loop_neon_16x4(context, asm)
    add(asm, 1, f"add {R_loop3}, {R_loop3}, #4")
    add(asm, 1, f"b elimination_row_eliminate_inner_loop_neon_16x4")
    # Step 2: Do scalar loops while there are less than 4 lanes left
    add(asm, 0, f"elimination_row_eliminate_inner_loop_scalar:")
    add(asm, 1, f"cmp {R_loop3}, {R_Mc}")
    add(asm, 1, f"b.ge elimination_row_eliminate_inner_loop_end")
    add_elimination_row_eliminate_inner_loop_scalar(context, asm)
    add(asm, 1, f"add {R_loop3}, {R_loop3}, #1")
    add(asm, 1, f"b elimination_row_eliminate_inner_loop_scalar")
    # Step 3: End
    add(asm, 0, f"elimination_row_eliminate_inner_loop_end:")

def add_elimination_loop(context, asm):
    # First, load the index of M[r][r]: M_c * r + r
    add(asm, 1, f"madd {R_Mrr_i}, {R_Mc}, {R_loop1}, {R_loop1}")

    # Add swap code if it is needed
    if context.do_swap:
        add_swap(context, asm)
    
    # Add the loop that makes sure the current M[r][r] is not zero
    add_loop(asm, "elimination_row_zero_fix_outer_loop", R_loop2, R_loop1, context.Mr, 1, lambda: add_elimination_row_zero_fix_loop1(context, asm))

    # If, after attempting to make M[r][r] non-zero, it is still zero, return with -1 in the return value
    add(asm, 1, f"ldrh {R_Mrr_h}, [{R_M}, {R_Mrr_i}, lsl #1]")
    add(asm, 1, f"cmp {R_Mrr}, #0")
    add(asm, 1, f"b.eq ret_fail")

    # Compute the multiplicative inverse of M[r][r]
    add_compute_inverse(context, asm)

    # Normalize the current row of M by multiplying it by the inverse of M[r][r] (causing M[r][r] to become 1)
    # Load R_Mrr into RN_Mrr
    add(asm, 1, f"dup {RN_Mrr}, {R_Mrr_h}")
    add_normalize_loop(context, asm)

    # Eliminate the rows below the current one (so that M[r + x][c] becomes 0)
    add_loop(asm, "elimination_eliminate_rows_loop", R_loop2, R_loop1, context.Mr, 1, lambda: add_elimination_row_eliminate_outer_loop(context, asm))

def add_backsub_column_loop_neon_16x4(context, asm):
    # Load Mrc and Mr2c
    add(asm, 1, f"ld1 {{{RN_Mrc}}}, [{R_Mrc_l}], #8")
    add(asm, 1, f"ld1 {{{RN_Mr2c}}}, [{R_Mr2c_l}]") # + #8 is done at the store in the end of this loop
    # Compute T1 = Mrc * Mr2r
    add(asm, 1, f"umull {RN_T1_w}, {RN_Mrc}, {RN_Mr2r}")
    # Reduce T1
    add_freeze_reduce_neon_16x4(asm, RN_T1_w, RN_T2_w, RN_T2, RN_T1, context.GFq_bits)
    # Compute val=(MEDS_p + tmp1 - val) % MEDS_p;
    add(asm, 1, f"add {RN_T2}, {RN_Mr2c}, {RN_MEDSp}")
    add(asm, 1, f"sub {RN_T2}, {RN_T2}, {RN_T1}")
    add_freeze_neon_16x4(asm, RN_T2, RN_T1, RN_Mr2c)
    # Store the result into M[r2][c]
    add(asm, 1, f"st1 {{{RN_Mr2c}}}, [{R_Mr2c_l}], #8")

def add_backsub_column_loop_scalar(context, asm):
    # Load Mrc and Mr2c
    add(asm, 1, f"ldrh {R_Mrc_h}, [{R_Mrc_l}], #2")
    add(asm, 1, f"ldrh {R_Mr2c_h}, [{R_Mr2c_l}]") # + #2 is done at the store in the end of this loop
    # Compute T1 = Mrc * Mr2r
    add(asm, 1, f"mul {R_T1}, {R_Mrc}, {R_Mr2r}")
    # Reduce T1
    add_freeze_reduce(asm, R_T1, R_T2, R_T1, context.MEDS_p, context.GFq_bits)
    # Compute val=(MEDS_p + tmp1 - val) % MEDS_p;
    add(asm, 1, f"add {R_T2}, {R_Mr2c}, #{context.MEDS_p}")
    add(asm, 1, f"sub {R_T2}, {R_T2}, {R_T1}")
    add_freeze(asm, R_T2, R_T1, R_Mr2c, context.MEDS_p)
    # Store the result into M[r2][c]
    add(asm, 1, f"strh {R_Mr2c_h}, [{R_Mr2c_l}], #2")

def add_backsub_inner_loop(context, asm):
    ## INITIALIZE VARIABLES
    # Load R_Mr2r = M[r2][r] = M + 2 * M_c * r2 + 2 * r
    add(asm, 1, f"madd {R_T1}, {R_Mc}, {R_loop2}, {R_loop1}")
    add(asm, 1, f"ldrh {R_Mr2r_h}, [{R_M}, {R_T1}, lsl #1]")
    # Store M[r2][r] into RN_Mr2r
    add(asm, 1, f"dup {RN_Mr2r}, {R_Mr2r_h}")

    # Load R_Mrc = M[r][r] = M + 2 * M_c * r + 2 * r
    add(asm, 1, f"madd {R_T1}, {R_Mc}, {R_loop1}, {R_loop1}")
    add(asm, 1, f"ldrh {R_Mrc_h}, [{R_M}, {R_T1}, lsl #1]")
    # Load R_Mr2c = M[r2][r] = M + 2 * M_c * r2 + 2 * r
    add(asm, 1, f"madd {R_Mr2c_l}, {R_Mc}, {R_loop2}, {R_loop1}")
    add(asm, 1, f"ldrh {R_Mr2c_h}, [{R_M}, {R_Mr2c_l}, lsl #1]")
    # Compute T1 = Mrc * Mr2r
    add(asm, 1, f"mul {R_T1}, {R_Mrc}, {R_Mr2r}")
    # Reduce T1
    add_freeze_reduce(asm, R_T1, R_T2, R_T1, context.MEDS_p, context.GFq_bits)
    # Compute val=(MEDS_p + tmp1 - val) % MEDS_p;
    add(asm, 1, f"add {R_T2}, {R_Mr2c}, #{context.MEDS_p}")
    add(asm, 1, f"sub {R_T2}, {R_T2}, {R_T1}")
    add_freeze(asm, R_T2, R_T1, R_Mr2c, context.MEDS_p)
    # Store the result into M[r2][r]
    add(asm, 1, f"strh {R_Mr2c_h}, [{R_M}, {R_Mr2c_l}, lsl #1]")

    add(asm, 1, f"mov {R_loop3}, #{context.max_r}")
    # Load R_Mrc_l = M[r][c] = M + 2 * M_c * r + 2 * c (= max_r)
    add(asm, 1, f"madd {R_Mrc_l}, {R_Mc}, {R_loop1}, {R_loop3}")
    add(asm, 1, f"add {R_Mrc_l}, {R_M}, {R_Mrc_l}, lsl #1")
    # Load R_Mr2c_l = M[r2][c] = M + 2 * M_c * r2 + 2 * c (= max_r)
    add(asm, 1, f"madd {R_Mr2c_l}, {R_Mc}, {R_loop2}, {R_loop3}")
    add(asm, 1, f"add {R_Mr2c_l}, {R_M}, {R_Mr2c_l}, lsl #1")


    ## PERFORM LOOP
    # Step 1: Do 16x4 NEON if there are at least 4 lanes left
    add(asm, 0, f"backsub_column_loop_neon_16x4:")
    add(asm, 1, f"sub {R_T1}, {R_Mc}, {R_loop3}")
    add(asm, 1, f"cmp {R_T1}, #4")
    add(asm, 1, f"b.lt backsub_column_loop_scalar")
    add_backsub_column_loop_neon_16x4(context, asm)
    add(asm, 1, f"add {R_loop3}, {R_loop3}, #4")
    add(asm, 1, f"b backsub_column_loop_neon_16x4")
    # Step 2: Do scalar loops while there are less than 4 lanes left
    add(asm, 0, f"backsub_column_loop_scalar:")
    add(asm, 1, f"cmp {R_loop3}, {R_Mc}")
    add(asm, 1, f"b.ge backsub_column_loop_end")
    add_backsub_column_loop_scalar(context, asm)
    add(asm, 1, f"add {R_loop3}, {R_loop3}, #1")
    add(asm, 1, f"b backsub_column_loop_scalar")
    # Step 3: End
    add(asm, 0, f"backsub_column_loop_end:")

def add_backsub_outer_loop(context, asm):
    add_loop(asm, "backsub_inner_loop", R_loop2, "#0", R_loop1, 0, lambda: add_backsub_inner_loop(context, asm))

def generate_systemizer_asm(context, fun_id):
    asm = []
    # Header and function information
    add(asm, 0, ".cpu cortex-a72")
    add(asm, 0, ".arch armv8-a")
    add(asm, 0, f".global {fun_id}")
    add(asm, 0, f"{fun_id}:")
    # Set R_Mc
    add(asm, 1, f"mov {R_Mc}, #{context.Mc}")
    # Set MEDS_p
    add(asm, 1, f"mov {R_MEDSp}, #{context.MEDS_p}")
    # Initialize MEDS_p into RN_MEDSp registers
    add(asm, 1, f"dup {RN_MEDSp}, {R_MEDSp_h}")
    add(asm, 1, f"dup {RN_MEDSp_8h}, {R_MEDSp_h}")
    add(asm, 1, f"dup {RN_MEDSp_w}, {R_MEDSp_h}")
    # Initialize the return value
    if context.do_swap:
        add(asm, 1, f"mov {R_ret}, #{context.Mr}")
    else:
        add(asm, 1, f"mov {R_ret}, xzr")
    add_loop(asm, "elimination_loop", R_loop1, "#0", f"#{context.max_r}", 0, lambda: add_elimination_loop(context, asm))
    if context.do_backsub:
        add_loop(asm, "backsub_outer_loop", R_loop1, f"#{context.max_r - 1}", "#0", -2, lambda: add_backsub_outer_loop(context, asm))
    add(asm, 0, "ret_success:")
    add(asm, 1, f"mov x0, {R_ret}")
    add(asm, 1, "ret")
    add(asm, 0, "ret_fail:")
    add(asm, 1, "mov x0, #-1")
    add(asm, 1, "ret")
    return asm

class Context:
    def __init__(self, Mr, Mc, max_r, do_swap, do_backsub, MEDS_p, GFq_bits):
        self.Mr = Mr
        self.Mc = Mc
        self.max_r = max_r
        self.do_swap = do_swap
        self.do_backsub = do_backsub
        self.MEDS_p = MEDS_p
        self.GFq_bits = GFq_bits

def generate_systemizer_file(name, context, fun_id):
    asm = generate_systemizer_asm(context, fun_id)
    path = f"{dir_path}/{name}/{fun_id}.s"
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, "w") as f:
        for line in asm:
            f.write(line + "\n")
    print(f'Generated {fun_id}.s ({context.Mr}, {context.Mc}, {context.max_r}, {context.do_swap}, {context.do_backsub}, {MEDS_p}, {GFq_bits})')

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
    
    name, m, n, k, MEDS_p, GFq_bits = parse_params_h(sys.argv[1])
    # Generate the 6 different systemizations required for MEDS
    generate_systemizer_file(name, Context(5, 5, 5, False, False, MEDS_p, GFq_bits), 'pmod_mat_syst_5_5_5_0_0')
    generate_systemizer_file(name, Context(k, k, k, False, False, MEDS_p, GFq_bits), 'pmod_mat_syst_k_k_k_0_0')
    generate_systemizer_file(name, Context(n, 2*n, n, False, True, MEDS_p, GFq_bits), 'pmod_mat_syst_n_2n_n_0_1')
    generate_systemizer_file(name, Context(m, 2*m, m, False, True, MEDS_p, GFq_bits), 'pmod_mat_syst_m_2m_m_0_1')
    generate_systemizer_file(name, Context(k, 2*k, k, False, True, MEDS_p, GFq_bits), 'pmod_mat_syst_k_2k_k_0_1')
    generate_systemizer_file(name, Context(n, 2*m, n-1, False, True, MEDS_p, GFq_bits), 'pmod_mat_syst_n_2m_nr1_0_1')
    generate_systemizer_file(name, Context(m-1, m, m-1, True, True, MEDS_p, GFq_bits), 'pmod_mat_syst_mr1_m_mr1_1_1')
    # generate_systemizer_file(name, Context(2*m*n, m*m+n*n, 2*m*n, True, True, MEDS_p, GFq_bits), 'pmod_mat_syst_2mn_mmann_2mn_1_1')