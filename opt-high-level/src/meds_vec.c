#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "log.h"
#include "profiler.h"

#include "fips202.h"
#include "fips202_vec.h"
#include "params.h"
#include "api.h"
#include "randombytes.h"
#include "seed.h"
#include "util.h"
#include "util_vec.h"
#include "matrixmod.h"
#include "matrixmod_vec.h"
#include "bitstream.h"
#include "meds_vec.h"
#include "vec_16x8.h"

#define solve solve_opt

#define CEILING(x, y) (((x) + (y) - 1) / (y))

int crypto_sign_keypair_vec(
    unsigned char *pk,
    unsigned char *sk)
{
  uint8_t delta[MEDS_sec_seed_bytes];

  randombytes(delta, MEDS_sec_seed_bytes);

  pmod_mat_t G_data[MEDS_k * MEDS_m * MEDS_n * MEDS_s];
  pmod_mat_t *G[MEDS_s];

  for (int i = 0; i < MEDS_s; i++)
    G[i] = &G_data[i * MEDS_k * MEDS_m * MEDS_n];

  uint8_t sigma_G0[MEDS_pub_seed_bytes];
  uint8_t sigma[MEDS_sec_seed_bytes];

  XOF((uint8_t *[]){sigma_G0, sigma},
      (size_t[]){MEDS_pub_seed_bytes, MEDS_sec_seed_bytes},
      delta, MEDS_sec_seed_bytes,
      2);

  LOG_VEC(sigma, MEDS_sec_seed_bytes);
  LOG_VEC_FMT(sigma_G0, MEDS_pub_seed_bytes, "sigma_G0");

  rnd_sys_mat(G[0], MEDS_k, MEDS_m * MEDS_n, sigma_G0, MEDS_pub_seed_bytes);

  LOG_MAT(G[0], MEDS_k, MEDS_m * MEDS_n);

  pmod_mat_t A_inv_data[MEDS_s * MEDS_m * MEDS_m];
  pmod_mat_t B_inv_data[MEDS_s * MEDS_n * MEDS_n];
  pmod_mat_t T_inv_data[MEDS_s * MEDS_k * MEDS_k];

  pmod_mat_t *A_inv[MEDS_s];
  pmod_mat_t *B_inv[MEDS_s];
  pmod_mat_t *T_inv[MEDS_s];

  for (int i = 0; i < MEDS_s; i++)
  {
    A_inv[i] = &A_inv_data[i * MEDS_m * MEDS_m];
    B_inv[i] = &B_inv_data[i * MEDS_n * MEDS_n];
    T_inv[i] = &T_inv_data[i * MEDS_k * MEDS_k];
  }

  for (int i = 1; i < MEDS_s; i++)
  {
    pmod_mat_t A[MEDS_m * MEDS_m] = {0};
    pmod_mat_t B[MEDS_n * MEDS_n] = {0};

    while (1 == 1) // redo generation for this index until success
    {
      uint8_t sigma_Ti[MEDS_sec_seed_bytes];

      XOF((uint8_t *[]){sigma_Ti, sigma},
          (size_t[]){MEDS_sec_seed_bytes, MEDS_sec_seed_bytes},
          sigma, MEDS_sec_seed_bytes,
          2);

      pmod_mat_t Ti[MEDS_k * MEDS_k];

      rnd_inv_matrix(Ti, MEDS_k, MEDS_k, sigma_Ti, MEDS_sec_seed_bytes);

      LOG_MAT(Ti, MEDS_k, MEDS_k);

      pmod_mat_t G0prime[MEDS_k * MEDS_m * MEDS_n];

      pmod_mat_mul(G0prime, MEDS_k, MEDS_m * MEDS_n,
                   Ti, MEDS_k, MEDS_k,
                   G[0], MEDS_k, MEDS_m * MEDS_n);

      LOG_MAT(G0prime, MEDS_k, MEDS_m * MEDS_n);

      if (solve(A, B_inv[i], G0prime) < 0)
      {
        LOG("no sol");
        continue;
      }

      if (pmod_mat_inv(B, B_inv[i], MEDS_n, MEDS_n) < 0)
      {
        LOG("no inv B");
        continue;
      }

      if (pmod_mat_inv(A_inv[i], A, MEDS_m, MEDS_m) < 0)
      {
        LOG("no inv A_inv");
        continue;
      }

      LOG_MAT_FMT(A, MEDS_m, MEDS_m, "A[%i]", i);
      LOG_MAT_FMT(A_inv[i], MEDS_m, MEDS_m, "A_inv[%i]", i);
      LOG_MAT_FMT(B, MEDS_n, MEDS_n, "B[%i]", i);
      LOG_MAT_FMT(B_inv[i], MEDS_n, MEDS_n, "B_inv[%i]", i);

      pi(G[i], A, B, G[0]);

      LOG_MAT_FMT(G[i], MEDS_k, MEDS_m * MEDS_n, "G[%i]", i);

      for (int r = 0; r < MEDS_k; r++)
        memcpy(&T_inv[i][r * MEDS_k], &G[i][r * MEDS_m * MEDS_n], MEDS_k * sizeof(GFq_t));

      if (SF(G[i], G[i]) != 0)
      {
        LOG("redo G[%i]", i);
        continue; // Not systematic; try again for index i.
      }

      LOG_MAT_FMT(G[i], MEDS_k, MEDS_m * MEDS_n, "G[%i]", i);
      LOG_MAT_FMT(T_inv[i], MEDS_k, MEDS_k, "T_inv[%i]", i);

      // successfull generated G[s]; break out of while loop
      break;
    }
  }

  // copy pk data
  {
    uint8_t *tmp_pk = pk;

    memcpy(tmp_pk, sigma_G0, MEDS_pub_seed_bytes);
    LOG_VEC(tmp_pk, MEDS_pub_seed_bytes, "sigma_G0 (pk)");
    tmp_pk += MEDS_pub_seed_bytes;

    bitstream_t bs;

    PROFILER_START("bs_fill");
    bs_init(&bs, tmp_pk, MEDS_PK_BYTES - MEDS_pub_seed_bytes);

    for (int si = 1; si < MEDS_s; si++)
    {
      for (int r = 2; r < MEDS_k; r++)
        for (int j = MEDS_k; j < MEDS_m * MEDS_n; j++)
          bs_write(&bs, G[si][r * MEDS_m * MEDS_n + j], GFq_bits);

      bs_finalize(&bs);
    }
    PROFILER_STOP("bs_fill");

    LOG_VEC(tmp_pk, MEDS_PK_BYTES - MEDS_pub_seed_bytes, "G[1:] (pk)");
    tmp_pk += MEDS_PK_BYTES - MEDS_pub_seed_bytes;

    LOG_HEX(pk, MEDS_PK_BYTES);

    if (MEDS_PK_BYTES != MEDS_pub_seed_bytes + bs.byte_pos + (bs.bit_pos > 0 ? 1 : 0))
    {
      fprintf(stderr, "ERROR: MEDS_PK_BYTES and actual pk size do not match! %i vs %i\n", MEDS_PK_BYTES, MEDS_pub_seed_bytes + bs.byte_pos + (bs.bit_pos > 0 ? 1 : 0));
      fprintf(stderr, "%i %i\n", MEDS_pub_seed_bytes + bs.byte_pos, MEDS_pub_seed_bytes + bs.byte_pos + (bs.bit_pos > 0 ? 1 : 0));
      return -1;
    }
  }

  // copy sk data
  {
    memcpy(sk, delta, MEDS_sec_seed_bytes);
    memcpy(sk + MEDS_sec_seed_bytes, sigma_G0, MEDS_pub_seed_bytes);

    bitstream_t bs;

    PROFILER_START("bs_fill");
    bs_init(&bs, sk + MEDS_sec_seed_bytes + MEDS_pub_seed_bytes, MEDS_SK_BYTES - MEDS_sec_seed_bytes - MEDS_pub_seed_bytes);

    for (int si = 1; si < MEDS_s; si++)
    {
      for (int j = 0; j < MEDS_m * MEDS_m; j++)
        bs_write(&bs, A_inv[si][j], GFq_bits);

      bs_finalize(&bs);
    }

    for (int si = 1; si < MEDS_s; si++)
    {
      for (int j = 0; j < MEDS_n * MEDS_n; j++)
        bs_write(&bs, B_inv[si][j], GFq_bits);

      bs_finalize(&bs);
    }

    for (int si = 1; si < MEDS_s; si++)
    {
      for (int j = 0; j < MEDS_k * MEDS_k; j++)
        bs_write(&bs, T_inv[si][j], GFq_bits);

      bs_finalize(&bs);
    }
    PROFILER_STOP("bs_fill");

    LOG_HEX(sk, MEDS_SK_BYTES);
  }

  return 0;
}

int crypto_sign_vec(
    unsigned char *sm, unsigned long long *smlen,
    const unsigned char *m, unsigned long long mlen,
    const unsigned char *sk)
{
  // If MEDS_t is not 8, this optimized version of MEDS cannot be used.
  if (MEDS_t < 8)
  {
    fprintf(stderr, "ERROR: This high-level optimized version of MEDS requires MEDS_t >= 8\n");
    return -1;
  }

  // skip secret seed
  sk += MEDS_sec_seed_bytes;

  // Load G_0 from sk seed
  pmod_mat_t G_0[MEDS_k * MEDS_m * MEDS_n];
  rnd_sys_mat(G_0, MEDS_k, MEDS_m * MEDS_n, sk, MEDS_pub_seed_bytes);
  sk += MEDS_pub_seed_bytes;

  // Initialize matrices
  pmod_mat_t A_inv_data[MEDS_s * MEDS_m * MEDS_m];
  pmod_mat_t B_inv_data[MEDS_s * MEDS_n * MEDS_n];
  pmod_mat_t T_inv_data[MEDS_s * MEDS_k * MEDS_k];

  pmod_mat_t *A_inv[MEDS_s];
  pmod_mat_t *B_inv[MEDS_s];
  pmod_mat_t *T_inv[MEDS_s];

  for (int i = 0; i < MEDS_s; i++)
  {
    A_inv[i] = &A_inv_data[i * MEDS_m * MEDS_m];
    B_inv[i] = &B_inv_data[i * MEDS_n * MEDS_n];
    T_inv[i] = &T_inv_data[i * MEDS_k * MEDS_k];
  }

  // Load secret key matrices.
  {
    bitstream_t bs;

    bs_init(&bs, (uint8_t *)sk, MEDS_SK_BYTES - MEDS_sec_seed_bytes - MEDS_pub_seed_bytes);

    for (int si = 1; si < MEDS_s; si++)
    {
      for (int j = 0; j < MEDS_m * MEDS_m; j++)
        A_inv[si][j] = bs_read(&bs, GFq_bits);

      bs_finalize(&bs);
    }

    for (int si = 1; si < MEDS_s; si++)
    {
      for (int j = 0; j < MEDS_n * MEDS_n; j++)
        B_inv[si][j] = bs_read(&bs, GFq_bits);

      bs_finalize(&bs);
    }

    for (int si = 1; si < MEDS_s; si++)
    {
      for (int j = 0; j < MEDS_k * MEDS_k; j++)
        T_inv[si][j] = bs_read(&bs, GFq_bits);

      bs_finalize(&bs);
    }

    bs_finalize(&bs);
  }

  for (int i = 1; i < MEDS_s; i++)
    LOG_MAT_FMT(A_inv[i], MEDS_m, MEDS_m, "A_inv[%i]", i);

  for (int i = 1; i < MEDS_s; i++)
    LOG_MAT_FMT(B_inv[i], MEDS_n, MEDS_n, "B_inv[%i]", i);

  for (int i = 1; i < MEDS_s; i++)
    LOG_MAT_FMT(T_inv[i], MEDS_k, MEDS_k, "T_inv[%i]", i);

  LOG_MAT(G_0, MEDS_k, MEDS_m * MEDS_n);

  // Generate a random seed delta
  uint8_t delta[MEDS_sec_seed_bytes];
  randombytes(delta, MEDS_sec_seed_bytes);

  LOG_VEC(delta, MEDS_sec_seed_bytes);

  // Generate two random seeds rho and alpha from delta
  uint8_t rho[MEDS_st_salt_bytes] = {0};
  uint8_t alpha[MEDS_st_salt_bytes] = {0};

  XOF((uint8_t *[]){rho, alpha},
      (size_t[]){MEDS_st_seed_bytes, MEDS_st_salt_bytes},
      delta, MEDS_sec_seed_bytes,
      2);

  // Generate seeds from rho and alpha as root and salt
  uint8_t stree[MEDS_st_seed_bytes * SEED_TREE_size] = {0};
  t_hash(stree, rho, alpha, 0, 0);

  // Get the address of the first sigma seed
  uint8_t *sigma = &stree[MEDS_st_seed_bytes * SEED_TREE_ADDR(MEDS_seed_tree_height, 0)];

  for (int i = 0; i < MEDS_t; i++)
  {
    LOG_HEX_FMT((&sigma[i * MEDS_st_seed_bytes]), MEDS_st_seed_bytes, "sigma[%i]", i);
  }

  // Initialize matrices that will be filled during the commitment phase
  // These matrices are still needed for the hash computation
  pmod_mat_t M_tilde_data[MEDS_t << 1][2 * MEDS_k];
  pmod_mat_t *M_tilde[MEDS_t << 1];
  static uint8_t bs_buf_data[MEDS_t << 1][CEILING((MEDS_k * (MEDS_m * MEDS_n - MEDS_k)) * GFq_bits, 8)];
  static uint8_t *bs_buf[MEDS_t << 1];

  for (int i = 0; i < MEDS_t << 1; i++)
  {
    M_tilde[i] = M_tilde_data[i];
    bs_buf[i] = bs_buf_data[i];
  }

  // Define temporary arrays that will be used during a single commitment computation
  pmod_mat_vec_t A_tilde[MEDS_m * MEDS_m];
  pmod_mat_vec_t B_tilde[MEDS_n * MEDS_n];
  pmod_mat_vec_t A_tilde_inv[MEDS_m * MEDS_m];
  pmod_mat_vec_t B_tilde_inv[MEDS_n * MEDS_n];
  pmod_mat_vec_t C[2 * MEDS_m * MEDS_n];
  uint8_t sigma_M_tilde_i[MEDS_pub_seed_bytes];
  pmod_mat_vec_t M_tilde_vec[2 * MEDS_k];
  static pmod_mat_vec_t G_tilde_ti_vec[MEDS_k * MEDS_m * MEDS_n];
  pmod_mat_vec_t G_0_vec[MEDS_k * MEDS_m * MEDS_n];

  // Initialize G_0_vec from G_0 (all lanes are the same)
  for (int i = 0; i < MEDS_k * MEDS_m * MEDS_n; i++)
    G_0_vec[i] = SET_VEC(G_0[i]);

  // Initialize hash function
  keccak_state h_shake;
  shake256_init(&h_shake);

  // Create a seed buffer for use during the challenges. Set the start of the buffer to the salt.
  uint8_t seed_buf[MEDS_st_salt_bytes + MEDS_st_seed_bytes + sizeof(uint32_t)] = {0};
  memcpy(seed_buf, alpha, MEDS_st_salt_bytes);

  // Establish a pointer to the location of the address in the seed buffer
  uint8_t *addr_pos = seed_buf + MEDS_st_salt_bytes + MEDS_st_seed_bytes;

  // Loop variables
  int num_valid = 0;
  int num_invalid = 0;
  int num_tried = 0;
  int indexes[MEDS_t << 1];

  for (int i = 0; i < MEDS_t; i++)
    indexes[i] = i;
  for (int i = MEDS_t; i < (MEDS_t << 1); i++)
    indexes[i] = 0;

  PROFILER_START("SEC_COMMIT");
  while (num_valid < MEDS_t)
  {
    PROFILER_START("SEC_COMMIT_INIT");
    int index = num_tried;

    // Determine the batch size for the current iteration
    int loop_batch_size = MEDS_t + num_invalid - num_tried;
    if (loop_batch_size > BATCH_SIZE)
      loop_batch_size = BATCH_SIZE;

    for (int t = 0; t < loop_batch_size; t++)
    {
      int index_netto = indexes[num_tried + t];

      for (int j = 0; j < 4; j++)
        addr_pos[j] = (index_netto >> (j * 8)) & 0xff;

      memcpy(seed_buf + MEDS_st_salt_bytes, &sigma[index_netto * MEDS_st_seed_bytes], MEDS_st_seed_bytes);

      XOF((uint8_t *[]){sigma_M_tilde_i, &sigma[index_netto * MEDS_st_seed_bytes]},
          (size_t[]){MEDS_pub_seed_bytes, MEDS_st_seed_bytes},
          seed_buf, MEDS_st_salt_bytes + MEDS_st_seed_bytes + sizeof(uint32_t),
          2);

      rnd_matrix(M_tilde[index_netto], 2, MEDS_k, sigma_M_tilde_i, MEDS_pub_seed_bytes);
    }
    PROFILER_STOP("SEC_COMMIT_INIT");

    pmod_mat_s_vec_t valid = SET_S_VEC(1);

    // Load M_tilde_vec from M_tilde[index]
    for (int r = 0; r < 2; r++)
      for (int c = 0; c < MEDS_k; c++)
        M_tilde_vec[r * MEDS_k + c] = load_vec(M_tilde + index, 2, MEDS_k, r, c);

    // Compute C
    pmod_mat_mul_vec(C, 2, MEDS_m * MEDS_n, M_tilde_vec, 2, MEDS_k, G_0_vec, MEDS_k, MEDS_m * MEDS_n);

    // Solve for A_tilde and B_tilde
    valid = AND_S_VEC(valid, TO_S_VEC(GEQ_S_VEC(solve_vec(A_tilde, B_tilde_inv, C, 1), ZERO_S_VEC)));
    valid = AND_S_VEC(valid, TO_S_VEC(GEQ_S_VEC(pmod_mat_inv_vec(B_tilde, B_tilde_inv, MEDS_n, MEDS_n, 1), ZERO_S_VEC)));
    valid = AND_S_VEC(valid, TO_S_VEC(GEQ_S_VEC(pmod_mat_inv_vec(A_tilde_inv, A_tilde, MEDS_m, MEDS_m, 1), ZERO_S_VEC)));

    // Apply pi function
    pi_vec(G_tilde_ti_vec, A_tilde, B_tilde, G_0_vec);

    // Convert to systematic form
    valid = AND_S_VEC(valid, TO_S_VEC(EQ0_S_VEC(SF_vec(G_tilde_ti_vec, G_tilde_ti_vec, 1))));

    // Store G_tilde_ti_vec into G_tilde_ti[index]
    PROFILER_START("bs_fill");
    if (GFq_bits == 12)
    {
      // Use a 12-bit optimized parallel bitstream fill function (12 bits are used for all parameter sets)
      store_bitstream_12bit(bs_buf, index, G_tilde_ti_vec, loop_batch_size);
    }
    else
    {
      // In other cases, use a generic bitstream fill function
      for (int i = 0; i < loop_batch_size; i++)
      {
        bitstream_t bs;

        bs_init(&bs, bs_buf[index + i], CEILING((MEDS_k * (MEDS_m * MEDS_n - MEDS_k)) * GFq_bits, 8));

        for (int r = 0; r < MEDS_k; r++)
          for (int j = MEDS_k; j < MEDS_m * MEDS_n; j++)
            bs_write(&bs, G_tilde_ti_vec[r * MEDS_m * MEDS_n + j][i], GFq_bits);

        bs_finalize(&bs);
      }
    }
    PROFILER_STOP("bs_fill");

    int current_batch_invalids = 0;
    for (int t = 0; t < loop_batch_size; t++)
    {
      if (GET_LANE_S_VEC(valid, t) == VEC_FALSE)
      {
        // Compute indices
        int idx_source = num_tried + t;
        int idx_target = MEDS_t + num_invalid + current_batch_invalids;

        // Change pointers for the next iteration of the failed commitment
        indexes[idx_target] = indexes[idx_source];
        bs_buf[idx_target] = bs_buf_data[idx_source];
        M_tilde[idx_target] = M_tilde_data[idx_source];

        // Update counters
        num_invalid++;
        current_batch_invalids++;
      }
      else
      {
        num_valid++;
      }
    }
    num_tried += loop_batch_size;
  }
  PROFILER_STOP("SEC_COMMIT");

  // Hash all commitments
  PROFILER_START("hash");
#ifdef MEDS_hash_opt
  // Use parallel hashing to compute 4 digests at once
  uint8_t digest_G_tilde_ti[MEDS_t][MEDS_digest_bytes];
  for (int i = 0; i < MEDS_t; i += 4)
  {
    keccak_state_vec4 h_shake_G_tilde_ti;
    shake256_init_vec4(&h_shake_G_tilde_ti);
    shake256_absorb_vec4(&h_shake_G_tilde_ti, bs_buf[i], bs_buf[i + 1], bs_buf[i + 2], bs_buf[i + 3], CEILING((MEDS_k * (MEDS_m * MEDS_n - MEDS_k)) * GFq_bits, 8));
    shake256_finalize_vec4(&h_shake_G_tilde_ti);
    shake256_squeeze_vec4(digest_G_tilde_ti[i], digest_G_tilde_ti[i + 1], digest_G_tilde_ti[i + 2], digest_G_tilde_ti[i + 3], MEDS_digest_bytes, &h_shake_G_tilde_ti);
  }
  // Hash the digests into a single digest
  for (int i = 0; i < MEDS_t; i++)
    shake256_absorb(&h_shake, digest_G_tilde_ti[i], MEDS_digest_bytes);
#else
  for (int i = 0; i < MEDS_t; i++)
    shake256_absorb(&h_shake, bs_buf[i], CEILING((MEDS_k * (MEDS_m * MEDS_n - MEDS_k)) * GFq_bits, 8));
#endif
  PROFILER_STOP("hash");

  shake256_absorb(&h_shake, (uint8_t *)m, mlen);

  shake256_finalize(&h_shake);

  uint8_t digest[MEDS_digest_bytes];

  shake256_squeeze(digest, MEDS_digest_bytes, &h_shake);

  LOG_VEC(digest, MEDS_digest_bytes);

  uint8_t h[MEDS_t];

  parse_hash(digest, MEDS_digest_bytes, h, MEDS_t);

  LOG_VEC(h, MEDS_t);

  bitstream_t bs;

  bs_init(&bs, sm, MEDS_w * CEILING(2 * MEDS_k * GFq_bits, 8));

  uint8_t *path = sm + MEDS_w * CEILING(2 * MEDS_k * GFq_bits, 8);

  t_hash(stree, rho, alpha, 0, 0);

  stree_to_path(stree, h, path, alpha);

  for (int i = 0; i < MEDS_t; i++)
  {
    if (h[i] > 0)
    {
      {
        pmod_mat_t kappa[2 * MEDS_k];

        pmod_mat_mul(kappa, 2, MEDS_k, M_tilde[i], 2, MEDS_k, T_inv[h[i]], MEDS_k, MEDS_k);

        LOG_MAT_FMT(kappa, 2, MEDS_k, "kappa[%i]", i);

        for (int j = 0; j < 2 * MEDS_k; j++)
          bs_write(&bs, kappa[j], GFq_bits);
      }

      bs_finalize(&bs);
    }
  }

  memcpy(sm + MEDS_SIG_BYTES - MEDS_digest_bytes - MEDS_st_salt_bytes, digest, MEDS_digest_bytes);
  memcpy(sm + MEDS_SIG_BYTES - MEDS_st_salt_bytes, alpha, MEDS_st_salt_bytes);
  memcpy(sm + MEDS_SIG_BYTES, m, mlen);

  *smlen = MEDS_SIG_BYTES + mlen;

  LOG_HEX(sm, MEDS_SIG_BYTES + mlen);

  return 0;
}

int crypto_sign_open_vec(
    unsigned char *m, unsigned long long *mlen,
    const unsigned char *sm, unsigned long long smlen,
    const unsigned char *pk)
{
  // If MEDS_t is not 8, this optimized version of MEDS cannot be used.
  if (MEDS_t < 8)
  {
    fprintf(stderr, "ERROR: This high-level optimized version of MEDS requires MEDS_t >= 8\n");
    return -1;
  }
  
  LOG_HEX(pk, MEDS_PK_BYTES);
  LOG_HEX(sm, smlen);

  pmod_mat_t G_data[MEDS_k * MEDS_m * MEDS_n * MEDS_s];
  pmod_mat_t *G[MEDS_s];

  for (int i = 0; i < MEDS_s; i++)
    G[i] = &G_data[i * MEDS_k * MEDS_m * MEDS_n];

  rnd_sys_mat(G[0], MEDS_k, MEDS_m * MEDS_n, pk, MEDS_pub_seed_bytes);

  {
    bitstream_t bs;

    bs_init(&bs, (uint8_t *)pk + MEDS_pub_seed_bytes, MEDS_PK_BYTES - MEDS_pub_seed_bytes);

    for (int i = 1; i < MEDS_s; i++)
    {
      for (int r = 0; r < MEDS_k; r++)
        for (int c = 0; c < MEDS_k; c++)
          if (r == c)
            pmod_mat_set_entry(G[i], MEDS_k, MEDS_m * MEDS_n, r, c, 1);
          else
            pmod_mat_set_entry(G[i], MEDS_k, MEDS_m * MEDS_n, r, c, 0);

      for (int r = 2; r < MEDS_k; r++)
        for (int j = MEDS_k; j < MEDS_m * MEDS_n; j++)
          G[i][r * MEDS_m * MEDS_n + j] = bs_read(&bs, GFq_bits);

      for (int ii = 0; ii < MEDS_m; ii++)
        for (int j = 0; j < MEDS_n; j++)
          G[i][ii * MEDS_n + j] = ii == j ? 1 : 0;

      for (int ii = 0; ii < MEDS_m; ii++)
        for (int j = 0; j < MEDS_n; j++)
          G[i][MEDS_m * MEDS_n + ii * MEDS_n + j] = (ii + 1) == j ? 1 : 0;

      bs_finalize(&bs);
    }
  }

  for (int i = 0; i < MEDS_s; i++)
    LOG_MAT_FMT(G[i], MEDS_k, MEDS_m * MEDS_n, "G[%i]", i);

  LOG_HEX_FMT(sm, MEDS_w * CEILING(2 * MEDS_k * GFq_bits, 8), "kappa");
  LOG_HEX_FMT(sm + MEDS_w * CEILING(2 * MEDS_k * GFq_bits, 8),
              MEDS_max_path_len * MEDS_st_seed_bytes, "path");

  uint8_t *digest = (uint8_t *)sm + (MEDS_SIG_BYTES - MEDS_digest_bytes - MEDS_st_salt_bytes);

  uint8_t *alpha = (uint8_t *)sm + (MEDS_SIG_BYTES - MEDS_st_salt_bytes);

  LOG_HEX(digest, MEDS_digest_bytes);
  LOG_HEX(alpha, MEDS_st_salt_bytes);

  uint8_t h[MEDS_t];

  parse_hash(digest, MEDS_digest_bytes, h, MEDS_t);

  bitstream_t bs;

  bs_init(&bs, (uint8_t *)sm, MEDS_w * CEILING(2 * MEDS_k * GFq_bits, 8));

  uint8_t *path = (uint8_t *)sm + MEDS_w * CEILING(2 * MEDS_k * GFq_bits, 8);

  uint8_t stree[MEDS_st_seed_bytes * SEED_TREE_size] = {0};

  path_to_stree(stree, h, path, alpha);

  // LOG_HEX(stree, MEDS_st_seed_bytes * SEED_TREE_size);

  uint8_t *sigma = &stree[MEDS_st_seed_bytes * SEED_TREE_ADDR(MEDS_seed_tree_height, 0)];

  // Initialize matrices that will be filled during the commitment phase
  // These matrices are still needed for the hash computation
  pmod_mat_t kappa_or_M_hat_i_data[MEDS_t << 1][2 * MEDS_k];
  pmod_mat_t *kappa_or_M_hat_i[MEDS_t << 1];
  static uint8_t bs_buf_data[MEDS_t << 1][CEILING((MEDS_k * (MEDS_m * MEDS_n - MEDS_k)) * GFq_bits, 8)];
  static uint8_t *bs_buf[MEDS_t << 1];

  for (int i = 0; i < MEDS_t << 1; i++)
  {
    kappa_or_M_hat_i[i] = kappa_or_M_hat_i_data[i];
    bs_buf[i] = bs_buf_data[i];
  }

  // Define temporary arrays that will be used during a single commitment computation
  pmod_mat_vec_t A_hat[MEDS_m * MEDS_m];
  pmod_mat_vec_t B_hat[MEDS_n * MEDS_n];
  pmod_mat_vec_t A_hat_inv[MEDS_m * MEDS_m];
  pmod_mat_vec_t B_hat_inv[MEDS_n * MEDS_n];
  pmod_mat_vec_t kappa_or_M_hat_i_vec[2 * MEDS_k];
  pmod_mat_vec_t G0_prime_or_C_hat[2 * MEDS_m * MEDS_n];
  static pmod_mat_vec_t G_vec[MEDS_k * MEDS_m * MEDS_n];
  uint8_t sigma_M_hat_i[MEDS_pub_seed_bytes];
  static pmod_mat_vec_t G_hat_i_vec[MEDS_k * MEDS_m * MEDS_n];

  // Initialize hash function
  keccak_state shake;
  shake256_init(&shake);

  // Create a seed buffer for use during the challenges. Set the start of the buffer to the salt.
  uint8_t seed_buf[MEDS_st_salt_bytes + MEDS_st_seed_bytes + sizeof(uint32_t)] = {0};
  memcpy(seed_buf, alpha, MEDS_st_salt_bytes);

  // Establish a pointer to the location of the address in the seed buffer
  uint8_t *addr_pos = seed_buf + MEDS_st_salt_bytes + MEDS_st_seed_bytes;

  // Loop variables
  int num_valid = 0;
  int num_invalid = 0;
  int num_tried = 0;
  int indexes[MEDS_t << 1];

  for (int i = 0; i < MEDS_t; i++)
    indexes[i] = i;
  for (int i = MEDS_t; i < (MEDS_t << 1); i++)
    indexes[i] = 0;

  PROFILER_START("SEC_COMMIT");
  while (num_valid < MEDS_t)
  {
    int index = num_tried;

    // Determine the batch size for the current iteration
    int loop_batch_size = MEDS_t + num_invalid - num_tried;
    if (loop_batch_size > BATCH_SIZE)
      loop_batch_size = BATCH_SIZE;

    for (int t = 0; t < loop_batch_size; t++)
    {
      // Load indices
      int index_netto = indexes[num_tried + t];

      if (h[index_netto] > 0)
      {
        for (int j = 0; j < 2 * MEDS_k; j++)
          kappa_or_M_hat_i[index + t][j] = bs_read(&bs, GFq_bits);

        bs_finalize(&bs);
      }
      else
      {
        for (int j = 0; j < 4; j++)
          addr_pos[j] = (index_netto >> (j * 8)) & 0xff;

        memcpy(seed_buf + MEDS_st_salt_bytes, &sigma[index_netto * MEDS_st_seed_bytes], MEDS_st_seed_bytes);

        XOF((uint8_t *[]){sigma_M_hat_i, &sigma[index_netto * MEDS_st_seed_bytes]},
            (size_t[]){MEDS_pub_seed_bytes, MEDS_st_seed_bytes},
            seed_buf, MEDS_st_salt_bytes + MEDS_st_seed_bytes + sizeof(uint32_t),
            2);

        rnd_matrix(kappa_or_M_hat_i[index + t], 2, MEDS_k, sigma_M_hat_i, MEDS_pub_seed_bytes);
      }

      PROFILER_START("init_G");
      // Load G_vec from G
      int G_index = h[index_netto];
      for (int i = 0; i < MEDS_k * MEDS_m * MEDS_n; i++)
        G_vec[i][t] = G[G_index][i];
      PROFILER_STOP("init_G");
    }

    // Load kappa_or_M_hat_i into kappa_or_M_hat_i_vec
    for (int r = 0; r < 2; r++)
      for (int c = 0; c < MEDS_k; c++)
        kappa_or_M_hat_i_vec[r * MEDS_k + c] = load_vec(kappa_or_M_hat_i + index, 2, MEDS_k, r, c);

    pmod_mat_s_vec_t valid = SET_S_VEC(1);

    pmod_mat_mul_vec(G0_prime_or_C_hat, 2, MEDS_m * MEDS_n, kappa_or_M_hat_i_vec, 2, MEDS_k, G_vec, MEDS_k, MEDS_m * MEDS_n);

    valid = AND_S_VEC(valid, TO_S_VEC(GEQ_S_VEC(solve_vec(A_hat, B_hat_inv, G0_prime_or_C_hat, 0), ZERO_S_VEC)));
    valid = AND_S_VEC(valid, TO_S_VEC(GEQ_S_VEC(pmod_mat_inv_vec(B_hat, B_hat_inv, MEDS_n, MEDS_n, 0), ZERO_S_VEC)));
    valid = AND_S_VEC(valid, TO_S_VEC(GEQ_S_VEC(pmod_mat_inv_vec(A_hat_inv, A_hat, MEDS_m, MEDS_m, 0), ZERO_S_VEC)));

    pi_vec(G_hat_i_vec, A_hat, B_hat, G_vec);

    valid = AND_S_VEC(valid, TO_S_VEC(EQ0_S_VEC(SF_vec(G_hat_i_vec, G_hat_i_vec, 0))));

    // Store G_hat_i_vec into G_hat_i[index]
    PROFILER_START("bs_fill");
    if (GFq_bits == 12)
    {
      // Use a 12-bit optimized parallel bitstream fill function (12 bits are used for all parameter sets)
      store_bitstream_12bit(bs_buf, index, G_hat_i_vec, loop_batch_size);
    }
    else
    {
      // In other cases, use a generic bitstream fill function
      for (int i = 0; i < loop_batch_size; i++)
      {
        bitstream_t bs;

        bs_init(&bs, bs_buf[index + i], CEILING((MEDS_k * (MEDS_m * MEDS_n - MEDS_k)) * GFq_bits, 8));

        for (int r = 0; r < MEDS_k; r++)
          for (int j = MEDS_k; j < MEDS_m * MEDS_n; j++)
            bs_write(&bs, G_hat_i_vec[r * MEDS_m * MEDS_n + j][i], GFq_bits);

        bs_finalize(&bs);
      }
    }
    PROFILER_STOP("bs_fill");

    int current_batch_invalids = 0;

    for (int t = 0; t < loop_batch_size; t++)
    {
      if (GET_LANE_S_VEC(valid, t) == VEC_FALSE)
      {
        // Compute indices
        int idx_source = num_tried + t;
        int idx_target = MEDS_t + num_invalid + current_batch_invalids;

        // If the commitment was invalid and the hash is a non-zero hash, return failure
        if (h[indexes[idx_source]] > 0)
        {
          printf("Signature verification failed!\n");
          return -1;
        }

        indexes[idx_target] = indexes[idx_source];
        bs_buf[idx_target] = bs_buf_data[idx_source];
        kappa_or_M_hat_i[idx_target] = kappa_or_M_hat_i_data[idx_source];

        // Update counters
        num_invalid++;
        current_batch_invalids++;
      }
      else
      {
        num_valid++;
      }
    }
    num_tried += loop_batch_size;
  }
  PROFILER_STOP("SEC_COMMIT");

  // Hash all commitments
  PROFILER_START("hash");
#ifdef MEDS_hash_opt
  // Use parallel hashing to compute 4 digests at once
  uint8_t digest_G_hat[MEDS_t][MEDS_digest_bytes];
  for (int i = 0; i < MEDS_t; i += 4)
  {
    keccak_state_vec4 shake_G_hat;
    shake256_init_vec4(&shake_G_hat);
    shake256_absorb_vec4(&shake_G_hat, bs_buf[i], bs_buf[i + 1], bs_buf[i + 2], bs_buf[i + 3], CEILING((MEDS_k * (MEDS_m * MEDS_n - MEDS_k)) * GFq_bits, 8));
    shake256_finalize_vec4(&shake_G_hat);
    shake256_squeeze_vec4(digest_G_hat[i], digest_G_hat[i + 1], digest_G_hat[i + 2], digest_G_hat[i + 3], MEDS_digest_bytes, &shake_G_hat);
  }
  // Hash the digests into a single digest
  for (int i = 0; i < MEDS_t; i++)
    shake256_absorb(&shake, digest_G_hat[i], MEDS_digest_bytes);
#else
  for (int i = 0; i < MEDS_t; i++)
    shake256_absorb(&shake, bs_buf[i], CEILING((MEDS_k * (MEDS_m * MEDS_n - MEDS_k)) * GFq_bits, 8));
#endif
  PROFILER_STOP("hash");

  shake256_absorb(&shake, (uint8_t *)(sm + MEDS_SIG_BYTES), smlen - MEDS_SIG_BYTES);

  shake256_finalize(&shake);

  uint8_t check[MEDS_digest_bytes];

  shake256_squeeze(check, MEDS_digest_bytes, &shake);

  if (memcmp(digest, check, MEDS_digest_bytes) != 0)
  {
    fprintf(stderr, "Signature verification failed!\n");

    return -1;
  }

  memcpy(m, (uint8_t *)(sm + MEDS_SIG_BYTES), smlen - MEDS_SIG_BYTES);
  *mlen = smlen - MEDS_SIG_BYTES;

  return 0;
}
