#ifndef API_H
#define API_H

#define CRYPTO_SECRETKEYBYTES 19054
#define CRYPTO_PUBLICKEYBYTES 173282
#define CRYPTO_BYTES 7526

#define CRYPTO_ALGNAME "oldlevel5167717"

int crypto_sign_keypair(
    unsigned char *pk,
    unsigned char *sk
  );

int crypto_sign(
    unsigned char *sm, unsigned long long *smlen,
    const unsigned char *m, unsigned long long mlen,
    const unsigned char *sk
  );

int crypto_sign_open(
    unsigned char *m, unsigned long long *mlen,
    const unsigned char *sm, unsigned long long smlen,
    const unsigned char *pk
  );

#endif

