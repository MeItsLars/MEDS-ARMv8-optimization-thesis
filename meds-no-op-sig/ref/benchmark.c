#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "params.h"
#include "api.h"
#include "meds.h"

int main(int argc, char *argv[])
{
  printf("name: %s\n", MEDS_name);

  int rounds = 128;

  if (argc > 1)
    rounds = atoi(argv[1]);

  char msg[16] = "TestTestTestTest";

  uint8_t sk[CRYPTO_SECRETKEYBYTES] = {0};
  uint8_t pk[CRYPTO_PUBLICKEYBYTES] = {0};

  uint8_t sig[CRYPTO_BYTES + sizeof(msg)] = {0};
  unsigned long long sig_len = sizeof(sig);


  for (int round = 0; round < rounds; round++)
  {
    crypto_sign_keypair(pk, sk);

    crypto_sign(sig, &sig_len, (const unsigned char *)msg, sizeof(msg), sk);

    unsigned char msg_out[17];
    unsigned long long msg_out_len = sizeof(msg_out);

    int ret = crypto_sign_open(msg_out, &msg_out_len, sig, sizeof(sig), pk);

    if (ret != 0)
    {
      fprintf(stderr, "\n\n   ERROR! Signature did not verify!\n\n\n");

      exit(-1);
    }
  }
  printf("Done.\n");

  return 0;
}

