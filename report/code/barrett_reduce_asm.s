// Compute a*m for the lower and higher registers
umull v3.2d, v0.2s, v1.2s
umull2 v4.2d, v0.4s, v1.4s
// Combine low and high parts. Gets rid of least significant 32 bits of each element, effectively executing a right shift by 32
uzp2 v3.4s, v3.4s, v4.4s
// Right shift by 11 (remaining part)
ushr v3.4s, v3.4s, 11
// Multiply by MEDS_p and subtract from a
mls v0.4s, v3.4s, v2.4s
// (Optional) shrink to 16-bit lane
xtn v0.4h, v0.4s