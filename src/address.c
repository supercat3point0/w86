// SPDX-License-Identifier: GPL-3.0-or-later

#include "address.h"

#include <stdint.h>

#include "w86.h"

uint8_t w86_get_byte(struct w86_cpu_state* state, uint16_t segment, uint16_t pointer) {
  return state->memory[W86_REAL_ADDRESS(segment, pointer)];
}

void w86_set_byte(struct w86_cpu_state* state, uint16_t segment, uint16_t pointer, uint8_t value) {
  state->memory[W86_REAL_ADDRESS(segment, pointer)] = value;
}

uint16_t w86_get_word(struct w86_cpu_state* state, uint16_t segment, uint16_t pointer) {
  return state->memory[W86_REAL_ADDRESS(segment, pointer)]
       | state->memory[W86_REAL_ADDRESS(segment, pointer + 1)] << 8;
}

void w86_set_word(struct w86_cpu_state* state, uint16_t segment, uint16_t pointer, uint16_t value) {
  state->memory[W86_REAL_ADDRESS(segment, pointer)] = value;
  state->memory[W86_REAL_ADDRESS(segment, pointer)] = value >> 8;
}
