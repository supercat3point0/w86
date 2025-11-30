// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef W86_INSTRUCTION_H_
#define W86_INSTRUCTION_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "decode.h"
#include "w86.h"

typedef enum w86_status w86_instruction(struct w86_cpu_state* state, uint16_t offset, struct w86_instruction_prefixes prefixes);

// these are functions
w86_instruction w86_instruction_mov;
w86_instruction w86_instruction_xchg;

w86_instruction w86_instruction_add;
w86_instruction w86_instruction_inc;
w86_instruction w86_instruction_sub;
w86_instruction w86_instruction_dec;
w86_instruction w86_instruction_cmp;

w86_instruction w86_instruction_call;
w86_instruction w86_instruction_ret;
w86_instruction w86_instruction_jmp;
w86_instruction w86_instruction_jcc;

#ifdef __cplusplus
}
#endif

#endif /* W86_INSTRUCTION_H_ */
