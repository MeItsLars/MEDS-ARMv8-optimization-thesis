#include "vec_16x4.h"

uint16x4_t reduce_vec(uint32x4_t a)
{
    uint32x4_t t;
    t = vshrq_n_u32(a, GFq_bits);
    t = vmulq_n_u32(t, MEDS_p);
    a = vsubq_u32(a, t);
    t = vshrq_n_u32(a, GFq_bits);
    t = vmulq_n_u32(t, MEDS_p);
    a = vsubq_u32(a, t);
    return vqmovn_u32(a);
}

uint16x4_t freeze_vec(uint16x4_t a)
{
    uint16x4_t t;
    t = vcge_u16(a, MEDS_p_VEC);
    t = vand_u16(t, MEDS_p_VEC);
    return vsub_u16(a, t);
}