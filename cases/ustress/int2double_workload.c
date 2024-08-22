/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Copyright (C) Arm Ltd. 2022
 */

/*
 * Purpose:
 *  This program performs repeated int to double conversions.
 */


void ustress_int2double(long runs) {
  double result = 0;
  for(long n=runs; n>0; n--) {
    result += (double)n;
  }

  /* This volatile use of result should prevent the computation from being optimised away by the compiler. */
  *(volatile double*)&result = result;
}
