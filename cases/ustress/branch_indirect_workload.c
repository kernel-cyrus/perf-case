/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Copyright (C) Arm Ltd. 2022
 */

/*
 * Purpose:
 *   This program aims to stress CPU indirect branch prediction.
 *
 * Theory:
 *   The program uses makes a large number of indirect branches using a
 *   random index into an array of code labels. The branch to each label
 *   is achieved using a computed goto (gcc specific feature). To
 *   prevent the compiler from optimising all the labels down to a single
 *   address then the code at each label performs a read from a volatile
 *   variable (and the resulting sequence of reads for each label is hard
 *   to optimise away). The compution to determin the next destination
 *   label is performed as close as possible to the branch to reduce the
 *   opportunity for the CPU to compute the address well in advance.
 *
 *   It is advisabke to check the disassembly to ensure that the generated
 *   code does contain distinct addresses for each label, and that the
 *   address computation occurs immediately before the branch.
 */

#include <stdint.h>
#include <stdlib.h>

static volatile int v = 0;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
void ustress_branch_indirect(long runs) {
  void* labels[] = {
    &&l0, &&l1, &&l2, &&l3, &&l4, &&l5, &&l6, &&l7,
    &&l8, &&l9, &&l10, &&l11, &&l12, &&l13, &&l14, &&l15,
    &&l16, &&l17, &&l18, &&l19, &&l20, &&l21, &&l22, &&l23,
    &&l24, &&l25, &&l26, &&l27, &&l28, &&l29, &&l30, &&l31
  };
  for(unsigned int mask = 0x1F; mask > 0; mask >>= 1) {
    uint16_t lfsr = 0xACE1u;
    long n=runs; // 3000000
    while(n > 0) {
      l31:  v;
      l30:  v;
      l29:  v;
      l28:  v;
      l27:  v;
      l26:  v;
      l25:  v;
      l24:  v;
      l23:  v;
      l22:  v;
      l21:  v;
      l20:  v;
      l19:  v;
      l18:  v;
      l17:  v;
      l16:  v;
      l15:  v;
      l14:  v;
      l13:  v;
      l12:  v;
      l11:  v;
      l10:  v;
      l9:   v;
      l8:   v;
      l7:   v;
      l6:   v;
      l5:   v;
      l4:   v;
      l3:   v;
      l2:   v;
      l1:   v;
      lfsr = (lfsr >> 1) | ((((lfsr >> 0) ^ (lfsr >> 2) ^ (lfsr >> 3) ^ (lfsr >> 5) ) & 1) << 15);
      goto *labels[lfsr & mask];
      l0:   n--;
    }

    /* This volatile use of result should prevent the above code from being optimised away by the compiler. */
    *((volatile uint16_t*)&lfsr) = lfsr + 1;
  }
}
#pragma GCC diagnostic pop
