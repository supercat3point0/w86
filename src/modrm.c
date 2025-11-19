// SPDX-License-Identifier: GPL-3.0-or-later

#include "modrm.h"

#include <stdint.h>

#include "address.h"
#include "decode.h"
#include "w86.h"

struct w86_modrm_info w86_modrm_parse(struct w86_cpu_state* state, uint16_t offset, enum w86_segment_prefix segment) {
  uint8_t modrm = w86_get_byte(state, state->registers.cs, offset);
  struct w86_modrm_info info = {
    .mod = modrm >> 6 & 0b11,
    .reg = modrm >> 3 & 0b111,
    .segment = segment
  };

  switch (info.mod) {
  case W86_MODRM_MOD_MEM:
    info.rm.mem = modrm & 0b111;
    info.disp = 0;
    info.rm_is_reg = false;
    info.size = info.rm.mem == W86_MODRM_MEM_DIRECT ? 2 : 0;
    break;

  case W86_MODRM_MOD_MEM_DISP8:
    info.rm.mem = modrm & 0b111;
    info.disp = (int8_t) w86_get_byte(state, state->registers.cs, offset + 1);
    info.rm_is_reg = false;
    info.size = 1;
    break;

  case W86_MODRM_MOD_MEM_DISP16:
    info.rm.mem = modrm & 0b111;
    info.disp = (int16_t) w86_get_word(state, state->registers.cs, offset + 1);
    info.rm_is_reg = false;
    info.size = 2;
    break;

  case W86_MODRM_MOD_REG:
    info.rm.reg = modrm & 0b111;
    info.disp = 0;
    info.rm_is_reg = true;
    info.address = 0x0000;
    info.size = 0;
  }

  if (info.mod != W86_MODRM_MOD_REG) switch (info.rm.mem) {
  case W86_MODRM_MEM_BX_SI:
    info.address = state->registers.bx + state->registers.si + info.disp;
    break;
    
  case W86_MODRM_MEM_BX_DI:
    info.address = state->registers.bx + state->registers.di + info.disp;
    break;
    
  case W86_MODRM_MEM_BP_SI:
    info.address = state->registers.bp + state->registers.si + info.disp;
    break;

  case W86_MODRM_MEM_BP_DI:
    info.address = state->registers.bp + state->registers.di + info.disp;
    break;

  case W86_MODRM_MEM_SI:
    info.address = state->registers.si + info.disp;
    break;

  case W86_MODRM_MEM_DI:
    info.address = state->registers.di + info.disp;
    break;

  case W86_MODRM_MEM_BP:
    info.address = info.mod == W86_MODRM_MOD_MEM ? w86_get_word(state, state->registers.cs, offset + 1) : state->registers.bp + info.disp;
    break;

  case W86_MODRM_MEM_BX:
    info.address = state->registers.bx + info.disp;
  }

  return info;
}

bool w86_modrm_byte_load(struct w86_cpu_state* state, struct w86_modrm_info info, uint8_t* ret) {
  uint8_t value;
  if (info.mod == W86_MODRM_MOD_REG) switch (info.rm.reg) {
  case W86_MODRM_REG_AL:
    value = state->registers.ax;
    break;

  case W86_MODRM_REG_CL:
    value = state->registers.cx;
    break;

  case W86_MODRM_REG_DL:
    value = state->registers.dx;
    break;

  case W86_MODRM_REG_BL:
    value = state->registers.bx;
    break;

  case W86_MODRM_REG_AH:
    value = state->registers.ax >> 8;
    break;

  case W86_MODRM_REG_CH:
    value = state->registers.cx >> 8;
    break;

  case W86_MODRM_REG_DH:
    value = state->registers.dx >> 8;
    break;

  case W86_MODRM_REG_BH:
    value = state->registers.bx >> 8;
    break;

  default:
    return false;
  } else switch (info.segment) {
  case W86_SEGMENT_PREFIX_CS:
    value = w86_get_byte(state, state->registers.cs, info.address);
    break;

  case W86_SEGMENT_PREFIX_NONE:
  case W86_SEGMENT_PREFIX_DS:
    value = w86_get_byte(state, state->registers.ds, info.address);
    break;

  case W86_SEGMENT_PREFIX_ES:
    value = w86_get_byte(state, state->registers.es, info.address);
    break;

  case W86_SEGMENT_PREFIX_SS:
    value = w86_get_byte(state, state->registers.ss, info.address);
    break;

  default:
    return false;
  }

  switch (info.reg) {
  case W86_MODRM_REG_AL:
    state->registers.ax &= 0xff00;
    state->registers.ax |= value;
    break;

  case W86_MODRM_REG_CL:
    state->registers.cx &= 0xff00;
    state->registers.cx |= value;
    break;

  case W86_MODRM_REG_DL:
    state->registers.dx &= 0xff00;
    state->registers.dx |= value;
    break;

  case W86_MODRM_REG_BL:
    state->registers.bx &= 0xff00;
    state->registers.bx |= value;
    break;

  case W86_MODRM_REG_AH:
    state->registers.ax &= 0x00ff;
    state->registers.ax |= value << 8;
    break;

  case W86_MODRM_REG_CH:
    state->registers.cx &= 0x00ff;
    state->registers.cx |= value << 8;
    break;

  case W86_MODRM_REG_DH:
    state->registers.dx &= 0x00ff;
    state->registers.dx |= value << 8;
    break;

  case W86_MODRM_REG_BH:
    state->registers.bx &= 0x00ff;
    state->registers.bx |= value << 8;
    break;

  default:
    return false;
  }

  if (ret) *ret = value;
  return true;
}

bool w86_modrm_byte_store(struct w86_cpu_state* state, struct w86_modrm_info info, uint8_t* ret) {
  uint8_t value;
  switch (info.reg) {
  case W86_MODRM_REG_AL:
    value = state->registers.ax;
    break;

  case W86_MODRM_REG_CL:
    value = state->registers.cx;
    break;

  case W86_MODRM_REG_DL:
    value = state->registers.dx;
    break;

  case W86_MODRM_REG_BL:
    value = state->registers.bx;
    break;

  case W86_MODRM_REG_AH:
    value = state->registers.ax >> 8;
    break;

  case W86_MODRM_REG_CH:
    value = state->registers.cx >> 8;
    break;

  case W86_MODRM_REG_DH:
    value = state->registers.dx >> 8;
    break;

  case W86_MODRM_REG_BH:
    value = state->registers.bx >> 8;
    break;

  default:
    return false;
  }

  if (info.mod == W86_MODRM_MOD_REG) switch (info.rm.reg) {
  case W86_MODRM_REG_AL:
    state->registers.ax &= 0xff00;
    state->registers.ax |= value;
    break;

  case W86_MODRM_REG_CL:
    state->registers.cx &= 0xff00;
    state->registers.cx |= value;
    break;

  case W86_MODRM_REG_DL:
    state->registers.dx &= 0xff00;
    state->registers.dx |= value;
    break;

  case W86_MODRM_REG_BL:
    state->registers.bx &= 0xff00;
    state->registers.bx |= value;
    break;

  case W86_MODRM_REG_AH:
    state->registers.ax &= 0x00ff;
    state->registers.ax |= value << 8;
    break;

  case W86_MODRM_REG_CH:
    state->registers.cx &= 0x00ff;
    state->registers.cx |= value << 8;
    break;

  case W86_MODRM_REG_DH:
    state->registers.dx &= 0x00ff;
    state->registers.dx |= value << 8;
    break;

  case W86_MODRM_REG_BH:
    state->registers.bx &= 0x00ff;
    state->registers.bx |= value << 8;
    break;

  default:
    return false;
  } else switch (info.segment) {
  case W86_SEGMENT_PREFIX_CS:
    w86_set_byte(state, state->registers.cs, info.address, value);
    break;

  case W86_SEGMENT_PREFIX_NONE:
  case W86_SEGMENT_PREFIX_DS:
    w86_set_byte(state, state->registers.ds, info.address, value);
    break;

  case W86_SEGMENT_PREFIX_ES:
    w86_set_byte(state, state->registers.es, info.address, value);
    break;
    
  case W86_SEGMENT_PREFIX_SS:
    w86_set_byte(state, state->registers.ss, info.address, value);
    break;

  default:
    return false;
  }

  if (ret) *ret = value;
  return true;
}

bool w86_modrm_word_load(struct w86_cpu_state* state, struct w86_modrm_info info, uint16_t* ret) {
  uint16_t value;
  if (info.mod == W86_MODRM_MOD_REG) switch (info.rm.reg) {
  case W86_MODRM_REG_AX:
    value = state->registers.ax;
    break;

  case W86_MODRM_REG_CX:
    value = state->registers.cx;
    break;

  case W86_MODRM_REG_DX:
    value = state->registers.dx;
    break;

  case W86_MODRM_REG_BX:
    value = state->registers.bx;
    break;

  case W86_MODRM_REG_SP:
    value = state->registers.sp;
    break;

  case W86_MODRM_REG_BP:
    value = state->registers.bp;
    break;

  case W86_MODRM_REG_SI:
    value = state->registers.si;
    break;

  case W86_MODRM_REG_DI:
    value = state->registers.di;
    break;

  default:
    return false;
  } else switch (info.segment) {
  case W86_SEGMENT_PREFIX_CS:
    value = w86_get_word(state, state->registers.cs, info.address);
    break;

  case W86_SEGMENT_PREFIX_NONE:
  case W86_SEGMENT_PREFIX_DS:
    value = w86_get_word(state, state->registers.ds, info.address);
    break;

  case W86_SEGMENT_PREFIX_ES:
    value = w86_get_word(state, state->registers.es, info.address);
    break;

  case W86_SEGMENT_PREFIX_SS:
    value = w86_get_word(state, state->registers.ss, info.address);
    break;

  default:
    return false;
  }

  switch (info.reg) {
  case W86_MODRM_REG_AX:
    state->registers.ax = value;
    break;

  case W86_MODRM_REG_CX:
    state->registers.cx = value;
    break;

  case W86_MODRM_REG_DX:
    state->registers.dx = value;
    break;

  case W86_MODRM_REG_BX:
    state->registers.bx = value;
    break;

  case W86_MODRM_REG_SP:
    state->registers.sp = value;
    break;

  case W86_MODRM_REG_BP:
    state->registers.bp = value;
    break;

  case W86_MODRM_REG_SI:
    state->registers.si = value;
    break;

  case W86_MODRM_REG_DI:
    state->registers.di = value;
    break;

  default:
    return false;
  }

  if (ret) *ret = value;
  return true;
}

bool w86_modrm_word_store(struct w86_cpu_state* state, struct w86_modrm_info info, uint16_t* ret) {
  uint16_t value;
  switch (info.reg) {
  case W86_MODRM_REG_AX:
    value = state->registers.ax;
    break;

  case W86_MODRM_REG_CX:
    value = state->registers.cx;
    break;

  case W86_MODRM_REG_DX:
    value = state->registers.dx;
    break;

  case W86_MODRM_REG_BX:
    value = state->registers.bx;
    break;

  case W86_MODRM_REG_SP:
    value = state->registers.sp;
    break;

  case W86_MODRM_REG_BP:
    value = state->registers.bp;
    break;

  case W86_MODRM_REG_SI:
    value = state->registers.si;
    break;

  case W86_MODRM_REG_DI:
    value = state->registers.di;
    break;

  default:
    return false;
  }

  if (info.mod == W86_MODRM_MOD_REG) switch (info.rm.reg) {
  case W86_MODRM_REG_AX:
    state->registers.ax = value;
    break;

  case W86_MODRM_REG_CX:
    state->registers.cx = value;
    break;

  case W86_MODRM_REG_DX:
    state->registers.dx = value;
    break;

  case W86_MODRM_REG_BX:
    state->registers.bx = value;
    break;

  case W86_MODRM_REG_SP:
    state->registers.sp = value;
    break;

  case W86_MODRM_REG_BP:
    state->registers.bp = value;
    break;

  case W86_MODRM_REG_SI:
    state->registers.si = value;
    break;

  case W86_MODRM_REG_DI:
    state->registers.di = value;
    break;

  default:
    return false;
  } else switch (info.segment) {
  case W86_SEGMENT_PREFIX_CS:
    w86_set_word(state, state->registers.cs, info.address, value);
    break;

  case W86_SEGMENT_PREFIX_NONE:
  case W86_SEGMENT_PREFIX_DS:
    w86_set_word(state, state->registers.ds, info.address, value);
    break;

  case W86_SEGMENT_PREFIX_ES:
    w86_set_word(state, state->registers.es, info.address, value);
    break;

  case W86_SEGMENT_PREFIX_SS:
    w86_set_word(state, state->registers.ss, info.address, value);
    break;

  default:
    return false;
  }

  if (ret) *ret = value;
  return true;
}

bool w86_modrm_segment_load(struct w86_cpu_state* state, struct w86_modrm_info info, uint16_t* ret) {
  uint16_t value;
  if (info.mod == W86_MODRM_MOD_REG) switch (info.rm.reg) {
  case W86_MODRM_REG_AX:
    value = state->registers.ax;
    break;

  case W86_MODRM_REG_CX:
    value = state->registers.cx;
    break;

  case W86_MODRM_REG_DX:
    value = state->registers.dx;
    break;

  case W86_MODRM_REG_BX:
    value = state->registers.bx;
    break;

  case W86_MODRM_REG_SP:
    value = state->registers.sp;
    break;

  case W86_MODRM_REG_BP:
    value = state->registers.bp;
    break;

  case W86_MODRM_REG_SI:
    value = state->registers.si;
    break;

  case W86_MODRM_REG_DI:
    value = state->registers.di;
    break;

  default:
    return false;
  } else switch (info.segment) {
  case W86_SEGMENT_PREFIX_CS:
    value = w86_get_word(state, state->registers.cs, info.address);
    break;

  case W86_SEGMENT_PREFIX_NONE:
  case W86_SEGMENT_PREFIX_DS:
    value = w86_get_word(state, state->registers.ds, info.address);
    break;

  case W86_SEGMENT_PREFIX_ES:
    value = w86_get_word(state, state->registers.es, info.address);
    break;

  case W86_SEGMENT_PREFIX_SS:
    value = w86_get_word(state, state->registers.ss, info.address);
    break;

  default:
    return false;
  }

  switch (info.reg) {
  case W86_MODRM_REG_ES:
    state->registers.es = value;
    break;

  case W86_MODRM_REG_CS:
    state->registers.cs = value;
    break;

  case W86_MODRM_REG_SS:
    state->registers.ss = value;
    break;

  case W86_MODRM_REG_DS:
    state->registers.ds = value;
    break;

  default:
    return false;
  }

  if (ret) *ret = value;
  return true;
}

bool w86_modrm_segment_store(struct w86_cpu_state* state, struct w86_modrm_info info, uint16_t* ret) {
  uint16_t value;
  switch (info.reg) {
  case W86_MODRM_REG_ES:
    value = state->registers.es;
    break;

  case W86_MODRM_REG_CS:
    value = state->registers.cs;
    break;

  case W86_MODRM_REG_SS:
    value = state->registers.ss;
    break;

  case W86_MODRM_REG_DS:
    value = state->registers.ds;
    break;

  default:
    return false;
  }

  if (info.mod == W86_MODRM_MOD_REG) switch (info.rm.reg) {
  case W86_MODRM_REG_AX:
    state->registers.ax = value;
    break;

  case W86_MODRM_REG_CX:
    state->registers.cx = value;
    break;

  case W86_MODRM_REG_DX:
    state->registers.dx = value;
    break;

  case W86_MODRM_REG_BX:
    state->registers.bx = value;
    break;

  case W86_MODRM_REG_SP:
    state->registers.sp = value;
    break;

  case W86_MODRM_REG_BP:
    state->registers.bp = value;
    break;

  case W86_MODRM_REG_SI:
    state->registers.si = value;
    break;

  case W86_MODRM_REG_DI:
    state->registers.di = value;
    break;

  default:
    return false;
  } else switch (info.segment) {
  case W86_SEGMENT_PREFIX_CS:
    w86_set_word(state, state->registers.cs, info.address, value);
    break;

  case W86_SEGMENT_PREFIX_NONE:
  case W86_SEGMENT_PREFIX_DS:
    w86_set_word(state, state->registers.ds, info.address, value);
    break;

  case W86_SEGMENT_PREFIX_ES:
    w86_set_word(state, state->registers.es, info.address, value);
    break;

  case W86_SEGMENT_PREFIX_SS:
    w86_set_word(state, state->registers.ss, info.address, value);
    break;

  default:
    return false;
  }

  if (ret) *ret = value;
  return true;
}
