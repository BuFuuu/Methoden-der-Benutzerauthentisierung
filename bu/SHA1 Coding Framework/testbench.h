/* 
 * DO NOT CHANGE ANYTHING HERE
 */

#ifndef TESTBENCH_H
#define TESTBENCH_H

#include <stdlib.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "sha1.h"

#ifdef __i386
__inline__ uint64_t rdtsc() {
  uint64_t x;
  __asm__ volatile ("rdtsc" : "=A" (x));
  return x;
}
#elif __amd64
__inline__ uint64_t rdtsc() {
  uint64_t a, d;
  __asm__ volatile ("rdtsc" : "=a" (a), "=d" (d));
  return (d<<32) | a;
}
#endif

//TESTBENCH_H
#endif 