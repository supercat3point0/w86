// SPDX-License-Identifier: GPL-3.0-or-later

#include "instruction.h"

#include <stddef.h>
#include <stdint.h>

#include "decode.h"
#include "w86.h"

enum w86_status w86_instruction_mov(struct w86_cpu_state* state, size_t offset, struct w86_instruction_prefixes) {
  switch (state->memory[offset]) {
  case 0xb0 + 0b000: // imm8 -> reg8
    state->registers.ax |= W86_GET_BYTE(state->memory, offset + 1);
    break;

  case 0xb0 + 0b001:
    state->registers.cx |= W86_GET_BYTE(state->memory, offset + 1);
    break;

  case 0xb0 + 0b010:
    state->registers.dx |= W86_GET_BYTE(state->memory, offset + 1);
    break;
  
  case 0xb0 + 0b011:
    state->registers.bx |= W86_GET_BYTE(state->memory, offset + 1);
    break;
  
  case 0xb0 + 0b100:
    state->registers.ax |= W86_GET_BYTE(state->memory, offset + 1) << 8;
    break;

  case 0xb0 + 0b101:
    state->registers.cx |= W86_GET_BYTE(state->memory, offset + 1) << 8;
    break;

  case 0xb0 + 0b110:
    state->registers.dx |= W86_GET_BYTE(state->memory, offset + 1) << 8;
    break;

  case 0xb0 + 0b111:
    state->registers.bx |= W86_GET_BYTE(state->memory, offset + 1) << 8;
    break;
  
  case 0xb8 + 0b000: // imm16 -> reg16
    state->registers.ax = W86_GET_WORD(state->memory, offset + 1);
    break;

  case 0xb8 + 0b001:
    state->registers.cx = W86_GET_WORD(state->memory, offset + 1);
    break;

  case 0xb8 + 0b010:
    state->registers.dx = W86_GET_WORD(state->memory, offset + 1);
    break;
  
  case 0xb8 + 0b011:
    state->registers.bx = W86_GET_WORD(state->memory, offset + 1);
    break;
  
  case 0xb8 + 0b100:
    state->registers.sp = W86_GET_WORD(state->memory, offset + 1);
    break;

  case 0xb8 + 0b101:
    state->registers.bp = W86_GET_WORD(state->memory, offset + 1);
    break;

  case 0xb8 + 0b110:
    state->registers.si = W86_GET_WORD(state->memory, offset + 1);
    break;

  case 0xb8 + 0b111:
    state->registers.di = W86_GET_WORD(state->memory, offset + 1);
    break;

  case 0x88:
  case 0x89:
  case 0x8a:
  case 0x8b:
  case 0x8c:
  case 0x8e:
  case 0xa0:
  case 0xa1:
  case 0xa2:
  case 0xa3:
  case 0xc6:
  case 0xc7:
    return W86_STATUS_UNIMPLEMENTED_OPCODE;

  default:
    return W86_STATUS_INVALID_OPCODE;
  }

  if ((state->memory[offset] & 0b11111000) == 0xb0) state->registers.ip = W86_BOUND_ADDRESS(offset + 2);
  else if ((state->memory[offset] & 0b11111000) == 0xb8) state->registers.ip = W86_BOUND_ADDRESS(offset + 3);

  return W86_STATUS_SUCCESS;
}

enum w86_status w86_instruction_jmp(struct w86_cpu_state* state, size_t offset, struct w86_instruction_prefixes) {
  switch (state->memory[offset]) {
  case 0xe9: // near jump
    state->registers.ip += (int16_t) W86_GET_WORD(state->memory, offset + 1);
    break;

  case 0xea: // far jump
    state->registers.ip = W86_GET_WORD(state->memory, offset + 1);
    state->registers.cs = W86_GET_WORD(state->memory, offset + 3);
    break;

  case 0xeb: // short jump
    state->registers.ip += (int8_t) W86_GET_BYTE(state->memory, offset + 1);
    break;

  case 0xff: // indirect jump
    return W86_STATUS_UNIMPLEMENTED_OPCODE;

  default:
    return W86_STATUS_INVALID_OPCODE;
  }

  return W86_STATUS_SUCCESS;
}
