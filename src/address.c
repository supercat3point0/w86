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
  state->memory[W86_REAL_ADDRESS(segment, pointer + 1)] = value >> 8;
}

uint8_t w86_in_byte(struct w86_cpu_state* state, uint16_t port) {
  return state->io.reads[W86_BOUND_IO_PORT(port)];
}

void w86_out_byte(struct w86_cpu_state* state, uint16_t port, uint8_t value) {
  state->io.writes[W86_BOUND_IO_PORT(port)] = value;
}

uint16_t w86_in_word(struct w86_cpu_state* state, uint16_t port) {
  return state->io.reads[W86_BOUND_IO_PORT(port)]
       | state->io.reads[W86_BOUND_IO_PORT(port + 1)] << 8;
}

void w86_out_word(struct w86_cpu_state* state, uint16_t port, uint16_t value) {
  state->io.writes[W86_BOUND_IO_PORT(port)] = value;
  state->io.writes[W86_BOUND_IO_PORT(port + 1)] = value >> 8;
}
