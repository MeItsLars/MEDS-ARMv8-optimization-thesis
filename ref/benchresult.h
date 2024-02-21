#ifndef BENCHRESULT_H
#define BENCHRESULT_H

static long long cpucycles(void)
{
    unsigned long long result;
    asm volatile(".byte 15;.byte 49;shlq $32,%%rdx;orq %%rdx,%%rax"
                 : "=a"(result)::"%rdx");
    return result;
}

typedef struct
{
    char name[256];
    long long total_cycle_count;
    long long start_cycle_count;
    long long cycle_counts[100];
    int cycle_count_index;
    int runs;
} benchresult;

extern benchresult benchresults[1000];
extern int number_of_benchresults;
extern int benchmark_enabled;

static int compare_benchresults(const void *a, const void *b)
{
    return ((benchresult *)b)->total_cycle_count - ((benchresult *)a)->total_cycle_count;
}

static void complete_benchmark()
{
    qsort(benchresults, number_of_benchresults, sizeof(benchresult), compare_benchresults);
}

#if BENCHMARK

#include <stdio.h>
#include <string.h>

#define BENCH_ENABLE()         \
    do                         \
    {                          \
        benchmark_enabled = 1; \
    } while (0)

#define BENCH_DISABLE()        \
    do                         \
    {                          \
        benchmark_enabled = 0; \
    } while (0)

#define BENCH_START(bench_name)                                                     \
    do                                                                              \
    {                                                                               \
        if (benchmark_enabled == 0)                                                 \
            break;                                                                  \
        int found = 0;                                                              \
        for (int i = 0; i < number_of_benchresults; i++)                            \
        {                                                                           \
            if (strcmp(benchresults[i].name, bench_name) == 0)                      \
            {                                                                       \
                found = 1;                                                          \
                benchresults[i].start_cycle_count = cpucycles();                    \
                break;                                                              \
            }                                                                       \
        }                                                                           \
        if (!found)                                                                 \
        {                                                                           \
            strcpy(benchresults[number_of_benchresults].name, bench_name);          \
            benchresults[number_of_benchresults].total_cycle_count = 0;             \
            benchresults[number_of_benchresults].start_cycle_count = cpucycles();   \
            number_of_benchresults++;                                               \
            if (number_of_benchresults > 999)                                       \
            {                                                                       \
                fprintf(stderr, "Benchmarks exceeded maximum number of results\n"); \
                exit(-1);                                                           \
            }                                                                       \
        }                                                                           \
    } while (0)

#define BENCH_END(bench_name)                                                                      \
    do                                                                                             \
    {                                                                                              \
        if (benchmark_enabled == 0)                                                                \
            break;                                                                                 \
        long long end_cycle_count = cpucycles();                                                   \
        for (int i = 0; i < number_of_benchresults; i++)                                           \
        {                                                                                          \
            if (strcmp(benchresults[i].name, bench_name) == 0)                                     \
            {                                                                                      \
                long long cycle_count = end_cycle_count - benchresults[i].start_cycle_count;       \
                benchresults[i].total_cycle_count += cycle_count;                                  \
                benchresults[i].start_cycle_count = 0;                                             \
                if (benchresults[i].cycle_count_index < 100)                                       \
                {                                                                                  \
                    benchresults[i].cycle_counts[benchresults[i].cycle_count_index] = cycle_count; \
                    benchresults[i].cycle_count_index++;                                           \
                }                                                                                  \
                benchresults[i].runs++;                                                            \
                break;                                                                             \
            }                                                                                      \
        }                                                                                          \
    } while (0)

#else

#define BENCH_ENABLE() \
    do                 \
    {                  \
    } while (0)
#define BENCH_DISABLE() \
    do                  \
    {                   \
    } while (0)
#define BENCH_START(bench_name) \
    do                          \
    {                           \
    } while (0)
#define BENCH_END(bench_name) \
    do                        \
    {                         \
    } while (0)

#endif

#endif
