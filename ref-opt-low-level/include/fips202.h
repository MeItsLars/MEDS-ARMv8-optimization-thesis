#ifndef FIPS202_H
#define FIPS202_H

#include <stddef.h>
#include <stdint.h>

#define SHAKE128_RATE 168
#define SHAKE256_RATE 136
#define SHA3_256_RATE 136
#define SHA3_512_RATE 72

#define FIPS202_NAMESPACE(s) meds_fips202_ref_##s

typedef struct {
  uint64_t s[25];
  unsigned int pos;
} keccak_state;

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

#define shake128_init FIPS202_NAMESPACE(shake128_init)
void shake128_init(keccak_state *state);
#define shake128_absorb FIPS202_NAMESPACE(shake128_absorb)
void shake128_absorb(keccak_state *state, const uint8_t *in, size_t inlen);
#define shake128_finalize FIPS202_NAMESPACE(shake128_finalize)
void shake128_finalize(keccak_state *state);
#define shake128_squeeze FIPS202_NAMESPACE(shake128_squeeze)
void shake128_squeeze(uint8_t *out, size_t outlen, keccak_state *state);
#define shake128_absorb_once FIPS202_NAMESPACE(shake128_absorb_once)
void shake128_absorb_once(keccak_state *state, const uint8_t *in, size_t inlen);
#define shake128_squeezeblocks FIPS202_NAMESPACE(shake128_squeezeblocks)
void shake128_squeezeblocks(uint8_t *out, size_t nblocks, keccak_state *state);

#define shake256_init FIPS202_NAMESPACE(shake256_init)
void shake256_init(keccak_state *state);
#define shake256_absorb FIPS202_NAMESPACE(shake256_absorb)
void shake256_absorb(keccak_state *state, const uint8_t *in, size_t inlen);
#define shake256_finalize FIPS202_NAMESPACE(shake256_finalize)
void shake256_finalize(keccak_state *state);
#define shake256_squeeze FIPS202_NAMESPACE(shake256_squeeze)
void shake256_squeeze(uint8_t *out, size_t outlen, keccak_state *state);
#define shake256_absorb_once FIPS202_NAMESPACE(shake256_absorb_once)
void shake256_absorb_once(keccak_state *state, const uint8_t *in, size_t inlen);
#define shake256_squeezeblocks FIPS202_NAMESPACE(shake256_squeezeblocks)
void shake256_squeezeblocks(uint8_t *out, size_t nblocks,  keccak_state *state);

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

#define shake128 FIPS202_NAMESPACE(shake128)
void shake128(uint8_t *out, size_t outlen, const uint8_t *in, size_t inlen);
#define shake256 FIPS202_NAMESPACE(shake256)
void shake256(uint8_t *out, size_t outlen, const uint8_t *in, size_t inlen);
#define sha3_256 FIPS202_NAMESPACE(sha3_256)
void sha3_256(uint8_t h[32], const uint8_t *in, size_t inlen);
#define sha3_512 FIPS202_NAMESPACE(sha3_512)
void sha3_512(uint8_t h[64], const uint8_t *in, size_t inlen);

#endif
