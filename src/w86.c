// SPDX-License-Identifier: GPL-3.0-or-later

#include "w86.h"

#include <stdio.h>

#include "decode.h"

enum w86_status w86_cpu_step(struct w86_cpu_state* state) {
  enum w86_status status = w86_decode(state);

  return status;
}
