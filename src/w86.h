// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef W86_W86_H
#define W86_W86_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

struct w86_cpu_state {
  struct w86_reg_file {
    int16_t ax;
    int16_t bx;
    int16_t cx;
    int16_t dx;
    int16_t si;
    int16_t di;
    int16_t sp;
    int16_t bp;
    int16_t cs;
    int16_t ds;
    int16_t es;
    int16_t ss;
    int16_t ip;
    int16_t flags;
  } reg;
#ifdef EMBIND // embind doesn't support pointers to primitive types, so we have cheat a little
  intptr_t mem;
#else
  uint8_t* mem;
#endif
};

void w86_cpu_step(struct w86_cpu_state*);

#ifdef __cplusplus
}
#endif

#endif /* W86_W86_H */
