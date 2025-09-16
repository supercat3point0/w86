// SPDX-License-Identifier: GPL-3.0-or-later

#include <emscripten/bind.h>

#include "w86.h"

using namespace emscripten;

EMSCRIPTEN_BINDINGS(w86) {
  class_<multiplication>("Multiplication")
    .constructor<>()
    .property("x", &multiplication::x)
    .property("y", &multiplication::y)
    .property("z", &multiplication::z);
  function("multiply", multiply, allow_raw_pointers());
}
