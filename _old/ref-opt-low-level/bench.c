#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include <sys/random.h>
#include <sys/time.h>

#include "params.h"
#include "api.h"
#include "meds.h"

double osfreq(void);

int main(int argc, char *argv[])
{
  enable_cyclecounter();
  printf("name: %s\n", MEDS_name);

  int64_t keygen_time = 0;
  int64_t sign_time = 0;
  int64_t verify_time = 0;

  int rounds = 128;
  //int rounds = 8;

  if (argc > 1)
    rounds = atoi(argv[1]);

  char msg[16] = "TestTestTestTest";

  uint8_t sk[CRYPTO_SECRETKEYBYTES] = {0};
  uint8_t pk[CRYPTO_PUBLICKEYBYTES] = {0};

  uint8_t sig[CRYPTO_BYTES + sizeof(msg)] = {0};
  unsigned long long sig_len = sizeof(sig);


  printf("m:    %i\n", MEDS_m);
  printf("n:    %i\n", MEDS_n);
  printf("q:    %i\n", MEDS_p);
  printf("k:    %i\n", MEDS_k);
  printf("s:    %i\n", MEDS_s);
  printf("t:    %i\n", MEDS_t);
  printf("w:    %i\n", MEDS_w);

  printf("pk:   %i bytes\n", MEDS_PK_BYTES);

  printf("sig:  %i bytes\n", MEDS_SIG_BYTES);


  for (int round = 0; round < rounds; round++)
  {
    keygen_time = -get_cyclecounter();
    crypto_sign_keypair(pk, sk);
    keygen_time += get_cyclecounter();

    sign_time = -get_cyclecounter();
    crypto_sign(sig, &sig_len, (const unsigned char *)msg, sizeof(msg), sk);
    sign_time += get_cyclecounter();

    unsigned char msg_out[17];
    unsigned long long msg_out_len = sizeof(msg_out);

    verify_time = -get_cyclecounter();
    int ret = crypto_sign_open(msg_out, &msg_out_len, sig, sizeof(sig), pk);
    verify_time += get_cyclecounter();

    if (ret != 0)
    {
      fprintf(stderr, "\n\n   ERROR! Signature did not verify!\n\n\n");

      exit(-1);
    }

    double freq = osfreq() / 1000;
    printf("F: %f\n", freq);

    printf("%f (%llu cycles)  ", keygen_time / freq, keygen_time);
    printf("%f (%llu cycles)  ", sign_time / freq, sign_time);
    printf("%f (%llu cycles)  \n", verify_time / freq, verify_time);
  }

  disable_cyclecounter();
  return 0;
}

