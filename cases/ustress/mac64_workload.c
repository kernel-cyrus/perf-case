/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Copyright (C) Arm Ltd. 2022
 */

/*
 * Purpose:
 *   This program aims to stress CPU with back-to-back 64-bit integer multiply-adds.
 *
 * Theory:
 *   The program performs back-to-back 64-bit integer multiply-adds where the
 *   result of one operation is needed for the next operation.
 */

#include <stdint.h>

static int64_t kernel(long runs, int64_t result, int64_t mul) {
  for(long n=runs; n>0; n--) {
    result += result * mul;
    result += result * mul;
    result += result * mul;
    result += result * mul;
  }
  return result;
}

void ustress_mac64(long runs) {
  /* This volatile use of result should prevent the computation from being optimised away by the compiler. */
  int64_t result;
  volatile int64_t a = 99, b = 457;
  *((volatile int64_t*)&result) = kernel(runs, a, b);
}
