// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef W86_W86_H
#define W86_W86_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

struct w86_cpu_state {
  struct w86_register_file {
    uint16_t ax;
    uint16_t bx;
    uint16_t cx;
    uint16_t dx;
    uint16_t si;
    uint16_t di;
    uint16_t sp;
    uint16_t bp;
    uint16_t cs;
    uint16_t ds;
    uint16_t es;
    uint16_t ss;
    uint16_t ip;
    uint16_t flags;
  } registers;
#ifdef EMBIND // embind doesn't support pointers to primitive types, so we have cheat a little
  intptr_t memory;
#else
  uint8_t* memory;
#endif
};

void w86_cpu_step(struct w86_cpu_state*);

#ifdef __cplusplus
}
#endif

#endif /* W86_W86_H */
