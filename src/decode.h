// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef W86_DECODE_H_
#define W86_DECODE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "w86.h"

enum w86_segment_prefix {
  W86_SEGMENT_PREFIX_NONE,
  W86_SEGMENT_PREFIX_CS,
  W86_SEGMENT_PREFIX_DS,
  W86_SEGMENT_PREFIX_ES,
  W86_SEGMENT_PREFIX_SS
};

enum w86_repeat_prefix {
  W86_REPEAT_PREFIX_NONE,
  W86_REPEAT_PREFIX_REP,
  W86_REPEAT_PREFIX_REPE = W86_REPEAT_PREFIX_REP,
  W86_REPEAT_PREFIX_REPNE
};

struct w86_instruction_prefixes {
  enum w86_segment_prefix segment;
  enum w86_repeat_prefix repeat;
  bool lock;
};

enum w86_status w86_decode(struct w86_cpu_state*);

#ifdef __cplusplus
}
#endif

#endif /* W86_DECODE_H_ */
