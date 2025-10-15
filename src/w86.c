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
    "mem[0]: %.2hhx\n",
    state->reg.ax,
    state->reg.bx,
    state->reg.cx,
    state->reg.dx,
    state->reg.si,
    state->reg.di,
    state->reg.sp,
    state->reg.bp,
    state->reg.cs,
    state->reg.ds,
    state->reg.es,
    state->reg.ss,
    state->reg.ip,
    state->reg.flags,
    state->mem[0]
  );
}
