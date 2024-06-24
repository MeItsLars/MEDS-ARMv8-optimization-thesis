# magic = 0x8018_0481
import random


k = 43
MEDS_p = 4093
magic = 2**k // MEDS_p + 1

print("magic:", magic)
print("k:", k)
print("MEDS_p:", MEDS_p)

def reduce(i):
    return i % 4093

def reduce_ct(i, k, magic):
    t1 = i * magic
    t1 = t1 >> k
    res = i - t1 * MEDS_p
    # if res >= MEDS_p:
    #     res -= MEDS_p
    return res

def reduce_ct2(i):
    t1 = i * magic
    t1_top_half = t1 >> 32
    t1_top_half = t1_top_half >> 11
    t1 = t1_top_half * MEDS_p
    return i - t1

def test_with_magic_value(k, magic, until):
    t = 1
    while t < until:
        for j in range(100000):
            i = t + random.randint(0, t)
            if reduce(i) != reduce_ct(i, k, magic):
                print("Error:")
                print("reduce({}) = {}".format(i, reduce(i)))
                print("reduce_ct({}) = {}".format(i, reduce_ct(i, k, magic)))
                return False
        t *= 2
    print("All tests passed")
    return True

def find_smallest_k(max_input):
    for i in range(12, 64):
        k = i
        magic = 2**k // MEDS_p + 1
        print(f"Attempting with k = {k}, magic = {hex(magic)}")
        success = test_with_magic_value(k, magic, max_input)
        if success:
            print("Success")
            return
        else:
            print("Failed")

find_smallest_k(2**13)
