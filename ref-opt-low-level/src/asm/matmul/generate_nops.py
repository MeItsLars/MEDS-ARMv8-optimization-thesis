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

def generate_mat_mul_asm(context):
    asm = []
    # Header and function information
    add(asm, 0, ".cpu cortex-a72")
    add(asm, 0, ".arch armv8-a")
    fun_id = f"nop_test"
    add(asm, 0, f".global {fun_id}")
    add(asm, 0, f"{fun_id}:")
    
    add(asm, 1, "mov x1, #0")

    add(asm, 0, "loop:")

    for i in range(0, 10000):
        add(asm, 1, "nop")
    
    add(asm, 1, "add x1, x1, #1")
    add(asm, 1, "cmp x1, #10")
    add(asm, 1, "b.lt loop")

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

# fun_id, asm = generate_mat_mul_asm(Context(24, 67, 4, 4093, 12))
# with open(f"{dir_path}/{fun_id}.s", "w") as f:
#     for line in asm:
#         f.write(line + "\n")

# print(f"Generated {fun_id}.s")

fun_id, asm = generate_mat_mul_asm(Context(24, 24, 24*24, 4093, 12))
with open(f"{dir_path}/{fun_id}.s", "w") as f:
    for line in asm:
        f.write(line + "\n")

print(f"Generated {fun_id}.s")