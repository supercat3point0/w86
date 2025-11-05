// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef W86_W86_H
#define W86_W86_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define W86_ADDRESS_SIZE 20
#define W86_REAL_SEGMENT_SIZE 16
#define W86_REAL_POINTER_SIZE 16
#define W86_BOUND_ADDRESS(address) ((address) % (1 << W86_ADDRESS_SIZE))
#define W86_REAL_ADDRESS(segment, pointer) W86_BOUND_ADDRESS(((segment) % (1 << W86_REAL_SEGMENT_SIZE) << 4) + (pointer) % (1 << W86_REAL_POINTER_SIZE))
#define W86_GET_BYTE(array, index) ((uint8_t) (array)[W86_BOUND_ADDRESS(index)])
#define W86_GET_WORD(array, index) ((uint16_t) ((array)[W86_BOUND_ADDRESS(index)] | (array)[W86_BOUND_ADDRESS((index) + 1)] << 8))

struct w86_register_file {
  uint16_t ax;
  uint16_t bx;
  uint16_t cx;
  uint16_t dx;
  uint16_t si;
  uint16_t di;
  uint16_t sp;
  uint16_t bp;
  uint16_t cs;
  uint16_t ds;
  uint16_t es;
  uint16_t ss;
  uint16_t ip;
  uint16_t flags;
};

struct w86_cpu_state {
  struct w86_register_file registers;
#ifdef EMBIND // embind doesn't support pointers to primitive types, so we have cheat a little
  intptr_t memory;
#else
  uint8_t* memory;
#endif
};

enum w86_status {
  W86_STATUS_SUCCESS,
  W86_STATUS_UNKNOWN_ERROR,
  W86_STATUS_UNDEFINED_OPCODE,
  W86_STATUS_UNIMPLEMENTED_OPCODE,
  W86_STATUS_INVALID_OPCODE
};

enum w86_status w86_cpu_step(struct w86_cpu_state*);

#ifdef __cplusplus
}
#endif

#endif /* W86_W86_H */
