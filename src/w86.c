// SPDX-License-Identifier: GPL-3.0-or-later

#include <emscripten.h>

int EMSCRIPTEN_KEEPALIVE multiply (int x, int y) {
  return x * y;
}
