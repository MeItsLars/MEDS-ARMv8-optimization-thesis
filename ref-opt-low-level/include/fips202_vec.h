#ifndef FIPS202_VEC_H
#define FIPS202_VEC_H

#include <stddef.h>
#include <stdint.h>

#define SHAKE256_RATE 136

typedef struct {
  uint64_t s[50];
  unsigned int pos;
} keccak_state_vec2;

typedef struct {
  uint64_t s[100];
  unsigned int pos;
} keccak_state_vec4;

typedef struct {
  uint64_t s[125];
  unsigned int pos;
} keccak_state_vec5;

extern void keccak_f1600_x3_hybrid_asm_v3p(uint64_t state[3*25]);
extern void keccak_f1600_x4_hybrid_asm_v3p(uint64_t state[4*25]);
extern void keccak_f1600_x5_hybrid_asm_v8(uint64_t state[5*25]);

void shake256_init_vec2(keccak_state_vec2 *state);
void shake256_absorb_vec2(keccak_state_vec2 *state, const uint8_t *in1, const uint8_t *in2, size_t inlen);
void shake256_finalize_vec2(keccak_state_vec2 *state);
void shake256_squeeze_vec2(uint8_t *out1, uint8_t *out2, size_t outlen, keccak_state_vec2 *state);

void shake256_init_vec4(keccak_state_vec4 *state);
void shake256_absorb_vec4(keccak_state_vec4 *state, const uint8_t *in1, const uint8_t *in2, const uint8_t *in3, const uint8_t *in4, size_t inlen);
void shake256_finalize_vec4(keccak_state_vec4 *state);
void shake256_squeeze_vec4(uint8_t *out1, uint8_t *out2, uint8_t *out3, uint8_t *out4, size_t outlen, keccak_state_vec4 *state);

void shake256_init_vec5(keccak_state_vec5 *state);
void shake256_absorb_vec5(keccak_state_vec5 *state, const uint8_t *in1, const uint8_t *in2, const uint8_t *in3, const uint8_t *in4, const uint8_t *in5, size_t inlen);
void shake256_finalize_vec5(keccak_state_vec5 *state);
void shake256_squeeze_vec5(uint8_t *out1, uint8_t *out2, uint8_t *out3, uint8_t *out4, uint8_t *out5, size_t outlen, keccak_state_vec5 *state);

#endif
