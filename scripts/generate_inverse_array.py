MEDS_p = 4093

def get_mod_inverse(val):
    t0 = val
    t1 = (t0 * t0) % MEDS_p
    t2 = (t1 * t1) % MEDS_p
    t3 = (t2 * t0) % MEDS_p
    t4 = (t3 * t3) % MEDS_p
    t5 = (t4 * t3) % MEDS_p
    t6 = (t5 * t5) % MEDS_p
    t7 = (t6 * t6) % MEDS_p
    t8 = (t7 * t7) % MEDS_p
    t9 = (t8 * t8) % MEDS_p
    t10 = (t9 * t5) % MEDS_p
    t11 = (t10 * t10) % MEDS_p
    t12 = (t11 * t11) % MEDS_p
    t13 = (t12 * t12) % MEDS_p
    t14 = (t13 * t3) % MEDS_p
    t15 = (t14 * t14) % MEDS_p
    t16 = (t15 * t0) % MEDS_p
    return t16

total = 0

print('uint16_t mod_inverses[] = {', end='')
for i in range(0, MEDS_p):
    total += 1
    if i != 0:
        print(' ', end='')
    print(get_mod_inverse(i), end='')
    if i != MEDS_p - 1:
        print(',', end='')
print('};')

print(f'\nDone. Generated {total} mod inverses.')