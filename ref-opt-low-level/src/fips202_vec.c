/* Based on the public domain implementation in crypto_hash/keccakc512/simple/ from
 * http://bench.cr.yp.to/supercop.html by Ronny Van Keer and the public domain "TweetFips202"
 * implementation from https://twitter.com/tweetfips202 by Gilles Van Assche, Daniel J. Bernstein,
 * and Peter Schwabe */

#include <stddef.h>
#include <stdint.h>
#include "fips202_vec.h"
#include "profiler.h"
#include "keccakx2_C.h"

/*************************************************
 * Name:        keccak_init
 *
 * Description: Initializes the Keccak state.
 *
 * Arguments:   - uint64_t *s: pointer to Keccak state
 **************************************************/
static void keccak_init_vec2(uint64_t s[50])
{
  unsigned int i;
  for (i = 0; i < 50; i++)
    s[i] = 0;
}
static void keccak_init_vec4(uint64_t s[100])
{
  unsigned int i;
  for (i = 0; i < 100; i++)
    s[i] = 0;
}
static void keccak_init_vec5(uint64_t s[125])
{
  unsigned int i;
  for (i = 0; i < 125; i++)
    s[i] = 0;
}

/*************************************************
 * Name:        keccak_absorb
 *
 * Description: Absorb step of Keccak; incremental.
 *
 * Arguments:   - uint64_t *s: pointer to Keccak state
 *              - unsigned int pos: position in current block to be absorbed
 *              - unsigned int r: rate in bytes (e.g., 168 for SHAKE128)
 *              - const uint8_t *in: pointer to input to be absorbed into s
 *              - size_t inlen: length of input in bytes
 *
 * Returns new position pos in current block
 **************************************************/
static unsigned int keccak_absorb_vec2(uint64_t s[50],
                                       unsigned int pos,
                                       unsigned int r,
                                       const uint8_t *in1,
                                       const uint8_t *in2,
                                       size_t inlen)
{
  unsigned int i;

  while (pos + inlen >= r)
  {
    for (i = pos; i < r; i++)
    {
      s[2 * (i / 8)] ^= (uint64_t)*in1++ << 8 * (i % 8);
      s[1 + 2 * (i / 8)] ^= (uint64_t)*in2++ << 8 * (i % 8);
    }
    inlen -= r - pos;
    keccak_f1600_x2_scalar_C(s);
    pos = 0;
  }

  for (i = pos; i < pos + inlen; i++)
  {
    s[2 * (i / 8)] ^= (uint64_t)*in1++ << 8 * (i % 8);
    s[1 + 2 * (i / 8)] ^= (uint64_t)*in2++ << 8 * (i % 8);
  }

  return i;
}
static unsigned int keccak_absorb_vec4(uint64_t s[100],
                                       unsigned int pos,
                                       unsigned int r,
                                       const uint8_t *in1,
                                       const uint8_t *in2,
                                       const uint8_t *in3,
                                       const uint8_t *in4,
                                       size_t inlen)
{
  unsigned int i;

  while (pos + inlen >= r)
  {
    for (i = pos; i < r; i++)
    {
      s[4 * (i / 8)] ^= (uint64_t)*in1++ << 8 * (i % 8);
      s[1 + 4 * (i / 8)] ^= (uint64_t)*in2++ << 8 * (i % 8);
      s[2 + 4 * (i / 8)] ^= (uint64_t)*in3++ << 8 * (i % 8);
      s[3 + 4 * (i / 8)] ^= (uint64_t)*in4++ << 8 * (i % 8);
    }
    inlen -= r - pos;
    keccak_f1600_x4_hybrid_asm_v3p(s);
    pos = 0;
  }

  for (i = pos; i < pos + inlen; i++)
  {
    s[4 * (i / 8)] ^= (uint64_t)*in1++ << 8 * (i % 8);
    s[1 + 4 * (i / 8)] ^= (uint64_t)*in2++ << 8 * (i % 8);
    s[2 + 4 * (i / 8)] ^= (uint64_t)*in3++ << 8 * (i % 8);
    s[3 + 4 * (i / 8)] ^= (uint64_t)*in4++ << 8 * (i % 8);
  }

  return i;
}
static unsigned int keccak_absorb_vec5(uint64_t s[125],
                                       unsigned int pos,
                                       unsigned int r,
                                       const uint8_t *in1,
                                       const uint8_t *in2,
                                       const uint8_t *in3,
                                       const uint8_t *in4,
                                       const uint8_t *in5,
                                       size_t inlen)
{
  unsigned int i;

  while (pos + inlen >= r)
  {
    for (i = pos; i < r; i++)
    {
      s[2 * (i / 8)] ^= (uint64_t)*in1++ << 8 * (i % 8);
      s[1 + 2 * (i / 8)] ^= (uint64_t)*in2++ << 8 * (i % 8);
      s[50 + (i / 8)] ^= (uint64_t)*in3++ << 8 * (i % 8);
      s[75 + (i / 8)] ^= (uint64_t)*in4++ << 8 * (i % 8);
      s[100 + (i / 8)] ^= (uint64_t)*in5++ << 8 * (i % 8);
    }
    inlen -= r - pos;
    keccak_f1600_x5_hybrid_asm_v8(s);
    pos = 0;
  }

  for (i = pos; i < pos + inlen; i++)
  {
    s[2 * (i / 8)] ^= (uint64_t)*in1++ << 8 * (i % 8);
    s[1 + 2 * (i / 8)] ^= (uint64_t)*in2++ << 8 * (i % 8);
    s[50 + (i / 8)] ^= (uint64_t)*in3++ << 8 * (i % 8);
    s[75 + (i / 8)] ^= (uint64_t)*in4++ << 8 * (i % 8);
    s[100 + (i / 8)] ^= (uint64_t)*in5++ << 8 * (i % 8);
  }

  return i;
}

/*************************************************
 * Name:        keccak_finalize
 *
 * Description: Finalize absorb step.
 *
 * Arguments:   - uint64_t *s: pointer to Keccak state
 *              - unsigned int pos: position in current block to be absorbed
 *              - unsigned int r: rate in bytes (e.g., 168 for SHAKE128)
 *              - uint8_t p: domain separation byte
 **************************************************/
static void keccak_finalize_vec2(uint64_t s[50], unsigned int pos, unsigned int r, uint8_t p)
{
  s[2 * (pos / 8)] ^= (uint64_t)p << 8 * (pos % 8);
  s[2 * (r / 8 - 1)] ^= 1ULL << 63;
  s[1 + 2 * (pos / 8)] ^= (uint64_t)p << 8 * (pos % 8);
  s[1 + 2 * (r / 8 - 1)] ^= 1ULL << 63;
}
static void keccak_finalize_vec4(uint64_t s[100], unsigned int pos, unsigned int r, uint8_t p)
{
  s[4 * (pos / 8)] ^= (uint64_t)p << 8 * (pos % 8);
  s[4 * (r / 8 - 1)] ^= 1ULL << 63;
  s[1 + 4 * (pos / 8)] ^= (uint64_t)p << 8 * (pos % 8);
  s[1 + 4 * (r / 8 - 1)] ^= 1ULL << 63;
  s[2 + 4 * (pos / 8)] ^= (uint64_t)p << 8 * (pos % 8);
  s[2 + 4 * (r / 8 - 1)] ^= 1ULL << 63;
  s[3 + 4 * (pos / 8)] ^= (uint64_t)p << 8 * (pos % 8);
  s[3 + 4 * (r / 8 - 1)] ^= 1ULL << 63;
}
static void keccak_finalize_vec5(uint64_t s[125], unsigned int pos, unsigned int r, uint8_t p)
{
  s[2 * (pos / 8)] ^= (uint64_t)p << 8 * (pos % 8);
  s[2 * (r / 8 - 1)] ^= 1ULL << 63;
  s[1 + 2 * (pos / 8)] ^= (uint64_t)p << 8 * (pos % 8);
  s[1 + 2 * (r / 8 - 1)] ^= 1ULL << 63;
  s[50 + (pos / 8)] ^= (uint64_t)p << 8 * (pos % 8);
  s[50 + (r / 8 - 1)] ^= 1ULL << 63;
  s[75 + (pos / 8)] ^= (uint64_t)p << 8 * (pos % 8);
  s[75 + (r / 8 - 1)] ^= 1ULL << 63;
  s[100 + (pos / 8)] ^= (uint64_t)p << 8 * (pos % 8);
  s[100 + (r / 8 - 1)] ^= 1ULL << 63;
}

/*************************************************
 * Name:        keccak_squeeze
 *
 * Description: Squeeze step of Keccak. Squeezes arbitratrily many bytes.
 *              Modifies the state. Can be called multiple times to keep
 *              squeezing, i.e., is incremental.
 *
 * Arguments:   - uint8_t *out: pointer to output
 *              - size_t outlen: number of bytes to be squeezed (written to out)
 *              - uint64_t *s: pointer to input/output Keccak state
 *              - unsigned int pos: number of bytes in current block already squeezed
 *              - unsigned int r: rate in bytes (e.g., 168 for SHAKE128)
 *
 * Returns new position pos in current block
 **************************************************/
static unsigned int keccak_squeeze_vec2(uint8_t *out1, uint8_t *out2, size_t outlen, uint64_t s[50], unsigned int pos, unsigned int r)
{
  unsigned int i;

  while (outlen)
  {
    if (pos == r)
    {
      keccak_f1600_x2_scalar_C(s);
      pos = 0;
    }
    for (i = pos; i < r && i < pos + outlen; i++)
    {
      *out1++ = s[2 * (i / 8)] >> 8 * (i % 8);
      *out2++ = s[1 + 2 * (i / 8)] >> 8 * (i % 8);
    }
    outlen -= i - pos;
    pos = i;
  }

  return pos;
}
static unsigned int keccak_squeeze_vec4(uint8_t *out1, uint8_t *out2, uint8_t *out3, uint8_t *out4, size_t outlen, uint64_t s[100], unsigned int pos, unsigned int r)
{
  unsigned int i;

  while (outlen)
  {
    if (pos == r)
    {
      keccak_f1600_x4_hybrid_asm_v3p(s);
      pos = 0;
    }
    for (i = pos; i < r && i < pos + outlen; i++)
    {
      *out1++ = s[4 * (i / 8)] >> 8 * (i % 8);
      *out2++ = s[1 + 4 * (i / 8)] >> 8 * (i % 8);
      *out3++ = s[2 + 4 * (i / 8)] >> 8 * (i % 8);
      *out4++ = s[3 + 4 * (i / 8)] >> 8 * (i % 8);
    }
    outlen -= i - pos;
    pos = i;
  }

  return pos;
}
static unsigned int keccak_squeeze_vec5(uint8_t *out1, uint8_t *out2, uint8_t *out3, uint8_t *out4, uint8_t *out5, size_t outlen, uint64_t s[125], unsigned int pos, unsigned int r)
{
  unsigned int i;

  while (outlen)
  {
    if (pos == r)
    {
      keccak_f1600_x5_hybrid_asm_v8(s);
      pos = 0;
    }
    for (i = pos; i < r && i < pos + outlen; i++)
    {
      *out1++ = s[2 * (i / 8)] >> 8 * (i % 8);
      *out2++ = s[1 + 2 * (i / 8)] >> 8 * (i % 8);
      *out3++ = s[50 + (i / 8)] >> 8 * (i % 8);
      *out4++ = s[75 + (i / 8)] >> 8 * (i % 8);
      *out5++ = s[100 + (i / 8)] >> 8 * (i % 8);
    }
    outlen -= i - pos;
    pos = i;
  }

  return pos;
}

/*************************************************
 * Name:        shake256_init
 *
 * Description: Initilizes Keccak state for use as SHAKE256 XOF
 *
 * Arguments:   - keccak_state *state: pointer to (uninitialized) Keccak state
 **************************************************/
void shake256_init_vec2(keccak_state_vec2 *state)
{
  keccak_init_vec2(state->s);
  state->pos = 0;
}
void shake256_init_vec4(keccak_state_vec4 *state)
{
  keccak_init_vec4(state->s);
  state->pos = 0;
}
void shake256_init_vec5(keccak_state_vec5 *state)
{
  keccak_init_vec5(state->s);
  state->pos = 0;
}

/*************************************************
 * Name:        shake256_absorb
 *
 * Description: Absorb step of the SHAKE256 XOF; incremental.
 *
 * Arguments:   - keccak_state *state: pointer to (initialized) output Keccak state
 *              - const uint8_t *in: pointer to input to be absorbed into s
 *              - size_t inlen: length of input in bytes
 **************************************************/
void shake256_absorb_vec2(keccak_state_vec2 *state, const uint8_t *in1, const uint8_t *in2, size_t inlen)
{
  PROFILER_START("shake256_absorb");
  state->pos = keccak_absorb_vec2(state->s, state->pos, SHAKE256_RATE, in1, in2, inlen);
  PROFILER_STOP("shake256_absorb");
}
void shake256_absorb_vec4(keccak_state_vec4 *state, const uint8_t *in1, const uint8_t *in2, const uint8_t *in3, const uint8_t *in4, size_t inlen)
{
  PROFILER_START("shake256_absorb");
  state->pos = keccak_absorb_vec4(state->s, state->pos, SHAKE256_RATE, in1, in2, in3, in4, inlen);
  PROFILER_STOP("shake256_absorb");
}
void shake256_absorb_vec5(keccak_state_vec5 *state, const uint8_t *in1, const uint8_t *in2, const uint8_t *in3, const uint8_t *in4, const uint8_t *in5, size_t inlen)
{
  PROFILER_START("shake256_absorb");
  state->pos = keccak_absorb_vec5(state->s, state->pos, SHAKE256_RATE, in1, in2, in3, in4, in5, inlen);
  PROFILER_STOP("shake256_absorb");
}

/*************************************************
 * Name:        shake256_finalize
 *
 * Description: Finalize absorb step of the SHAKE256 XOF.
 *
 * Arguments:   - keccak_state *state: pointer to Keccak state
 **************************************************/
void shake256_finalize_vec2(keccak_state_vec2 *state)
{
  PROFILER_START("shake256_finalize");
  keccak_finalize_vec2(state->s, state->pos, SHAKE256_RATE, 0x1F);
  state->pos = SHAKE256_RATE;
  PROFILER_STOP("shake256_finalize");
}
void shake256_finalize_vec4(keccak_state_vec4 *state)
{
  PROFILER_START("shake256_finalize");
  keccak_finalize_vec4(state->s, state->pos, SHAKE256_RATE, 0x1F);
  state->pos = SHAKE256_RATE;
  PROFILER_STOP("shake256_finalize");
}
void shake256_finalize_vec5(keccak_state_vec5 *state)
{
  PROFILER_START("shake256_finalize");
  keccak_finalize_vec5(state->s, state->pos, SHAKE256_RATE, 0x1F);
  state->pos = SHAKE256_RATE;
  PROFILER_STOP("shake256_finalize");
}

/*************************************************
 * Name:        shake256_squeeze
 *
 * Description: Squeeze step of SHAKE256 XOF. Squeezes arbitraily many
 *              bytes. Can be called multiple times to keep squeezing.
 *
 * Arguments:   - uint8_t *out: pointer to output blocks
 *              - size_t outlen : number of bytes to be squeezed (written to output)
 *              - keccak_state *s: pointer to input/output Keccak state
 **************************************************/
void shake256_squeeze_vec2(uint8_t *out1, uint8_t *out2, size_t outlen, keccak_state_vec2 *state)
{
  PROFILER_START("shake256_squeeze");
  state->pos = keccak_squeeze_vec2(out1, out2, outlen, state->s, state->pos, SHAKE256_RATE);
  PROFILER_STOP("shake256_squeeze");
}
void shake256_squeeze_vec4(uint8_t *out1, uint8_t *out2, uint8_t *out3, uint8_t *out4, size_t outlen, keccak_state_vec4 *state)
{
  PROFILER_START("shake256_squeeze");
  state->pos = keccak_squeeze_vec4(out1, out2, out3, out4, outlen, state->s, state->pos, SHAKE256_RATE);
  PROFILER_STOP("shake256_squeeze");
}
void shake256_squeeze_vec5(uint8_t *out1, uint8_t *out2, uint8_t *out3, uint8_t *out4, uint8_t *out5, size_t outlen, keccak_state_vec5 *state)
{
  PROFILER_START("shake256_squeeze");
  state->pos = keccak_squeeze_vec5(out1, out2, out3, out4, out5, outlen, state->s, state->pos, SHAKE256_RATE);
  PROFILER_STOP("shake256_squeeze");
}
