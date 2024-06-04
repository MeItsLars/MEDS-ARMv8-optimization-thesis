#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "randombytes.h"

#include "cyclecounter.h"
#include "params.h"
#include "api.h"
#include "meds.h"
#include "profiler.h"
#include "poison.h"

profileresult profileresults[1000];
int number_of_profileresults = 0;
int profiler_enabled = 0;

#define WARMUP 16
#define ROUNDS 128

double osfreq(void);

int main(int argc, char *argv[])
{
  printf("Benchmarking MEDS\n");
  printf("- Parameter set: %s\n", MEDS_name);
  printf("- Warmup rounds: %d\n", WARMUP);
  printf("- Measurement rounds: %d\n", ROUNDS);
  printf("\n");

  int mode = 0;
  if (argc > 1)
  {
    if (strcmp(argv[1], "keygen") == 0)
      mode = 1;
    else if (strcmp(argv[1], "sign") == 0)
      mode = 2;
    else if (strcmp(argv[1], "verify") == 0)
      mode = 3;
    else
    {
      fprintf(stderr, "Usage: %s [keygen|sign|verify]\n", argv[0]);
      exit(-1);
    }
  }

  char msg[16] = "TestTestTestTest";
  uint8_t sk[CRYPTO_SECRETKEYBYTES] = {0};
  uint8_t pk[CRYPTO_PUBLICKEYBYTES] = {0};
  uint8_t sig[CRYPTO_BYTES + sizeof(msg)] = {0};
  unsigned long long sig_len = sizeof(sig);

  long long keygen_cycles[ROUNDS + 1];
  long long sign_cycles[ROUNDS + 1];
  long long verify_cycles[ROUNDS + 1];

  enable_cyclecounter();

  printf("> Generating sk, pk, and a signature\n");
  crypto_sign_keypair(pk, sk);
  crypto_sign(sig, &sig_len, (const unsigned char *)msg, sizeof(msg), sk);

  if (mode == 0 || mode == 1)
  {
    printf("> Benchmarking keygen\n");
    printf("  Warmup\n");
    for (int i = 0; i < WARMUP; i++)
      crypto_sign_keypair(pk, sk);

    printf("  Measuring\n");
    for (int i = 0; i < ROUNDS; i++)
    {
      keygen_cycles[i] = get_cyclecounter();
      crypto_sign_keypair(pk, sk);
    }
    keygen_cycles[ROUNDS] = get_cyclecounter();
  }

  if (mode == 0 || mode == 2)
  {
    printf("> Benchmarking sign\n");
    printf("  Warmup\n");
    for (int i = 0; i < WARMUP; i++)
      crypto_sign(sig, &sig_len, (const unsigned char *)msg, sizeof(msg), sk);

    printf("  Measuring\n");
    for (int i = 0; i < ROUNDS; i++)
    {
      sign_cycles[i] = get_cyclecounter();
      crypto_sign(sig, &sig_len, (const unsigned char *)msg, sizeof(msg), sk);
    }
    sign_cycles[ROUNDS] = get_cyclecounter();
  }

  if (mode == 0 || mode == 3)
  {
    printf("> Benchmarking verify\n");
    printf("  Warmup\n");
    for (int i = 0; i < WARMUP; i++)
    {
      unsigned long long sig_len_copy = sig_len;
      crypto_sign_open((unsigned char *)msg, (unsigned long long *)&sig_len_copy, sig, sig_len, pk);
    }

    printf("  Measuring\n");
    for (int i = 0; i < ROUNDS; i++)
    {
      verify_cycles[i] = get_cyclecounter();
      unsigned long long sig_len_copy = sig_len;
      crypto_sign_open((unsigned char *)msg, (unsigned long long *)&sig_len_copy, sig, sig_len, pk);
    }
    verify_cycles[ROUNDS] = get_cyclecounter();
  }

  printf("\nResults:\n");
  if (mode == 0 || mode == 1)
  {
    long cycles = median_2(keygen_cycles, ROUNDS + 1, 0);
    printf("> Keygen:\n");
    printf("  Median cycles: %ld\n", cycles);
    printf("  Time:          %.2f s\n", cycles / osfreq());
  }

  if (mode == 0 || mode == 2)
  {
    long cycles = median_2(sign_cycles, ROUNDS + 1, 0);
    printf("> Sign:\n");
    printf("  Median cycles: %ld\n", cycles);
    printf("  Time:          %.2f s\n", cycles / osfreq());
  }

  if (mode == 0 || mode == 3)
  {
    long cycles = median_2(verify_cycles, ROUNDS + 1, 0);
    printf("> Verify:\n");
    printf("  Median cycles: %ld\n", cycles);
    printf("  Time:          %.2f s\n", cycles / osfreq());
  }

  disable_cyclecounter();
  return 0;
}
