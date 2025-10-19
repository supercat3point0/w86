// SPDX-License-Identifier: GPL-3.0-or-later

#include <emscripten/bind.h>

#define EMBIND
#include "w86.h"

using namespace emscripten;

EMSCRIPTEN_BINDINGS(w86) {
  value_object<w86_cpu_state::w86_register_file>("W86RegisterFile")
    .field("ax", &w86_cpu_state::w86_register_file::ax)
    .field("bx", &w86_cpu_state::w86_register_file::bx)
    .field("cx", &w86_cpu_state::w86_register_file::cx)
    .field("dx", &w86_cpu_state::w86_register_file::dx)
    .field("si", &w86_cpu_state::w86_register_file::si)
    .field("di", &w86_cpu_state::w86_register_file::di)
    .field("bp", &w86_cpu_state::w86_register_file::bp)
    .field("sp", &w86_cpu_state::w86_register_file::sp)
    .field("cs", &w86_cpu_state::w86_register_file::cs)
    .field("ds", &w86_cpu_state::w86_register_file::ds)
    .field("es", &w86_cpu_state::w86_register_file::es)
    .field("ss", &w86_cpu_state::w86_register_file::ss)
    .field("ip", &w86_cpu_state::w86_register_file::ip)
    .field("flags", &w86_cpu_state::w86_register_file::flags);

  class_<w86_cpu_state>("W86CpuState")
    .constructor<>()
    .property("registers", &w86_cpu_state::registers)
    .property("memory", &w86_cpu_state::memory);

  function("w86CpuStep", w86_cpu_step, allow_raw_pointers());
}
