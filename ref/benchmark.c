#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <sys/random.h>
#include <sys/time.h>

#include "params.h"
#include "api.h"
#include "meds.h"
#include "benchresult.h"

benchresult benchresults[1000];
int number_of_benchresults = 0;
int benchmark_enabled = 0;

double osfreq(void);

long long cpucycles(void);

int compare(const void *a, const void *b)
{
  return (*(long long *)a - *(long long *)b);
}

double median(long long arr[], int n)
{
  qsort(arr, n, sizeof(long long), compare);

  if (n % 2 != 0)
  {
    return arr[n / 2];
  }
  else
  {
    return (arr[n / 2 - 1] + arr[n / 2]) / 2.0;
  }
}

int main(int argc, char *argv[])
{
  if (argc < 2)
  {
    fprintf(stderr, "Usage: %s <keygen|sign|verify> [rounds]\n", argv[0]);
    exit(-1);
  }

  int mode = 0;
  if (strcmp(argv[1], "keygen") == 0)
    mode = 1;
  else if (strcmp(argv[1], "sign") == 0)
    mode = 2;
  else if (strcmp(argv[1], "verify") == 0)
    mode = 3;
  else
  {
    fprintf(stderr, "Usage: %s <keygen|sign|verify> [rounds]\n", argv[0]);
    exit(-1);
  }

  int rounds = 1;
  if (argc > 2)
    rounds = atoi(argv[2]);  

  char msg[16] = "TestTestTestTest";

  uint8_t sk[CRYPTO_SECRETKEYBYTES] = {0};
  uint8_t pk[CRYPTO_PUBLICKEYBYTES] = {0};

  uint8_t sig[CRYPTO_BYTES + sizeof(msg)] = {0};
  unsigned long long sig_len = sizeof(sig);

  printf("Algorithm: %s\n", MEDS_name);
  printf("m:    %i\n", MEDS_m);
  printf("n:    %i\n", MEDS_n);
  printf("q:    %i\n", MEDS_p);
  printf("k:    %i\n", MEDS_k);
  printf("s:    %i\n", MEDS_s);
  printf("t:    %i\n", MEDS_t);
  printf("w:    %i\n", MEDS_w);
  printf("pk:   %i bytes\n", MEDS_PK_BYTES);
  printf("sig:  %i bytes\n", MEDS_SIG_BYTES);

  crypto_sign_keypair(pk, sk);
  crypto_sign(sig, &sig_len, (const unsigned char *)msg, sizeof(msg), sk);

  long long results[rounds];

  printf("Starting benchmark for %i rounds...\n", rounds);
  BENCH_ENABLE();
  for (int round = 0; round < rounds; round++)
  {
    if (rounds > 10 && round % (rounds / 10) == 0)
      printf("  %i%%\n", (round * 100) / rounds);

    switch (mode)
    {
    case 1:
    {
      long long keygen_time = -cpucycles();
      crypto_sign_keypair(pk, sk);
      keygen_time += cpucycles();
      results[round] = keygen_time;
      break;
    }
    case 2:
    {
      long long sign_time = -cpucycles();
      crypto_sign(sig, &sig_len, (const unsigned char *)msg, sizeof(msg), sk);
      sign_time += cpucycles();
      results[round] = sign_time;
      break;
    }
    case 3:
    {
      unsigned char msg_out[17];
      unsigned long long msg_out_len = sizeof(msg_out);

      long long verify_time = -cpucycles();
      int ret = crypto_sign_open(msg_out, &msg_out_len, sig, sizeof(sig), pk);
      
      verify_time += cpucycles();
      results[round] = verify_time;

      if (ret != 0)
      {
        fprintf(stderr, "\n\n   ERROR! Signature did not verify!\n\n\n");
        exit(-1);
      }
      break;
    }
    }
  }
  BENCH_DISABLE();

  double median_time = median(results, rounds);
  double median_time_s = median_time / osfreq();
  double median_time_ms = median_time_s * 1000;

  printf("Median time: %f cycles (%f s, %f ms)\n", median_time, median_time_s, median_time_ms);

  // If we executed only 1 round, print additional benchmarking results
  if (rounds == 1) {
    complete_benchmark();
    
    printf("\nBenchmark results (%i):\n", number_of_benchresults);

    for (int i = 0; i < number_of_benchresults; i++) {
      float percent = (float)benchresults[i].total_cycle_count / median_time * 100;
      printf("  %s: %lli cycles (%.2f%%)\n", benchresults[i].name, benchresults[i].total_cycle_count, percent);
    }
  }

  return 0;
}
