magic = 0x8018_0481
MEDS_p = 4093

def reduce(i):
    return i % 4093

# def reduce_ct(i):
#     t1 = i * 0x31
#     t2 = i - t1
#     t3 = t1 + t2 >> 1
#     t4 = t3 >> 11
#     t5 = t4 * 4093
#     return t5

def reduce_ct(i):
    t1 = i * magic
    t1 = t1 >> 43
    return i - t1 * MEDS_p

# def test():
#     for i in range(10000):
#         if reduce(i) != reduce_ct(i):
#             print("Error:")
#             print("reduce({}) = {}".format(i, reduce(i)))
#             print("reduce_ct({}) = {}".format(i, reduce_ct(i)))
#             return
#     print("All tests passed")

# test()

def reference(factor, tmp0, tmp1):
    return (MEDS_p + tmp1 - ((tmp0 * factor) % MEDS_p)) % MEDS_p

def alternative(factor, tmp0, tmp1):
    # val = factor * tmp0
    val = factor * tmp0
    
    # val = mod reduce val

    # Option 1:
    # v0v1 = val * magic
    # v0v1 = v0v1 >> 43
    # v0v1 = v0v1 * MEDS_p
    # val = v0v1 - val
    # val = val + tmp1 + MEDS_p
    # val = v0v1 - val + tmp1 + MEDS_p

    # Option 2:
    # val = val - v0v1
    # val = MEDS_p + tmp1 - val
    # val = MEDS_p + tmp1 - (val - v0v1)
    # val = MEDS_p + tmp1 - val + v0v1
    val = reduce_ct(val)
    val = tmp1 + MEDS_p - val

    # val = val % MEDS_p
    # v3v4 = v0v1 * magic
    # v3v4 = v3v4 >> 43
    # v0v1 = v0v1 - v3v4 * MEDS_p
    val = reduce_ct(val)

    return val

def test(factor, tmp0, tmp1):
    res1 = reference(factor, tmp0, tmp1)
    res2 = alternative(factor, tmp0, tmp1)
    if res1 != res2:
        print("Error:")
        print("reference({}, {}, {}) = {}".format(factor, tmp0, tmp1, res1))
        print("alternative({}, {}, {}) = {}".format(factor, tmp0, tmp1, res2))
        return False
    else:
        print("Success")
        return True

test(2131, 4222, 3232)
test(400, 32, 424)
test(10, 10, 20)
test(1, 1, 1)