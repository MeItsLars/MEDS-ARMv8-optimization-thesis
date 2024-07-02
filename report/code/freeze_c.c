uint16x8_t freeze(uint16x8_t a)
{
    // Create mask of 1^* for all lanes where a >= MEDS_p
    uint16x8_t mask = vcgeq_u16(a, MEDS_p_VEC_16x8);
    // Create vector of MEDS_p for all lanes with a >= MEDS_p
    uint16x8_t val = vandq_u16(mask, MEDS_p_VEC_16x8);
    // Subtract MEDS_p from all lanes where a >= MEDS_p
    return vsubq_u16(a, val);
}