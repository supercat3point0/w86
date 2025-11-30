// SPDX-License-Identifier: GPL-3.0-or-later

#include "instruction.h"

#include <stddef.h>
#include <stdint.h>

#include "address.h"
#include "decode.h"
#include "modrm.h"
#include "w86.h"

// welcome to switch statement hell...

static inline uint16_t sbw(uint8_t value) {
  return (int8_t) value;
}

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

static uint16_t get_flags_byte(uint8_t value) {
  uint16_t flags = (value == 0) << 6 | (value < 0) << 7;

  // parity
  uint8_t parity = value;
  parity ^= parity >> 4;
  parity ^= parity >> 2;
  parity ^= parity >> 1;
  flags |= !(parity & 1) << 2;

  return flags;
}

static uint16_t get_flags_add_byte(uint8_t a, uint8_t b, uint8_t* ret) {
  uint8_t value = a + b;
  if (ret) *ret = value;
  return (a > 0xff - b)
       | ((a & 0b1111) > 0xf - (b & 0b1111)) << 4
       | (((int8_t) b > 0 && (int8_t) a > 0x7f - (int8_t) b) || ((int8_t) b < 0 && (int8_t) a < -0x7f - 1 - (int8_t) b)) << 11
       | get_flags_byte(value);
}

static uint16_t get_flags_sub_byte(uint8_t a, uint8_t b, uint8_t* ret) {
  uint8_t value = a - b;
  if (ret) *ret = value;
  return (a < b)
       | ((a & 0b1111) < (b & 0b1111)) << 4
       | (((int8_t) b < 0 && (int8_t) a > 0x7f + (int8_t) b) || ((int8_t) b > 0 && (int8_t) a < -0x7f - 1 + (int8_t) b)) << 11
       | get_flags_byte(value);
}

static uint16_t get_flags_word(uint16_t value) {
  uint16_t flags = (value == 0) << 6 | (value < 0) << 7;

  // parity
  uint8_t parity = value;
  parity ^= parity >> 4;
  parity ^= parity >> 2;
  parity ^= parity >> 1;
  flags |= !(parity & 1) << 2;

  return flags;
}

static uint16_t get_flags_add_word(uint16_t a, uint16_t b, uint16_t* ret) {
  uint16_t value = a + b;
  if (ret) *ret = value;
  return (a > 0xffff - b)
       | ((a & 0b1111) > 0xf - (b & 0b1111)) << 4
       | (((int16_t) b > 0 && (int16_t) a > 0x7fff - (int16_t) b) || ((int16_t) b < 0 && (int16_t) a < -0x7fff - 1 - (int16_t) b)) << 11
       | get_flags_word(value);
}

static uint16_t get_flags_sub_word(uint16_t a, uint16_t b, uint16_t* ret) {
  uint16_t value = a - b;
  if (ret) *ret = value;
  return (a < b)
       | ((a & 0b1111) < (b & 0b1111)) << 4
       | (((int16_t) b < 0 && (int16_t) a > 0x7fff + (int16_t) b) || ((int16_t) b > 0 && (int16_t) a < -0x7fff - 1 + (int16_t) b)) << 11
       | get_flags_word(value);
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
    state->registers.ax &= 0xff00;
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
    state->registers.ax &= 0xff00;
    state->registers.ax |= w86_get_byte(state, state->registers.cs, offset + 1);
    break;

  case 0xb0 | W86_MODRM_REG_CL:
    state->registers.cx &= 0xff00;
    state->registers.cx |= w86_get_byte(state, state->registers.cs, offset + 1);
    break;

  case 0xb0 | W86_MODRM_REG_DL:
    state->registers.dx &= 0xff00;
    state->registers.dx |= w86_get_byte(state, state->registers.cs, offset + 1);
    break;
  
  case 0xb0 | W86_MODRM_REG_BL:
    state->registers.bx &= 0xff00;
    state->registers.bx |= w86_get_byte(state, state->registers.cs, offset + 1);
    break;
  
  case 0xb0 | W86_MODRM_REG_AH:
    state->registers.ax &= 0x00ff;
    state->registers.ax |= w86_get_byte(state, state->registers.cs, offset + 1) << 8;
    break;

  case 0xb0 | W86_MODRM_REG_CH:
    state->registers.cx &= 0x00ff;
    state->registers.cx |= w86_get_byte(state, state->registers.cs, offset + 1) << 8;
    break;

  case 0xb0 | W86_MODRM_REG_DH:
    state->registers.dx &= 0x00ff;
    state->registers.dx |= w86_get_byte(state, state->registers.cs, offset + 1) << 8;
    break;

  case 0xb0 | W86_MODRM_REG_BH:
    state->registers.bx &= 0x00ff;
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

enum w86_status w86_instruction_xchg(struct w86_cpu_state* state, uint16_t offset, struct w86_instruction_prefixes prefixes) {
  uint8_t first_byte = w86_get_byte(state, state->registers.cs, offset);
  struct w86_modrm_info info = {};

  switch (first_byte) {
    union {
      uint8_t u8;
      uint16_t u16;
    } temp;

  case 0x86: // reg8 <-> r/m8
    info = w86_modrm_parse(state, offset + 1, prefixes.segment);
    w86_modrm_get_rm_byte(state, info, &temp.u8);
    w86_modrm_byte_store(state, info, nullptr);
    w86_modrm_set_reg_byte(state, info, temp.u8);
    break;

  case 0x87: // reg16 <-> r/m16
    info = w86_modrm_parse(state, offset + 1, prefixes.segment);
    w86_modrm_get_rm_word(state, info, &temp.u16);
    w86_modrm_word_store(state, info, nullptr);
    w86_modrm_set_reg_word(state, info, temp.u16);
    break;

  case 0x90 | W86_MODRM_REG_AX: // ax <-> reg16
    break;

  case 0x90 | W86_MODRM_REG_CX:
    temp.u16 = state->registers.cx;
    state->registers.cx = state->registers.ax;
    state->registers.ax = temp.u16;
    break;

  case 0x90 | W86_MODRM_REG_DX:
    temp.u16 = state->registers.dx;
    state->registers.dx = state->registers.ax;
    state->registers.ax = temp.u16;
    break;

  case 0x90 | W86_MODRM_REG_BX:
    temp.u16 = state->registers.bx;
    state->registers.bx = state->registers.ax;
    state->registers.ax = temp.u16;
    break;

  case 0x90 | W86_MODRM_REG_SP:
    temp.u16 = state->registers.sp;
    state->registers.sp = state->registers.ax;
    state->registers.ax = temp.u16;
    break;

  case 0x90 | W86_MODRM_REG_BP:
    temp.u16 = state->registers.bp;
    state->registers.bp = state->registers.ax;
    state->registers.ax = temp.u16;
    break;

  case 0x90 | W86_MODRM_REG_SI:
    temp.u16 = state->registers.si;
    state->registers.si = state->registers.ax;
    state->registers.ax = temp.u16;
    break;

  case 0x90 | W86_MODRM_REG_DI:
    temp.u16 = state->registers.di;
    state->registers.di = state->registers.ax;
    state->registers.ax = temp.u16;
    break;

  default:
    return W86_STATUS_INVALID_OPERATION;
  }

  if (first_byte == 0x86
   || first_byte == 0x87) {
    state->registers.ip = offset + 2;
  } else {
    state->registers.ip = offset + 1;
  }
  state->registers.ip += info.size;

  return W86_STATUS_SUCCESS;
}

// arithmetic functions should probably be combined into one, but i don't feel like doing that

enum w86_status w86_instruction_add(struct w86_cpu_state* state, uint16_t offset, struct w86_instruction_prefixes prefixes) {
  uint8_t first_byte = w86_get_byte(state, state->registers.cs, offset);
  struct w86_modrm_info info = {};

  uint16_t flags;
  switch (first_byte) {
    union {
      uint8_t u8;
      uint16_t u16;
    } a, b, c;

  case 0x00: // r/m8 + reg8 -> r/m8
    info = w86_modrm_parse(state, offset + 1, prefixes.segment);
    w86_modrm_get_rm_byte(state, info, &a.u8);
    w86_modrm_get_reg_byte(state, info, &b.u8);
    flags = get_flags_add_byte(a.u8, b.u8, &c.u8);
    w86_modrm_set_rm_byte(state, info, c.u8);
    break;

  case 0x01: // r/m16 + reg16 -> r/m16
    info = w86_modrm_parse(state, offset + 1, prefixes.segment);
    w86_modrm_get_rm_word(state, info, &a.u16);
    w86_modrm_get_reg_word(state, info, &b.u16);
    flags = get_flags_add_word(a.u16, b.u16, &c.u16);
    w86_modrm_set_rm_word(state, info, c.u16);
    break;

  case 0x02: // reg8 + r/m8 -> reg8
    info = w86_modrm_parse(state, offset + 1, prefixes.segment);
    w86_modrm_get_reg_byte(state, info, &a.u8);
    w86_modrm_get_rm_byte(state, info, &b.u8);
    flags = get_flags_add_byte(a.u8, b.u8, &c.u8);
    w86_modrm_set_reg_byte(state, info, c.u8);
    break;

  case 0x03: // reg16 + r/m16 -> reg16
    info = w86_modrm_parse(state, offset + 1, prefixes.segment);
    w86_modrm_get_reg_word(state, info, &a.u16);
    w86_modrm_get_rm_word(state, info, &b.u16);
    flags = get_flags_add_word(a.u16, b.u16, &c.u16);
    w86_modrm_set_reg_word(state, info, c.u16);
    break;

  case 0x04: // al + imm8 -> al
    flags = get_flags_add_byte(state->registers.ax, w86_get_byte(state, state->registers.cs, offset + 1), &c.u8);
    state->registers.ax = c.u8;
    break;

  case 0x05: // ax + imm16 -> ax
    flags = get_flags_add_word(state->registers.ax, w86_get_word(state, state->registers.cs, offset + 1), &state->registers.ax);
    break;

  case 0x80: // r/m8 + imm8 -> r/m8
  case 0x82:
    info = w86_modrm_parse(state, offset + 1, prefixes.segment);
    if (info.reg != 0b000) return W86_STATUS_INVALID_OPERATION;
    w86_modrm_get_rm_byte(state, info, &a.u8);
    flags = get_flags_add_byte(a.u8, w86_get_byte(state, state->registers.cs, offset + 2 + info.size), &c.u8);
    w86_modrm_set_rm_byte(state, info, c.u8);
    break;

  case 0x81: // r/m16 + imm16 -> r/m16
    info = w86_modrm_parse(state, offset + 1, prefixes.segment);
    if (info.reg != 0b000) return W86_STATUS_INVALID_OPERATION;
    w86_modrm_get_rm_word(state, info, &a.u16);
    flags = get_flags_add_word(a.u16, w86_get_word(state, state->registers.cs, offset + 2 + info.size), &c.u16);
    w86_modrm_set_rm_word(state, info, c.u16);
    break;

  case 0x83: // r/m16 + imm16sbw -> r/m16
    info = w86_modrm_parse(state, offset + 1, prefixes.segment);
    if (info.reg != 0b000) return W86_STATUS_INVALID_OPERATION;
    w86_modrm_get_rm_word(state, info, &a.u16);
    flags = get_flags_add_word(a.u16, sbw(w86_get_byte(state, state->registers.cs, offset + 2 + info.size)), &c.u16);
    w86_modrm_set_rm_word(state, info, c.u16);
    break;

  default:
    return W86_STATUS_INVALID_OPERATION;
  }
  state->registers.flags &= 0b00000111'00000000;
  state->registers.flags |= flags & 0b11111000'11111111;

  if (first_byte == 0x81) {
    state->registers.ip = offset + 4;
  } else if (first_byte == 0x05
          || first_byte == 0x80
          || first_byte == 0x82
          || first_byte == 0x83) {
    state->registers.ip = offset + 3;
  } else {
    state->registers.ip = offset + 2;
  }
  state->registers.ip += info.size;

  return W86_STATUS_SUCCESS;
}

enum w86_status w86_instruction_inc(struct w86_cpu_state* state, uint16_t offset, struct w86_instruction_prefixes prefixes) {
  uint8_t first_byte = w86_get_byte(state, state->registers.cs, offset);
  struct w86_modrm_info info = {};

  uint16_t flags;
  switch (first_byte) {
    union {
      uint8_t u8;
      uint16_t u16;
    } a;

  case 0x40 | W86_MODRM_REG_AX: // reg16 + 1 -> reg16
    flags = get_flags_add_word(state->registers.ax, 1, &state->registers.ax);
    break;

  case 0x40 | W86_MODRM_REG_CX:
    flags = get_flags_add_word(state->registers.cx, 1, &state->registers.cx);
    break;

  case 0x40 | W86_MODRM_REG_DX:
    flags = get_flags_add_word(state->registers.dx, 1, &state->registers.dx);
    break;

  case 0x40 | W86_MODRM_REG_BX:
    flags = get_flags_add_word(state->registers.bx, 1, &state->registers.bx);
    break;

  case 0x40 | W86_MODRM_REG_SP:
    flags = get_flags_add_word(state->registers.sp, 1, &state->registers.sp);
    break;

  case 0x40 | W86_MODRM_REG_BP:
    flags = get_flags_add_word(state->registers.bp, 1, &state->registers.bp);
    break;

  case 0x40 | W86_MODRM_REG_SI:
    flags = get_flags_add_word(state->registers.si, 1, &state->registers.si);
    break;

  case 0x40 | W86_MODRM_REG_DI:
    flags = get_flags_add_word(state->registers.di, 1, &state->registers.di);
    break;

  case 0xfe: // r/m8 + 1 -> r/m8
    info = w86_modrm_parse(state, offset + 1, prefixes.segment);
    if (info.reg != 0b000) return W86_STATUS_INVALID_OPERATION;
    w86_modrm_get_rm_byte(state, info, &a.u8);
    flags = get_flags_add_byte(a.u8, 1, &a.u8);
    w86_modrm_set_rm_byte(state, info, a.u8);
    break;

  case 0xff: // r/m16 + 1 -> r/m16
    info = w86_modrm_parse(state, offset + 1, prefixes.segment);
    if (info.reg != 0b000) return W86_STATUS_INVALID_OPERATION;
    w86_modrm_get_rm_word(state, info, &a.u16);
    flags = get_flags_add_word(a.u16, 1, &a.u16);
    w86_modrm_set_rm_word(state, info, a.u16);
    break;

  default:
    return W86_STATUS_INVALID_OPERATION;
  }
  state->registers.flags &= 0b00000111'00000001;
  state->registers.flags |= flags & 0b11111000'11111110;

  if (first_byte == 0xfe
   || first_byte == 0xff) {
    state->registers.ip = offset + 2;
  } else {
    state->registers.ip = offset + 1;
  }
  state->registers.ip += info.size;

  return W86_STATUS_SUCCESS;
}

enum w86_status w86_instruction_sub(struct w86_cpu_state* state, uint16_t offset, struct w86_instruction_prefixes prefixes) {
  uint8_t first_byte = w86_get_byte(state, state->registers.cs, offset);
  struct w86_modrm_info info = {};

  uint16_t flags;
  switch (first_byte) {
    union {
      uint8_t u8;
      uint16_t u16;
    } a, b, c;

  case 0x28: // r/m8 - reg8 -> r/m8
    info = w86_modrm_parse(state, offset + 1, prefixes.segment);
    w86_modrm_get_rm_byte(state, info, &a.u8);
    w86_modrm_get_reg_byte(state, info, &b.u8);
    flags = get_flags_sub_byte(a.u8, b.u8, &c.u8);
    w86_modrm_set_rm_byte(state, info, c.u8);
    break;

  case 0x29: // r/m16 - reg16 -> r/m16
    info = w86_modrm_parse(state, offset + 1, prefixes.segment);
    w86_modrm_get_rm_word(state, info, &a.u16);
    w86_modrm_get_reg_word(state, info, &b.u16);
    flags = get_flags_sub_word(a.u16, b.u16, &c.u16);
    w86_modrm_set_rm_word(state, info, c.u16);
    break;

  case 0x2a: // reg8 - r/m8 -> reg8
    info = w86_modrm_parse(state, offset + 1, prefixes.segment);
    w86_modrm_get_reg_byte(state, info, &a.u8);
    w86_modrm_get_rm_byte(state, info, &b.u8);
    flags = get_flags_sub_byte(a.u8, b.u8, &c.u8);
    w86_modrm_set_reg_byte(state, info, c.u8);
    break;

  case 0x2b: // reg16 - r/m16 -> reg16
    info = w86_modrm_parse(state, offset + 1, prefixes.segment);
    w86_modrm_get_reg_word(state, info, &a.u16);
    w86_modrm_get_rm_word(state, info, &b.u16);
    flags = get_flags_sub_word(a.u16, b.u16, &c.u16);
    w86_modrm_set_reg_word(state, info, c.u16);
    break;

  case 0x2c: // al - imm8 -> al
    flags = get_flags_sub_byte(state->registers.ax, w86_get_byte(state, state->registers.cs, offset + 1), &c.u8);
    state->registers.ax = c.u8;
    break;

  case 0x2d: // ax - imm16 -> ax
    flags = get_flags_sub_word(state->registers.ax, w86_get_word(state, state->registers.cs, offset + 1), &state->registers.ax);
    break;

  case 0x80: // r/m8 - imm8 -> r/m8
  case 0x82:
    info = w86_modrm_parse(state, offset + 1, prefixes.segment);
    if (info.reg != 0b101) return W86_STATUS_INVALID_OPERATION;
    w86_modrm_get_rm_byte(state, info, &a.u8);
    flags = get_flags_sub_byte(a.u8, w86_get_byte(state, state->registers.cs, offset + 2 + info.size), &c.u8);
    w86_modrm_set_rm_byte(state, info, c.u8);
    break;

  case 0x81: // r/m16 - imm16 -> r/m16
    info = w86_modrm_parse(state, offset + 1, prefixes.segment);
    if (info.reg != 0b101) return W86_STATUS_INVALID_OPERATION;
    w86_modrm_get_rm_word(state, info, &a.u16);
    flags = get_flags_sub_word(a.u16, w86_get_word(state, state->registers.cs, offset + 2 + info.size), &c.u16);
    w86_modrm_set_rm_word(state, info, c.u16);
    break;

  case 0x83: // r/m16 - imm16sbw -> r/m16
    info = w86_modrm_parse(state, offset + 1, prefixes.segment);
    if (info.reg != 0b101) return W86_STATUS_INVALID_OPERATION;
    w86_modrm_get_rm_word(state, info, &a.u16);
    flags = get_flags_sub_word(a.u16, sbw(w86_get_byte(state, state->registers.cs, offset + 2 + info.size)), &c.u16);
    w86_modrm_set_rm_word(state, info, c.u16);
    break;

  default:
    return W86_STATUS_INVALID_OPERATION;
  }
  state->registers.flags &= 0b00000111'00000000;
  state->registers.flags |= flags & 0b11111000'11111111;

  if (first_byte == 0x81) {
    state->registers.ip = offset + 4;
  } else if (first_byte == 0x2d
          || first_byte == 0x80
          || first_byte == 0x82
          || first_byte == 0x83) {
    state->registers.ip = offset + 3;
  } else {
    state->registers.ip = offset + 2;
  }
  state->registers.ip += info.size;

  return W86_STATUS_SUCCESS;
}

enum w86_status w86_instruction_dec(struct w86_cpu_state* state, uint16_t offset, struct w86_instruction_prefixes prefixes) {
  uint8_t first_byte = w86_get_byte(state, state->registers.cs, offset);
  struct w86_modrm_info info = {};

  uint16_t flags;
  switch (first_byte) {
    union {
      uint8_t u8;
      uint16_t u16;
    } a;

  case 0x48 | W86_MODRM_REG_AX: // reg16 - 1 -> reg16
    flags = get_flags_sub_word(state->registers.ax, 1, &state->registers.ax);
    break;

  case 0x48 | W86_MODRM_REG_CX:
    flags = get_flags_sub_word(state->registers.cx, 1, &state->registers.cx);
    break;

  case 0x48 | W86_MODRM_REG_DX:
    flags = get_flags_sub_word(state->registers.dx, 1, &state->registers.dx);
    break;

  case 0x48 | W86_MODRM_REG_BX:
    flags = get_flags_sub_word(state->registers.bx, 1, &state->registers.bx);
    break;

  case 0x48 | W86_MODRM_REG_SP:
    flags = get_flags_sub_word(state->registers.sp, 1, &state->registers.sp);
    break;

  case 0x48 | W86_MODRM_REG_BP:
    flags = get_flags_sub_word(state->registers.bp, 1, &state->registers.bp);
    break;

  case 0x48 | W86_MODRM_REG_SI:
    flags = get_flags_sub_word(state->registers.si, 1, &state->registers.si);
    break;

  case 0x48 | W86_MODRM_REG_DI:
    flags = get_flags_sub_word(state->registers.di, 1, &state->registers.di);
    break;

  case 0xfe: // r/m8 - 1 -> r/m8
    info = w86_modrm_parse(state, offset + 1, prefixes.segment);
    if (info.reg != 0b001) return W86_STATUS_INVALID_OPERATION;
    w86_modrm_get_rm_byte(state, info, &a.u8);
    flags = get_flags_sub_byte(a.u8, 1, &a.u8);
    w86_modrm_set_rm_byte(state, info, a.u8);
    break;

  case 0xff: // r/m16 - 1 -> r/m16
    info = w86_modrm_parse(state, offset + 1, prefixes.segment);
    if (info.reg != 0b001) return W86_STATUS_INVALID_OPERATION;
    w86_modrm_get_rm_word(state, info, &a.u16);
    flags = get_flags_sub_word(a.u16, 1, &a.u16);
    w86_modrm_set_rm_word(state, info, a.u16);
    break;

  default:
    return W86_STATUS_INVALID_OPERATION;
  }
  state->registers.flags &= 0b00000111'00000001;
  state->registers.flags |= flags & 0b11111000'11111110;

  if (first_byte == 0xfe
   || first_byte == 0xff) {
    state->registers.ip = offset + 2;
  } else {
    state->registers.ip = offset + 1;
  }
  state->registers.ip += info.size;

  return W86_STATUS_SUCCESS;
}

enum w86_status w86_instruction_cmp(struct w86_cpu_state* state, uint16_t offset, struct w86_instruction_prefixes prefixes) {
  uint8_t first_byte = w86_get_byte(state, state->registers.cs, offset);
  struct w86_modrm_info info = {};

  uint16_t flags;
  switch (first_byte) {
    union {
      uint8_t u8;
      uint16_t u16;
    } a, b;

  case 0x38: // r/m8 - reg8
    info = w86_modrm_parse(state, offset + 1, prefixes.segment);
    w86_modrm_get_rm_byte(state, info, &a.u8);
    w86_modrm_get_reg_byte(state, info, &b.u8);
    flags = get_flags_sub_byte(a.u8, b.u8, nullptr);
    break;

  case 0x39: // r/m16 - reg16
    info = w86_modrm_parse(state, offset + 1, prefixes.segment);
    w86_modrm_get_rm_word(state, info, &a.u16);
    w86_modrm_get_reg_word(state, info, &b.u16);
    flags = get_flags_sub_word(a.u16, b.u16, nullptr);
    break;

  case 0x3a: // reg8 - r/m8
    info = w86_modrm_parse(state, offset + 1, prefixes.segment);
    w86_modrm_get_reg_byte(state, info, &a.u8);
    w86_modrm_get_rm_byte(state, info, &b.u8);
    flags = get_flags_sub_byte(a.u8, b.u8, nullptr);
    break;

  case 0x3b: // reg16 - r/m16
    info = w86_modrm_parse(state, offset + 1, prefixes.segment);
    w86_modrm_get_reg_word(state, info, &a.u16);
    w86_modrm_get_rm_word(state, info, &b.u16);
    flags = get_flags_sub_word(a.u16, b.u16, nullptr);
    break;

  case 0x3c: // al - imm8
    flags = get_flags_sub_byte(state->registers.ax, w86_get_byte(state, state->registers.cs, offset + 1), nullptr);
    break;

  case 0x3d: // ax - imm16
    flags = get_flags_sub_word(state->registers.ax, w86_get_word(state, state->registers.cs, offset + 1), nullptr);
    break;

  case 0x80: // r/m8 - imm8
  case 0x82:
    info = w86_modrm_parse(state, offset + 1, prefixes.segment);
    if (info.reg != 0b111) return W86_STATUS_INVALID_OPERATION;
    w86_modrm_get_rm_byte(state, info, &a.u8);
    flags = get_flags_sub_byte(a.u8, w86_get_byte(state, state->registers.cs, offset + 2 + info.size), nullptr);
    break;

  case 0x81: // r/m16 - imm16
    info = w86_modrm_parse(state, offset + 1, prefixes.segment);
    if (info.reg != 0b111) return W86_STATUS_INVALID_OPERATION;
    w86_modrm_get_rm_word(state, info, &a.u16);
    flags = get_flags_sub_word(a.u16, w86_get_word(state, state->registers.cs, offset + 2 + info.size), nullptr);
    break;

  case 0x83: // r/m16 - imm16sbw
    info = w86_modrm_parse(state, offset + 1, prefixes.segment);
    if (info.reg != 0b111) return W86_STATUS_INVALID_OPERATION;
    w86_modrm_get_rm_word(state, info, &a.u16);
    flags = get_flags_sub_word(a.u16, sbw(w86_get_byte(state, state->registers.cs, offset + 2 + info.size)), nullptr);
    break;

  default:
    return W86_STATUS_INVALID_OPERATION;
  }
  state->registers.flags &= 0b00000111'00000000;
  state->registers.flags |= flags & 0b11111000'11111111;

  if (first_byte == 0x81) {
    state->registers.ip = offset + 4;
  } else if (first_byte == 0x3d
          || first_byte == 0x80
          || first_byte == 0x82
          || first_byte == 0x83) {
    state->registers.ip = offset + 3;
  } else {
    state->registers.ip = offset + 2;
  }
  state->registers.ip += info.size;

  return W86_STATUS_SUCCESS;
}

enum w86_status w86_instruction_call(struct w86_cpu_state* state, uint16_t offset, struct w86_instruction_prefixes) {
  switch (w86_get_byte(state, state->registers.cs, offset)) {
  case 0x9a: // far call
    state->registers.sp -= 4;
    w86_set_word(state, state->registers.ss, state->registers.sp, offset + 5);
    state->registers.ip = w86_get_word(state, state->registers.cs, offset + 1);
    w86_set_word(state, state->registers.ss, state->registers.sp + 2, state->registers.cs);
    state->registers.cs = w86_get_word(state, state->registers.cs, offset + 3);
    break;

  case 0xe8: // near call
    state->registers.sp -= 2;
    w86_set_word(state, state->registers.ss, state->registers.sp, offset + 3);
    state->registers.ip += (int16_t) w86_get_word(state, state->registers.cs, offset + 1) + 3;
    break;
  
  case 0xff: // indirect call
    return W86_STATUS_UNIMPLEMENTED_OPCODE;

  default:
    return W86_STATUS_INVALID_OPERATION;
  }

  return W86_STATUS_SUCCESS;
}

enum w86_status w86_instruction_ret(struct w86_cpu_state* state, uint16_t offset, struct w86_instruction_prefixes) {
  uint8_t first_byte = w86_get_byte(state, state->registers.cs, offset);
  switch (first_byte) {
  case 0xc2: // near return with imm16
  case 0xc3: // near return
  case 0xca: // far return with imm16
  case 0xcb: // far return
    uint16_t pop = !(first_byte & 0b00000001) ? w86_get_word(state, state->registers.cs, offset + 1) : 0;
    state->registers.ip = w86_get_word(state, state->registers.ss, state->registers.sp);
    state->registers.sp += 2;
    if (first_byte & 0b00001000) {
      state->registers.cs = w86_get_word(state, state->registers.ss, state->registers.sp);
      state->registers.sp += 2;
    }
    state->registers.sp += pop;
    break;

  default:
    return W86_STATUS_INVALID_OPERATION;
  }

  return W86_STATUS_SUCCESS;
}

enum w86_status w86_instruction_jmp(struct w86_cpu_state* state, uint16_t offset, struct w86_instruction_prefixes) {
  switch (w86_get_byte(state, state->registers.cs, offset)) {
  case 0xe9: // near jump
    state->registers.ip += (int16_t) w86_get_word(state, state->registers.cs, offset + 1) + 3;
    break;

  case 0xea: // far jump
    state->registers.ip = w86_get_word(state, state->registers.cs, offset + 1);
    state->registers.cs = w86_get_word(state, state->registers.cs, offset + 3);
    break;

  case 0xeb: // short jump
    state->registers.ip += (int8_t) w86_get_byte(state, state->registers.cs, offset + 1) + 2;
    break;

  case 0xff: // indirect jump
    return W86_STATUS_UNIMPLEMENTED_OPCODE;

  default:
    return W86_STATUS_INVALID_OPERATION;
  }

  return W86_STATUS_SUCCESS;
}

// look ma, no switch statements!
enum w86_status w86_instruction_jcc(struct w86_cpu_state* state, uint16_t offset, struct w86_instruction_prefixes) {
  uint8_t first_byte = w86_get_byte(state, state->registers.cs, offset);
  if ((first_byte & 0b11110000) != 0x70) return W86_STATUS_INVALID_OPERATION;

  // create flags bitmask
  uint16_t cond = (~first_byte >> 3 &                     first_byte >> 1  & 0b00000000'00000001) // cf
                | ( first_byte >> 1 & ~first_byte      &  first_byte << 1  & 0b00000000'00000100) // pf
                | (~first_byte << 3 &  first_byte << 4                     & 0b00000000'01000000) // zf
                | (                    first_byte << 4 &  first_byte << 5  & 0b00000000'01000000)
                | ( first_byte << 4 &  first_byte << 5                     & 0b00000000'10000000) // sf
                | (~first_byte << 8 & ~first_byte << 9 & ~first_byte << 10 & 0b00001000'00000000) // of
                | ( first_byte << 8 &  first_byte << 9                     & 0b00001000'00000000);
  cond &= state->registers.flags;
  state->registers.ip = offset + 2;
  if ((((cond >> 11 ^ cond >> 7) | cond >> 6 | cond >> 2 | cond) ^ first_byte) & 1) {
    state->registers.ip += (int8_t) w86_get_byte(state, state->registers.cs, offset + 1);
  }

  return W86_STATUS_SUCCESS;
}
