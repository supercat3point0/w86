// SPDX-License-Identifier: GPL-3.0-or-later

#include <stdio.h>

#include "w86.h"

void w86_cpu_step(struct w86_cpu_state* state) {
  printf(
    "ax: %.4hx\n"
    "bx: %.4hx\n"
    "cx: %.4hx\n"
    "dx: %.4hx\n"
    "si: %.4hx\n"
    "di: %.4hx\n"
    "sp: %.4hx\n"
    "bp: %.4hx\n"
    "cs: %.4hx\n"
    "ds: %.4hx\n"
    "es: %.4hx\n"
    "ss: %.4hx\n"
    "ip: %.4hx\n"
    "flags: %.4hx\n"
    "memory[0x00400]: %.2hhx\n",
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
    state->memory[0x00400]
  );
}
