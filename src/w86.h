// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef W86_H
#define W86_H

#ifdef __cplusplus
extern "C" {
#endif

struct multiplication {
  int x;
  int y;
  int z;
};

struct multiplication* multiply(struct multiplication*);

#ifdef __cplusplus
}
#endif

#endif /* W86_H */
