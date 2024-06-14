#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <sys/random.h>
#include <sys/time.h>

#include "params.h"
#include "api.h"
#include "meds.h"
#include "meds_vec.h"
#include "profiler.h"

profileresult profileresults[1000];
int number_of_profileresults = 0;
int profiler_enabled = 0;

double osfreq(void);

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

  enable_cyclecounter();

  crypto_sign_keypair(pk, sk);
  crypto_sign(sig, &sig_len, (const unsigned char *)msg, sizeof(msg), sk);

  long long results[rounds];

  printf("Starting profiler for %i rounds...\n", rounds);
  PROFILER_ENABLE();
  for (int round = 0; round < rounds; round++)
  {
    if (rounds > 10 && round % (rounds / 10) == 0)
      printf("  %i%%\n", (round * 100) / rounds);

    switch (mode)
    {
    case 1:
    {
      long long keygen_time = -get_cyclecounter();
      crypto_sign_keypair(pk, sk);
      keygen_time += get_cyclecounter();
      results[round] = keygen_time;
      break;
    }
    case 2:
    {
      long long sign_time = -get_cyclecounter();
      crypto_sign_vec(sig, &sig_len, (const unsigned char *)msg, sizeof(msg), sk);
      sign_time += get_cyclecounter();
      results[round] = sign_time;
      break;
    }
    case 3:
    {
      unsigned char msg_out[17];
      unsigned long long msg_out_len = sizeof(msg_out);

      long long verify_time = -get_cyclecounter();
      int ret = crypto_sign_open(msg_out, &msg_out_len, sig, sizeof(sig), pk);
      
      verify_time += get_cyclecounter();
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
  PROFILER_DISABLE();

  double median_time = median(results, rounds);
  double median_time_s = median_time / osfreq();
  double median_time_ms = median_time_s * 1000;

  printf("Median time: %f cycles (%f s, %f ms)\n", median_time, median_time_s, median_time_ms);

  // If we executed only 1 round, print additional profiling results
  if (rounds == 1) {
    complete_profiler();
    
    printf("\nProfiler results (%i):\n", number_of_profileresults);

    for (int i = 0; i < number_of_profileresults; i++) {
      float percent = (float)profileresults[i].total_cycle_count / median_time * 100;
      long long median_cycle = median(profileresults[i].cycle_counts, profileresults[i].cycle_count_index);
      printf("  %s: %lli cycles (%.2f%%) - tot runs: %i - median of op: %lli\n", profileresults[i].name, profileresults[i].total_cycle_count, percent, profileresults[i].runs, median_cycle);
    }
  }

  disable_cyclecounter();
  return 0;
}
