// SPDX-License-Identifier: GPL-3.0-or-later

#include "decode.h"

#include <stdint.h>

#include "address.h"
#include "instruction.h"
#include "w86.h"

// welcome to switch statement purgatory...

enum w86_status w86_decode(struct w86_cpu_state* state) {
  uint16_t offset = state->registers.ip;
  struct w86_instruction_prefixes prefixes = {
    .segment = W86_SEGMENT_PREFIX_NONE,
    .repeat = W86_REPEAT_PREFIX_NONE,
    .lock = false
  };

  while (true) switch (w86_get_byte(state, state->registers.cs, offset)) {
  case 0x88: // mov
  case 0x89:
  case 0x8a:
  case 0x8b:
  case 0x8c:
  case 0x8e:
  case 0xa0:
  case 0xa1:
  case 0xa2:
  case 0xa3:
  case 0xb0:
  case 0xb1:
  case 0xb2:
  case 0xb3:
  case 0xb4:
  case 0xb5:
  case 0xb6:
  case 0xb7:
  case 0xb8:
  case 0xb9:
  case 0xba:
  case 0xbb:
  case 0xbc:
  case 0xbd:
  case 0xbe:
  case 0xbf:
  case 0xc6:
  case 0xc7:
    return w86_instruction_mov(state, offset, prefixes);

  case 0x86: // xchg
  case 0x87:
  case 0x90:
  case 0x91:
  case 0x92:
  case 0x93:
  case 0x94:
  case 0x95:
  case 0x96:
  case 0x97:
    return w86_instruction_xchg(state, offset, prefixes);

  case 0x00: // add
  case 0x01:
  case 0x02:
  case 0x03:
  case 0x04:
  case 0x05:
    return w86_instruction_add(state, offset, prefixes);

  case 0x40: // inc
  case 0x41:
  case 0x42:
  case 0x43:
  case 0x44:
  case 0x45:
  case 0x46:
  case 0x47:
    return w86_instruction_inc(state, offset, prefixes);

  case 0x28: // sub
  case 0x29:
  case 0x2a:
  case 0x2b:
  case 0x2c:
  case 0x2d:
    return w86_instruction_sub(state, offset, prefixes);

  case 0x48: // dec
  case 0x49:
  case 0x4a:
  case 0x4b:
  case 0x4c:
  case 0x4d:
  case 0x4e:
  case 0x4f:
    return w86_instruction_dec(state, offset, prefixes);

  case 0x38: // cmp
  case 0x39:
  case 0x3a:
  case 0x3b:
  case 0x3c:
  case 0x3d:
    return w86_instruction_cmp(state, offset, prefixes);

  case 0x9a: // call
  case 0xe8:
    return w86_instruction_call(state, offset, prefixes);

  case 0xc2: // ret
  case 0xc3:
  case 0xca:
  case 0xcb:
    return w86_instruction_ret(state, offset, prefixes);

  case 0xe9: // jmp
  case 0xea:
  case 0xeb:
    return w86_instruction_jmp(state, offset, prefixes);

  case 0x70: // jo
  case 0x71: // jno
  case 0x72: // jb/jnae/jc
  case 0x73: // jae/jnb/jnc
  case 0x74: // je/jz
  case 0x75: // jne/jnz
  case 0x76: // jbe/jna
  case 0x77: // ja/jnbe
  case 0x78: // js
  case 0x79: // jns
  case 0x7a: // jp/jpe
  case 0x7b: // jnp/jpo
  case 0x7c: // jl/jnge
  case 0x7d: // jge/jnl
  case 0x7e: // jle/jng
  case 0x7f: // jg/jnle
    return w86_instruction_jcc(state, offset, prefixes);

  case 0xf8: // clc
    return w86_instruction_clc(state, offset, prefixes);

  case 0xf5: // cmc
    return w86_instruction_cmc(state, offset, prefixes);

  case 0xf9: // stc
    return w86_instruction_stc(state, offset, prefixes);

  case 0xfa: // cli
    return w86_instruction_cli(state, offset, prefixes);

  case 0xfb: // sti
    return w86_instruction_sti(state, offset, prefixes);

  case 0xfc: // cld
    return w86_instruction_cld(state, offset, prefixes);

  case 0xfd: // std
    return w86_instruction_std(state, offset, prefixes);

  case 0xf4: // hlt
    return w86_instruction_hlt(state, offset, prefixes);

  case 0x80: // immediate instruction group
  case 0x81:
  case 0x82:
  case 0x83:
    switch (w86_get_byte(state, state->registers.cs, offset + 1) >> 3 & 0b111) {
    case 0b000: // add
      return w86_instruction_add(state, offset, prefixes);

    case 0b101: // sub
      return w86_instruction_sub(state, offset, prefixes);

    case 0b111: // cmp
      return w86_instruction_cmp(state, offset, prefixes);

    case 0b001:
    case 0b011:
    case 0b100:
    case 0b110:
      return W86_STATUS_UNIMPLEMENTED_OPCODE;
    }

  case 0xd0: // shift instruction group
  case 0xd1:
  case 0xd2:
  case 0xd3:
    return W86_STATUS_UNIMPLEMENTED_OPCODE;

  case 0xf6: // instruction group 1
  case 0xf7:
    return W86_STATUS_UNIMPLEMENTED_OPCODE;

  case 0xfe: // instruction group 2
  case 0xff:
    switch (w86_get_byte(state, state->registers.cs, offset + 1) >> 3 & 0b111) {
    case 0b000: // inc
      return w86_instruction_inc(state, offset, prefixes);

    case 0b001: // dec
      return w86_instruction_dec(state, offset, prefixes);

    case 0b100: // jmp
    case 0b101:
      return w86_instruction_jmp(state, offset, prefixes);

    case 0b010:
    case 0b011:
    case 0b110:
      return W86_STATUS_UNIMPLEMENTED_OPCODE;

    default:
      return W86_STATUS_UNDEFINED_OPCODE;
    }

  case 0x06:
  case 0x07:
  case 0x08:
  case 0x09:
  case 0x0a:
  case 0x0b:
  case 0x0c:
  case 0x0d:
  case 0x0e:
  case 0x10:
  case 0x11:
  case 0x12:
  case 0x13:
  case 0x14:
  case 0x15:
  case 0x16:
  case 0x17:
  case 0x18:
  case 0x19:
  case 0x1a:
  case 0x1b:
  case 0x1c:
  case 0x1d:
  case 0x1e:
  case 0x1f:
  case 0x20:
  case 0x21:
  case 0x22:
  case 0x23:
  case 0x24:
  case 0x25:
  case 0x26:
  case 0x27:
  case 0x2e:
  case 0x2f:
  case 0x30:
  case 0x31:
  case 0x32:
  case 0x33:
  case 0x34:
  case 0x35:
  case 0x36:
  case 0x37:
  case 0x3e:
  case 0x3f:
  case 0x50:
  case 0x51:
  case 0x52:
  case 0x53:
  case 0x54:
  case 0x55:
  case 0x56:
  case 0x57:
  case 0x58:
  case 0x59:
  case 0x5a:
  case 0x5b:
  case 0x5c:
  case 0x5d:
  case 0x5e:
  case 0x5f:
  case 0x84:
  case 0x85:
  case 0x8d:
  case 0x8f:
  case 0x98:
  case 0x99:
  case 0x9b:
  case 0x9c:
  case 0x9d:
  case 0x9e:
  case 0x9f:
  case 0xa4:
  case 0xa5:
  case 0xa6:
  case 0xa7:
  case 0xa8:
  case 0xa9:
  case 0xaa:
  case 0xab:
  case 0xac:
  case 0xad:
  case 0xae:
  case 0xaf:
  case 0xc4:
  case 0xc5:
  case 0xcc:
  case 0xcd:
  case 0xce:
  case 0xcf:
  case 0xd4:
  case 0xd5:
  case 0xd7:
  case 0xd8:
  case 0xd9:
  case 0xda:
  case 0xdb:
  case 0xdc:
  case 0xdd:
  case 0xde:
  case 0xdf:
  case 0xe0:
  case 0xe1:
  case 0xe2:
  case 0xe3:
  case 0xe4:
  case 0xe5:
  case 0xe6:
  case 0xe7:
  case 0xec:
  case 0xed:
  case 0xee:
  case 0xef:
  case 0xf0:
  case 0xf2:
  case 0xf3:
    return W86_STATUS_UNIMPLEMENTED_OPCODE;

  default:
    return W86_STATUS_UNDEFINED_OPCODE;
  }
}
