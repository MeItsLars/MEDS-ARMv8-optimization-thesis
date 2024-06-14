#ifndef MEDS_H
#define MEDS_H

#include "api.h"
#include "params.h"

#define BUF_SET_12BIT_PAIR(buf, idx, v0, v1)     \
    buf[idx++] = (v0 & 0xff);                   \
    buf[idx++] = (v0 >> 8) | ((v1 & 0xf) << 4); \
    buf[idx++] = (v1 >> 4);

#endif
