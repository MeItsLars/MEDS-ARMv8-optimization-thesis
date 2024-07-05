#ifndef MEDS_VEC_H
#define MEDS_VEC_H

#include "api.h"
#include "params.h"

int crypto_sign_keypair_vec(unsigned char *pk, unsigned char *sk);

int crypto_sign_vec(unsigned char *sm, unsigned long long *smlen, const unsigned char *m, unsigned long long mlen, const unsigned char *sk);

int crypto_sign_open_vec(unsigned char *m, unsigned long long *mlen, const unsigned char *sm, unsigned long long smlen, const unsigned char *pk);

#endif

