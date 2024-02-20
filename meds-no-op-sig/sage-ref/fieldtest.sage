from util import *

q = 101

GFq = GF(q)

for val in range(q + 5):
    elem = GFq(val)
    print('val:', val, ' elem:', elem)