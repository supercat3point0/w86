// SPDX-License-Identifier: GPL-3.0-or-later

#include "w86.h"

#include <stdio.h>

#include "decode.h"

enum w86_status w86_cpu_step(struct w86_cpu_state* state) {
  enum w86_status status = w86_decode(state);

  printf(
    "ax: 0x%.4hx\n"
    "bx: 0x%.4hx\n"
    "cx: 0x%.4hx\n"
    "dx: 0x%.4hx\n"
    "si: 0x%.4hx\n"
    "di: 0x%.4hx\n"
    "sp: 0x%.4hx\n"
    "bp: 0x%.4hx\n"
    "cs: 0x%.4hx\n"
    "ds: 0x%.4hx\n"
    "es: 0x%.4hx\n"
    "ss: 0x%.4hx\n"
    "ip: 0x%.4hx\n"
    "flags: 0x%.4hx\n"
    "memory[0x%.5x]: 0x%.2hhx\n",
    state->registers.ax,
    state->registers.bx,
    state->registers.cx,
    state->registers.dx,
    state->registers.si,
    state->registers.di,
    state->registers.sp,
    state->registers.bp,
    state->registers.cs,
    state->registers.ds,
    state->registers.es,
    state->registers.ss,
    state->registers.ip,
    state->registers.flags,
    W86_REAL_ADDRESS(state->registers.cs, state->registers.ip),
    state->memory[W86_REAL_ADDRESS(state->registers.cs, state->registers.ip)]
  );

  return status;
}
