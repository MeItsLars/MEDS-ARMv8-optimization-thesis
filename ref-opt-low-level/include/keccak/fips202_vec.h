#ifndef FIPS202_VEC_H
#define FIPS202_VEC_H

#include <stddef.h>
#include <stdint.h>

#define SHAKE256_RATE 136

typedef struct {
  uint64_t s[100];
  unsigned int pos;
} keccak_state_vec4;

extern void keccak_f1600_x4_hybrid_asm_v3p(uint64_t state[4*25]);

void shake256_init_vec4(keccak_state_vec4 *state);
void shake256_absorb_vec4(keccak_state_vec4 *state, const uint8_t *in1, const uint8_t *in2, const uint8_t *in3, const uint8_t *in4, size_t inlen);
void shake256_finalize_vec4(keccak_state_vec4 *state);
void shake256_squeeze_vec4(uint8_t *out1, uint8_t *out2, uint8_t *out3, uint8_t *out4, size_t outlen, keccak_state_vec4 *state);

#endif
