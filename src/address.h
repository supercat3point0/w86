// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef W86_ADDRESS_H_
#define W86_ADDRESS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "w86.h"

#define W86_ADDRESS_SIZE 20
#define W86_REAL_SEGMENT_SIZE 16
#define W86_REAL_POINTER_SIZE 16
#define W86_BOUND_ADDRESS(address) ((address) % (1 << W86_ADDRESS_SIZE))
#define W86_REAL_ADDRESS(segment, pointer) W86_BOUND_ADDRESS(((segment) % (1 << W86_REAL_SEGMENT_SIZE) << 4) + (pointer) % (1 << W86_REAL_POINTER_SIZE))

uint8_t w86_get_byte(struct w86_cpu_state*, uint16_t, uint16_t);
void w86_set_byte(struct w86_cpu_state*, uint16_t, uint16_t, uint8_t);
uint16_t w86_get_word(struct w86_cpu_state*, uint16_t, uint16_t);
void w86_set_word(struct w86_cpu_state*, uint16_t, uint16_t, uint16_t);

#ifdef __cplusplus
}
#endif

#endif /* W86_ADDRESS_H_ */
