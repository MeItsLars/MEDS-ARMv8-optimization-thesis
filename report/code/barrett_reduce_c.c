uint16x4_t reduce(uint32x4_t a)
{
  // Compute a*m for the lower and higher registers
  uint64x2_t low = vmull_u32(vget_low_u32(a), vget_low_u32(MAGIC_VEC));
  uint64x2_t high = vmull_high_u32(a, MAGIC_VEC);
  // Combine low and high parts. Gets rid of least significant 32 bits of each element, effectively executing a right shift by 32
  uint32x4_t zip = vuzp2q_u32((uint32x4_t)low, (uint32x4_t)high);
  // Right shift by 11 (remaining part)
  uint32x4_t val = vshrq_n_u32(zip, 11);
  // Multiply by MEDS_p and subtract from a
  uint32x4_t result = vmlsq_u32(a, val, MEDS_p_VEC_32x4);
  // (Optional) shrink to uint16x4_t
  return vmovn_u32(result);
}