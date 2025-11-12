// SPDX-License-Identifier: GPL-3.0-or-later

#include "instruction.h"

#include <stddef.h>
#include <stdint.h>

#include "address.h"
#include "decode.h"
#include "modrm.h"
#include "w86.h"

// welcome to switch statement hell...
// TODO: actually make this switch statement hell

static inline uint16_t get_segment(struct w86_cpu_state* state, enum w86_segment_prefix segment) {
  switch (segment) {
  case W86_SEGMENT_PREFIX_CS:
    return state->registers.cs;

  case W86_SEGMENT_PREFIX_NONE:
  case W86_SEGMENT_PREFIX_DS:
    return state->registers.ds;

  case W86_SEGMENT_PREFIX_ES:
    return state->registers.es;

  case W86_SEGMENT_PREFIX_SS:
    return state->registers.ss;
  }
}

enum w86_status w86_instruction_mov(struct w86_cpu_state* state, uint16_t offset, struct w86_instruction_prefixes prefixes) {
  uint8_t first_byte = w86_get_byte(state, state->registers.cs, offset);
  uint8_t segment = get_segment(state, prefixes.segment);
  struct w86_modrm_info info = {};

  switch (first_byte) {
  case 0x88: // reg8 -> r/m8
    info = w86_modrm_parse(state, offset + 1, prefixes.segment);
    w86_modrm_byte_store(state, info, nullptr);
    break;

  case 0x89: // reg16 -> r/m16
    info = w86_modrm_parse(state, offset + 1, prefixes.segment);
    w86_modrm_word_store(state, info, nullptr);
    break;

  case 0x8a: // r/m8 -> reg8
    info = w86_modrm_parse(state, offset + 1, prefixes.segment);
    w86_modrm_byte_load(state, info, nullptr);
    break;

  case 0x8b: // r/m16 -> reg16
    info = w86_modrm_parse(state, offset + 1, prefixes.segment);
    w86_modrm_word_load(state, info, nullptr);
    break;

  case 0x8c: // seg -> r/m16
    info = w86_modrm_parse(state, offset + 1, prefixes.segment);
    if (info.reg & 0b100) return W86_STATUS_INVALID_OPERATION;
    w86_modrm_segment_store(state, info, nullptr);
    break;
  
  case 0x8e: // r/m16 -> seg
    info = w86_modrm_parse(state, offset + 1, prefixes.segment);
    if (info.reg & 0b100 || info.reg == W86_MODRM_REG_CS) return W86_STATUS_INVALID_OPERATION;
    w86_modrm_segment_load(state, info, nullptr);
    break;

  case 0xa0: // mem8 -> al
    state->registers.ax &= 0xf0;
    state->registers.ax |= w86_get_byte(state, segment, w86_get_word(state, state->registers.cs, offset + 1));
    break;

  case 0xa1: // mem16 -> ax
    state->registers.ax = w86_get_word(state, segment, w86_get_word(state, state->registers.cs, offset + 1));
    break;

  case 0xa2: // al -> mem8
    w86_set_byte(state, segment, w86_get_word(state, state->registers.cs, offset + 1), state->registers.ax);
    break;

  case 0xa3: // ax -> mem16
    w86_set_word(state, segment, w86_get_word(state, state->registers.cs, offset + 1), state->registers.ax);
    break;

  case 0xb0 | W86_MODRM_REG_AL: // imm8 -> reg8
    state->registers.ax &= 0xf0;
    state->registers.ax |= w86_get_byte(state, state->registers.cs, offset + 1);
    break;

  case 0xb0 | W86_MODRM_REG_CL:
    state->registers.cx &= 0xf0;
    state->registers.cx |= w86_get_byte(state, state->registers.cs, offset + 1);
    break;

  case 0xb0 | W86_MODRM_REG_DL:
    state->registers.dx &= 0xf0;
    state->registers.dx |= w86_get_byte(state, state->registers.cs, offset + 1);
    break;
  
  case 0xb0 | W86_MODRM_REG_BL:
    state->registers.bx &= 0xf0;
    state->registers.bx |= w86_get_byte(state, state->registers.cs, offset + 1);
    break;
  
  case 0xb0 | W86_MODRM_REG_AH:
    state->registers.ax &= 0x0f;
    state->registers.ax |= w86_get_byte(state, state->registers.cs, offset + 1) << 8;
    break;

  case 0xb0 | W86_MODRM_REG_CH:
    state->registers.cx &= 0x0f;
    state->registers.cx |= w86_get_byte(state, state->registers.cs, offset + 1) << 8;
    break;

  case 0xb0 | W86_MODRM_REG_DH:
    state->registers.dx &= 0x0f;
    state->registers.dx |= w86_get_byte(state, state->registers.cs, offset + 1) << 8;
    break;

  case 0xb0 | W86_MODRM_REG_BH:
    state->registers.bx &= 0x0f;
    state->registers.bx |= w86_get_byte(state, state->registers.cs, offset + 1) << 8;
    break;
  
  case 0xb8 | W86_MODRM_REG_AX: // imm16 -> reg16
    state->registers.ax = w86_get_word(state, state->registers.cs, offset + 1);
    break;

  case 0xb8 | W86_MODRM_REG_CX:
    state->registers.cx = w86_get_word(state, state->registers.cs, offset + 1);
    break;

  case 0xb8 | W86_MODRM_REG_DX:
    state->registers.dx = w86_get_word(state, state->registers.cs, offset + 1);
    break;
  
  case 0xb8 | W86_MODRM_REG_BX:
    state->registers.bx = w86_get_word(state, state->registers.cs, offset + 1);
    break;
  
  case 0xb8 | W86_MODRM_REG_SP:
    state->registers.sp = w86_get_word(state, state->registers.cs, offset + 1);
    break;

  case 0xb8 | W86_MODRM_REG_BP:
    state->registers.bp = w86_get_word(state, state->registers.cs, offset + 1);
    break;

  case 0xb8 | W86_MODRM_REG_SI:
    state->registers.si = w86_get_word(state, state->registers.cs, offset + 1);
    break;

  case 0xb8 | W86_MODRM_REG_DI:
    state->registers.di = w86_get_word(state, state->registers.cs, offset + 1);
    break;

  case 0xc6:
  case 0xc7:
    return W86_STATUS_UNIMPLEMENTED_OPCODE;

  default:
    return W86_STATUS_INVALID_OPERATION;
  }

  if ((first_byte & 0b11111100) == 0xa0
   || (first_byte & 0b11111000) == 0xb8) {
    state->registers.ip = offset + 3;
  } else {
    state->registers.ip = offset + 2;
  }
  state->registers.ip += info.size;

  return W86_STATUS_SUCCESS;
}

enum w86_status w86_instruction_jmp(struct w86_cpu_state* state, uint16_t offset, struct w86_instruction_prefixes) {
  switch (w86_get_byte(state, state->registers.cs, offset)) {
  case 0xe9: // near jump
    state->registers.ip += (int16_t) w86_get_word(state, state->registers.cs, offset + 1);
    break;

  case 0xea: // far jump
    state->registers.ip = w86_get_word(state, state->registers.cs, offset + 1);
    state->registers.cs = w86_get_word(state, state->registers.cs, offset + 3);
    break;

  case 0xeb: // short jump
    state->registers.ip += (int8_t) w86_get_byte(state, state->registers.cs, offset + 1);
    break;

  case 0xff: // indirect jump
    return W86_STATUS_UNIMPLEMENTED_OPCODE;

  default:
    return W86_STATUS_INVALID_OPERATION;
  }

  return W86_STATUS_SUCCESS;
}
