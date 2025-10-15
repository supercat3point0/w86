// SPDX-License-Identifier: GPL-3.0-or-later

#include <emscripten/bind.h>

#define EMBIND
#include "w86.h"

using namespace emscripten;

EMSCRIPTEN_BINDINGS(w86) {
  value_object<w86_cpu_state::w86_reg_file>("W86RegFile")
    .field("ax", &w86_cpu_state::w86_reg_file::ax)
    .field("bx", &w86_cpu_state::w86_reg_file::bx)
    .field("cx", &w86_cpu_state::w86_reg_file::cx)
    .field("dx", &w86_cpu_state::w86_reg_file::dx)
    .field("si", &w86_cpu_state::w86_reg_file::si)
    .field("di", &w86_cpu_state::w86_reg_file::di)
    .field("bp", &w86_cpu_state::w86_reg_file::bp)
    .field("sp", &w86_cpu_state::w86_reg_file::sp)
    .field("cs", &w86_cpu_state::w86_reg_file::cs)
    .field("ds", &w86_cpu_state::w86_reg_file::ds)
    .field("es", &w86_cpu_state::w86_reg_file::es)
    .field("ss", &w86_cpu_state::w86_reg_file::ss)
    .field("ip", &w86_cpu_state::w86_reg_file::ip)
    .field("flags", &w86_cpu_state::w86_reg_file::flags);

  class_<w86_cpu_state>("W86CPUState")
    .constructor<>()
    .property("reg", &w86_cpu_state::reg)
    .property("mem", &w86_cpu_state::mem);

  function("w86CPUStep", w86_cpu_step, allow_raw_pointers());
}
