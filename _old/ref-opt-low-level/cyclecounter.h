#ifndef CYCLECOUNTER_H
#define CYCLECOUNTER_H

#include <stdint.h>

void enable_cyclecounter();

void disable_cyclecounter();

uint64_t get_cyclecounter();

#endif