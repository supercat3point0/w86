// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef W86_INSTRUCTION_H
#define W86_INSTRUCTION_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

#include "decode.h"
#include "w86.h"

typedef enum w86_status w86_instruction(struct w86_cpu_state*, size_t, struct w86_instruction_prefixes);

// these are functions
w86_instruction w86_instruction_mov;
w86_instruction w86_instruction_jmp;

#ifdef __cplusplus
}
#endif

#endif /* W86_INSTRUCTION_H */
