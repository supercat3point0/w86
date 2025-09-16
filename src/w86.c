// SPDX-License-Identifier: GPL-3.0-or-later

#include "w86.h"

struct multiplication* multiply (struct multiplication* m) {
  m->z = m->x * m->y;
  return m;
}
