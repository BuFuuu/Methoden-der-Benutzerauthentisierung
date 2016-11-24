/* 
 * DO NOT CHANGE ANYTHING HERE
 */

#ifndef SHA1_H
#define SHA1_H

#include <inttypes.h>
#include <stdlib.h>

struct state {
	uint32_t a;
	uint32_t b;
	uint32_t c;
	uint32_t d;
	uint32_t e;
};

/*
 * PROTOTYPES
 */
int crackHash(struct state hash, char *result);




//SHA1_H
#endif