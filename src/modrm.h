// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef W86_MODRM_H_
#define W86_MODRM_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "w86.h"

#include <stddef.h>
#include <stdint.h>

#include "decode.h"

enum w86_modrm_mod {
  W86_MODRM_MOD_MEM = 0b00,
  W86_MODRM_MOD_MEM_DISP8 = 0b01,
  W86_MODRM_MOD_MEM_DISP16 = 0b10,
  W86_MODRM_MOD_REG = 0b11
};

enum w86_modrm_reg {
  W86_MODRM_REG_AX = 0b000,
  W86_MODRM_REG_CX = 0b001,
  W86_MODRM_REG_DX = 0b010,
  W86_MODRM_REG_BX = 0b011,
  W86_MODRM_REG_SP = 0b100,
  W86_MODRM_REG_BP = 0b101,
  W86_MODRM_REG_SI = 0b110,
  W86_MODRM_REG_DI = 0b111,

  W86_MODRM_REG_AL = 0b000,
  W86_MODRM_REG_CL = 0b001,
  W86_MODRM_REG_DL = 0b010,
  W86_MODRM_REG_BL = 0b011,
  W86_MODRM_REG_AH = 0b100,
  W86_MODRM_REG_CH = 0b101,
  W86_MODRM_REG_DH = 0b110,
  W86_MODRM_REG_BH = 0b111,

  W86_MODRM_REG_ES = 0b000,
  W86_MODRM_REG_CS = 0b001,
  W86_MODRM_REG_SS = 0b010,
  W86_MODRM_REG_DS = 0b011,
};

enum w86_modrm_mem {
  W86_MODRM_MEM_BX_SI = 0b000,
  W86_MODRM_MEM_BX_DI = 0b001,
  W86_MODRM_MEM_BP_SI = 0b010,
  W86_MODRM_MEM_BP_DI = 0b011,
  W86_MODRM_MEM_SI = 0b100,
  W86_MODRM_MEM_DI = 0b101,
  W86_MODRM_MEM_BP = 0b110,
  W86_MODRM_MEM_BX = 0b111,

  W86_MODRM_MEM_DIRECT = 0b110
};

struct w86_modrm_info {
  enum w86_modrm_mod mod;
  enum w86_modrm_reg reg;
  union {
    enum w86_modrm_reg reg;
    enum w86_modrm_mem mem;
  } rm;
  int16_t disp;
  bool rm_is_reg;
  enum w86_segment_prefix segment;
  uint16_t address;
  size_t size;
};

struct w86_modrm_info w86_modrm_parse(struct w86_cpu_state* state, uint16_t offset, enum w86_segment_prefix segment);

bool w86_modrm_get_reg_byte(struct w86_cpu_state* state, struct w86_modrm_info info, uint8_t* ret);
bool w86_modrm_get_rm_byte(struct w86_cpu_state* state, struct w86_modrm_info info, uint8_t* ret);
bool w86_modrm_set_reg_byte(struct w86_cpu_state* state, struct w86_modrm_info info, uint8_t value);
bool w86_modrm_set_rm_byte(struct w86_cpu_state* state, struct w86_modrm_info info, uint8_t value);
bool w86_modrm_get_reg_word(struct w86_cpu_state* state, struct w86_modrm_info info, uint16_t* ret);
bool w86_modrm_get_rm_word(struct w86_cpu_state* state, struct w86_modrm_info info, uint16_t* ret);
bool w86_modrm_set_reg_word(struct w86_cpu_state* state, struct w86_modrm_info info, uint16_t value);
bool w86_modrm_set_rm_word(struct w86_cpu_state* state, struct w86_modrm_info info, uint16_t value);

bool w86_modrm_byte_load(struct w86_cpu_state* state, struct w86_modrm_info info, uint8_t* ret);
bool w86_modrm_byte_store(struct w86_cpu_state* state, struct w86_modrm_info info, uint8_t* ret);
bool w86_modrm_word_load(struct w86_cpu_state* state, struct w86_modrm_info info, uint16_t* ret);
bool w86_modrm_word_store(struct w86_cpu_state* state, struct w86_modrm_info info, uint16_t* ret);
bool w86_modrm_segment_load(struct w86_cpu_state* state, struct w86_modrm_info info, uint16_t* ret);
bool w86_modrm_segment_store(struct w86_cpu_state* state, struct w86_modrm_info info, uint16_t* ret);

#ifdef __cplusplus
}
#endif

#endif /* W86_MODRM_H_ */
