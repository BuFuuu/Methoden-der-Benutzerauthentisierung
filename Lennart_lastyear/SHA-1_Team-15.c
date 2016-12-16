#include "sha1.h"
#include <stdio.h>
#include <string.h>
#include <emmintrin.h>

/**
 * SHA1 PASSWORD CRACKING
 * \author Patrick Geisler <patrick.geisler-a85@rub.de>
 * \author Lennart Lorenz <lennart.lorenz@rub.de>
 *
 * PLEASE SUPPLY YOUR IMPLEMENTATION HERE
 * AND RETURN THE STRING THAT WAS FOUND BY
 * YOUR IMPLEMENTATION
 * 
 * THE WHOLE PROJECT MUST COMPILE WITH: 
 * gcc -O2 -Wall -fomit-frame-pointer -msse2 -masm=intel testbench.c <YOUR IMPL>.c -o crackSHA1
 * 
 * ANY FURTHER INFORMATION CAN BE FOUND IN THE
 * PROBLEM DESCRIPTION
 */



/* f-functions for different sha1 rounds
 * note that alternative f-functions are used */

#define F0(x,y,z) _mm_or_si128 (_mm_and_si128 (x, y), _mm_andnot_si128 (x, z))
#define F20(x,y,z) _mm_xor_si128 (z, _mm_xor_si128 (x, y))
#define F40(x,y,z) _mm_or_si128 (_mm_and_si128 (z, _mm_or_si128 (x, y)), _mm_and_si128 (x, y))
#define F60(x,y,z) _mm_xor_si128 (z, _mm_xor_si128 (x, y))

/* constants used in different sha1 rounds */

#define ASHA1_K0  0x5a827999
#define ASHA1_K20 0x6ed9eba1
#define ASHA1_K40 0x8f1bbcdc
#define ASHA1_K60 0xca62c1d6

/* fixed pwd length so W[15] is const */

#define W15 _mm_set1_epi32(0x30)

/* constant precomputed values due to const W[15] (see above)
 * if W[15] werent const calculations would have been done as shown below
 * PW[18] = ROTATE_LEFT(( W[15]), 1);
 * PW[21] = ROTATE_LEFT(( PW[18]), 1);
 * PW[24] = ROTATE_LEFT(( PW[21]), 1);
 * PW[27] = ROTATE_LEFT(( PW[24]), 1);
 * PW[30] = ROTATE_LEFT(( PW[27]), 1); */

#define PW18 _mm_set1_epi32(0x60)
#define PW21 _mm_set1_epi32(0xc0)
#define PW24 _mm_set1_epi32(0x180)
#define PW27 _mm_set1_epi32(0x300)
#define PW30 _mm_set1_epi32(0x600)

/* sha1 initial constants  
 * we also precompute the first sha1 steps for H0, H1, H4
 * H_0 = ROTL (H0, 30)
 * H_1 = ROTL (H1, 30)
 * H_4 = ROTL (H0, 5)+F0(H1,H2,H3)+ASHA1_K0 */

#define H0 0x67452301
#define H1 0xefcdab89
#define H2 0x98badcfe
#define H3 0x10325476
#define H4 0xc3d2e1f0

#define H_0 0x59d148c0
#define H_1 0x7bf36ae2
#define H_4 0x9fb498b3

/* macro for bitwise rotation */

#define ROTL32_SSE(x, n) _mm_or_si128 (_mm_slli_epi32 ((x), (n)), _mm_srli_epi32 ((x), (32 - (n))))
#define ROTATE_RIGHT(x,n) (((x) >> (n)) | ((x) << (32-(n))))

/* macro of one sha1 step 
 * it essentially does the same like the schoolbook implementation
 * without the unnecesserary variable assignments on eachother.
 * each round it is called with shuffled variables instead
 * so we yield the same values in the end */

#define SHA1_ROUND(a,b,c,d,e,f,K,W)                  \
{                                                 \
  e = _mm_add_epi32 (e, K);                       \
  e = _mm_add_epi32 (e, W);                       \
  e = _mm_add_epi32 (e, f (b, c, d));             \
  e = _mm_add_epi32 (e, ROTL32_SSE(a, 5));        \
  b = ROTL32_SSE    (b, 30);                      \
}

/* same macro but used if W[i] == 0 */

#define SHA1_ROUND_NO_W(a,b,c,d,e,f,K)                  \
{                                                 \
  e = _mm_add_epi32 (e, K);                       \
  e = _mm_add_epi32 (e, f (b, c, d));             \
  e = _mm_add_epi32 (e, ROTL32_SSE(a, 5));        \
  b = ROTL32_SSE    (b, 30);                      \
}

#define XOR(x,y) ( _mm_xor_si128(x,y))

static inline int testEqual (__m128i a, __m128i b) {
	return _mm_movemask_epi8(_mm_cmpeq_epi32(a, b));
}

/* hashes one block (in our case all we need because of fixed pwd length of 6 chars
 * inspired by Jens Steube first calculate W0_1-W020
 * sha1 loops unrolled using macro described above
 * uses early exit (see below) to leave after step 75 */

static inline int sha1_hashBlock(__m128i* W, __m128i* dgst, int * res) {
	__m128i W0_1 = ROTL32_SSE(W[0],  1);
	__m128i W0_2 = ROTL32_SSE(W[0],  2);
	__m128i W0_3 = ROTL32_SSE(W[0],  3);
	__m128i W0_4 = ROTL32_SSE(W[0],  4);
	__m128i W0_5 = ROTL32_SSE(W[0],  5);
	__m128i W0_6 = ROTL32_SSE(W[0],  6);
	__m128i W0_7 = ROTL32_SSE(W[0],  7);
	__m128i W0_8 = ROTL32_SSE(W[0],  8);
	__m128i W0_9 = ROTL32_SSE(W[0],  9);
	__m128i W010 = ROTL32_SSE(W[0],  10);
	__m128i W011 = ROTL32_SSE(W[0],  11);
	__m128i W012 = ROTL32_SSE(W[0],  12);
	__m128i W013 = ROTL32_SSE(W[0],  13);
	__m128i W014 = ROTL32_SSE(W[0],  14);
	__m128i W015 = ROTL32_SSE(W[0],  15);
	__m128i W016 = ROTL32_SSE(W[0],  16);
	__m128i W017 = ROTL32_SSE(W[0],  17);
	__m128i W018 = ROTL32_SSE(W[0],  18);
	__m128i W019 = ROTL32_SSE(W[0],  19);
	__m128i W020 = ROTL32_SSE(W[0],  20);

	/* define constants for xor-operations occuring more than once
	 * in the following SHA1_ROUND macros to save some instructions */

	const __m128i W0_6___W0_4 = W0_6 ^ W0_4;
	const __m128i W0_8___W0_4 = W0_8 ^ W0_4;
	const __m128i W0_6___W0_4___W0_7 = W0_6___W0_4 ^ W0_7;
	const __m128i W0_7___W012 = W0_7 ^ W012;
	const __m128i W0_8___W012 = W0_8 ^ W012;
	const __m128i W012___W016 = W012 ^ W016;

	/* set initial sha1 values 
	 * use precomputed values where possible */
	
	__m128i a = _mm_set1_epi32 (H_0);
	__m128i b = _mm_set1_epi32 (H_1);
	__m128i c = _mm_set1_epi32 (H2);
	__m128i d = _mm_set1_epi32 (H3);
	__m128i e = _mm_set1_epi32 (H_4);

	/* set constant for first 20 rounds of sha1 */

	__m128i SHA1_K = _mm_set1_epi32 (ASHA1_K0);

	/* instead of
	 * SHA1_ROUND( a, b, c, d, e, F0, SHA1_K, W[0] );
	 * use precomputed constant and only add W[0] */

	e = _mm_add_epi32 (e, W[0]);

	/* analogous: instructions instead of
	 * SHA1_ROUND( e, a, b, c, d, F0, SHA1_K, W[1] ); 
	 * W[1] contains precomputed F0(a,ROTL(b,30),c)+SHA1_K+W[1] 
	 * which is only computed when W[1] changes */

	d = _mm_add_epi32 (d, W[1]);
	d = _mm_add_epi32 (d, ROTL32_SSE(e, 5));

	/* for the next rounds we can skip adding W[i] because W[2]-W[14] := 0 */ 	

	SHA1_ROUND_NO_W( d, e, a, b, c, F0, SHA1_K);
	SHA1_ROUND_NO_W( c, d, e, a, b, F0, SHA1_K);
	SHA1_ROUND_NO_W( b, c, d, e, a, F0, SHA1_K);
	SHA1_ROUND_NO_W( a, b, c, d, e, F0, SHA1_K);
	SHA1_ROUND_NO_W( e, a, b, c, d, F0, SHA1_K);
	SHA1_ROUND_NO_W( d, e, a, b, c, F0, SHA1_K);
	SHA1_ROUND_NO_W( c, d, e, a, b, F0, SHA1_K);
	SHA1_ROUND_NO_W( b, c, d, e, a, F0, SHA1_K);
	SHA1_ROUND_NO_W( a, b, c, d, e, F0, SHA1_K);
	SHA1_ROUND_NO_W( e, a, b, c, d, F0, SHA1_K);
	SHA1_ROUND_NO_W( d, e, a, b, c, F0, SHA1_K);
	SHA1_ROUND_NO_W( c, d, e, a, b, F0, SHA1_K);
	SHA1_ROUND_NO_W( b, c, d, e, a, F0, SHA1_K);

	/* wherever possible (due to nature of password structure like fixed length 
	 * etc) we use defined constants and precomputed values */	

	SHA1_ROUND( a, b, c, d, e, F0, SHA1_K, W15 );
	SHA1_ROUND( e, a, b, c, d, F0, SHA1_K, W0_1 );
	SHA1_ROUND( d, e, a, b, c, F0, SHA1_K, W[17] );
	SHA1_ROUND( c, d, e, a, b, F0, SHA1_K, PW18 );
	SHA1_ROUND( b, c, d, e, a, F0, SHA1_K, W0_2 );

	SHA1_K = _mm_set1_epi32 (ASHA1_K20);
	SHA1_ROUND( a, b, c, d, e, F20, SHA1_K, W[20] );
	SHA1_ROUND( e, a, b, c, d, F20, SHA1_K, PW21 );
	SHA1_ROUND( d, e, a, b, c, F20, SHA1_K, W0_3 );
	SHA1_ROUND( c, d, e, a, b, F20, SHA1_K, W[23] );
	SHA1_ROUND( b, c, d, e, a, F20, SHA1_K, PW24 ^ W0_2 );
	SHA1_ROUND( a, b, c, d, e, F20, SHA1_K, W[20] ^ W0_4 );
	SHA1_ROUND( e, a, b, c, d, F20, SHA1_K, W[26] );
	SHA1_ROUND( d, e, a, b, c, F20, SHA1_K, PW27 );
	SHA1_ROUND( c, d, e, a, b, F20, SHA1_K, W0_5 );
	SHA1_ROUND( b, c, d, e, a, F20, SHA1_K, W[29] );
	SHA1_ROUND( a, b, c, d, e, F20, SHA1_K, PW30 ^ W0_4 ^ W0_2 );
	SHA1_ROUND( e, a, b, c, d, F20, SHA1_K, W[31] ^ W0_6 );
	SHA1_ROUND( d, e, a, b, c, F20, SHA1_K, W[32] ^ W0_3 ^ W0_2 );
	SHA1_ROUND( c, d, e, a, b, F20, SHA1_K, W[33] );
	SHA1_ROUND( b, c, d, e, a, F20, SHA1_K, W[34] ^ W0_7 );
	SHA1_ROUND( a, b, c, d, e, F20, SHA1_K, W[35] ^ W0_4 );
	SHA1_ROUND( e, a, b, c, d, F20, SHA1_K, W[36] ^ W0_6___W0_4 );
	SHA1_ROUND( d, e, a, b, c, F20, SHA1_K, W[37] ^ W0_8 );
	SHA1_ROUND( c, d, e, a, b, F20, SHA1_K, W[38] ^ W0_4 );
	SHA1_ROUND( b, c, d, e, a, F20, SHA1_K, W[39] );

	SHA1_K = _mm_set1_epi32 (ASHA1_K40);
	SHA1_ROUND( a, b, c, d, e, F40, SHA1_K, W[40] ^ W0_4 ^ W0_9 );
	SHA1_ROUND( e, a, b, c, d, F40, SHA1_K, W[41] );
	SHA1_ROUND( d, e, a, b, c, F40, SHA1_K, W[42] ^ W0_6 ^ W0_8 );
	SHA1_ROUND( c, d, e, a, b, F40, SHA1_K, W[43] ^ W010 );
	SHA1_ROUND( b, c, d, e, a, F40, SHA1_K, W[44] ^ W0_6 ^ W0_3 ^ W0_7 );
	SHA1_ROUND( a, b, c, d, e, F40, SHA1_K, W[45] );
	SHA1_ROUND( e, a, b, c, d, F40, SHA1_K, W[46] ^ W0_4 ^ W011 );
	SHA1_ROUND( d, e, a, b, c, F40, SHA1_K, W[47] ^ W0_8___W0_4 );
	SHA1_ROUND( c, d, e, a, b, F40, SHA1_K, W[48] ^ W0_8___W0_4 ^ W0_3 ^ W010 ^ W0_5 );
	SHA1_ROUND( b, c, d, e, a, F40, SHA1_K, W[49] ^ W012 );
	SHA1_ROUND( a, b, c, d, e, F40, SHA1_K, W[50] ^ W0_8 );
	SHA1_ROUND( e, a, b, c, d, F40, SHA1_K, W[51] ^ W0_6___W0_4 );
	SHA1_ROUND( d, e, a, b, c, F40, SHA1_K, W[52] ^ W0_8___W0_4 ^ W013 );
	SHA1_ROUND( c, d, e, a, b, F40, SHA1_K, W[53] );
	SHA1_ROUND( b, c, d, e, a, F40, SHA1_K, W[54] ^ W0_7___W012 ^ W010 );
	SHA1_ROUND( a, b, c, d, e, F40, SHA1_K, W[55] ^ W014 );
	SHA1_ROUND( e, a, b, c, d, F40, SHA1_K, W[56] ^ W0_6___W0_4___W0_7 ^ W011 ^ W010 );
	SHA1_ROUND( d, e, a, b, c, F40, SHA1_K, W[57] ^ W0_8 );
	SHA1_ROUND( c, d, e, a, b, F40, SHA1_K, W[58] ^ W0_8___W0_4 ^ W015 );
	SHA1_ROUND( b, c, d, e, a, F40, SHA1_K, W[59] ^ W0_8___W012 );

	SHA1_K = _mm_set1_epi32 (ASHA1_K60);
	SHA1_ROUND( a, b, c, d, e, F60, SHA1_K, W[60] ^ W0_7___W012 ^ W0_8___W0_4 ^ W014 );
	SHA1_ROUND( e, a, b, c, d, F60, SHA1_K, W[61] ^ W016 );
	SHA1_ROUND( d, e, a, b, c, F60, SHA1_K, W[62] ^ W0_6___W0_4 ^ W0_8___W012 );
	SHA1_ROUND( c, d, e, a, b, F60, SHA1_K, W[63] ^ W0_8 );
	SHA1_ROUND( b, c, d, e, a, F60, SHA1_K, W[64] ^ W0_6___W0_4___W0_7 ^ W0_8___W012 ^ 		W017 );
	SHA1_ROUND( a, b, c, d, e, F60, SHA1_K, W[65] );
	SHA1_ROUND( e, a, b, c, d, F60, SHA1_K, W[66] ^ W014 ^ W016 );
	SHA1_ROUND( d, e, a, b, c, F60, SHA1_K, W[67] ^ W0_8 ^ W018 );
	SHA1_ROUND( c, d, e, a, b, F60, SHA1_K, W[68] ^ W011 ^ W014 ^ W015 );
	SHA1_ROUND( b, c, d, e, a, F60, SHA1_K, W[69] );
	SHA1_ROUND( a, b, c, d, e, F60, SHA1_K, W[70] ^ W012 ^ W019 );
	SHA1_ROUND( e, a, b, c, d, F60, SHA1_K, W[71] ^ W012___W016 );
	SHA1_ROUND( d, e, a, b, c, F60, SHA1_K, W[72] ^ W012___W016 ^ W011 ^ W018 ^ W013 ^ 		W0_5 );
	SHA1_ROUND( c, d, e, a, b, F60, SHA1_K, W[73] ^ W020 );
	SHA1_ROUND( b, c, d, e, a, F60, SHA1_K, W[74] ^ W0_8 ^ W016 );
	SHA1_ROUND( a, b, c, d, e, F60, SHA1_K, W[75] ^ W0_6 ^ W012 ^ W014 );

	/* early exit if part e of hash (last part) doesnt match
	 * if it does do another round and check next part
	 * analogous for remaining parts of hash and sha1 rounds
	 * we can exit this early because we pre-reversed hash->e 
	 * by ROTATE_RIGHT(hash.e-H4,30) and know how e should look like at this point  */

	if (!testEqual(e,dgst[4])) return 0;

	/* if we didn't exit we have a high probability of having the right pwd
	 * so call function testEqual() again to find out wich pwd candidate was the right one
	 * we mostly exit early and don't get this far so we didn't save the
	 * result of testEqual() at the expense of having to call it again now */

	else *res = testEqual(e,dgst[4]);

	/* calculate only needed W[i]'s (has partly been done already above in SHA1_ROUNDs
	 * but we assume to early exit most of the time and save some allocations
	 * W0xx values appearing more than once were eliminated (xoring themselfs out) */

	W[76] = ROTL32_SSE(( W[73] ^ W020 ^ W[68] ^ W011 ^ W015 ^ W[62] ^ W0_6 ^ W[60] ^ 		W0_7), 1);
	SHA1_ROUND( e, a, b, c, d, F60, SHA1_K, W [76] );
	if (!testEqual(d,dgst[3])) return 0;
	else *res = *res & testEqual(d,dgst[3]);
	SHA1_ROUND( d, e, a, b, c, F60, SHA1_K, ROTL32_SSE(( W[74] ^ W[69] ^ W[63] ^ 	W[61]), 1) );
	if (!testEqual(c,dgst[2])) return 0;
	else *res = *res & testEqual(c,dgst[2]);
	SHA1_ROUND( c, d, e, a, b, F60, SHA1_K, ROTL32_SSE(( W[75] ^ W0_6 ^ W014 ^ W[70] ^ 		W019 ^ W[64] ^ W0_7 ^ W017 ^ W[62]), 1) );
	if (!testEqual(b,dgst[1])) return 0;
	else *res = *res & testEqual(b,dgst[1]);
	SHA1_ROUND( b, c, d, e, a, F60, SHA1_K, ROTL32_SSE(( W[76] ^ W[71] ^ W012___W016 ^ 		W[65] ^ W[63] ^ W0_8), 1) );
	//if (!testEqual(a,dgst[0])) return 0;
	return (*res & testEqual(a,dgst[0]));
}



/* outer loop which does most parts of sha1 word expansion
 * and is only called if W[1] changes.
 * for changes in W[0] we don't need to recalculate all W[2]-W[79]
 * instead we calculate those values as far as possible for a given W[1]
 * and apply the missing W[0] at a later stage 
 * (possible because of structure of word expansion [xor])
 * see "Word-Expansion" exploit from Jens Steube */

/* some optimizations could be made too because in our case
 * W[2]-W[14] := 0 and W[15] := const
 * W[i] with value 0 were removed and constants defined for W[i] not changing */  

void precompute_outer(__m128i* W) {
	W[17] = ROTL32_SSE(( W[1]), 1);
	W[20] = ROTL32_SSE(( W[17]), 1);
	W[23] = ROTL32_SSE(( W[20] ^ W15), 1);

	/* W[25] == W[20] 
	 * every occurence of W[25] is replaced with W[20] */

	W[26] = ROTL32_SSE(( W[23] ^ PW18), 1);

	W[29] = ROTL32_SSE(( W[26] ^ PW21 ^ W15), 1);
	W[31] = ROTL32_SSE(( W[23] ^ W[17] ^ W15), 1);
	W[32] = ROTL32_SSE(( W[29] ^ PW24 ^ PW18), 1);
	W[33] = ROTL32_SSE(( PW30 ^ W[20] ^ W[17]), 1);
	
	/* seems to be const: W[34] := 0x180 for all W[1], W[i]
	 * not used here because of missing proof
	 * (though structure implies it) */
	
	W[34] = ROTL32_SSE(( W[31] ^ W[26] ^ W[20] ^ PW18), 1);

	W[35] = ROTL32_SSE(( W[32] ^ PW27 ^ PW21), 1);
	W[36] = ROTL32_SSE(( W[33] ^ W[20]), 1);
	W[37] = ROTL32_SSE(( W[34] ^ W[29] ^ W[23] ^ PW21), 1);
	W[38] = ROTL32_SSE(( W[35] ^ PW30 ^ PW24), 1);
	W[39] = ROTL32_SSE(( W[36] ^ W[31] ^ W[20] ^ W[23]), 1);
	W[40] = ROTL32_SSE(( W[37] ^ W[32] ^ W[26] ^ PW24), 1);
	W[41] = ROTL32_SSE(( W[38] ^ W[33] ^ PW27 ^ W[20]), 1);
	W[42] = ROTL32_SSE(( W[39] ^ W[34] ^ W[26]), 1);
	W[43] = ROTL32_SSE(( W[40] ^ W[35] ^ W[29] ^ PW27), 1);
	W[44] = ROTL32_SSE(( W[41] ^ W[36] ^ PW30), 1);
	W[45] = ROTL32_SSE(( W[42] ^ W[37] ^ W[31] ^ W[29]), 1);
	W[46] = ROTL32_SSE(( W[43] ^ W[38] ^ W[32] ^ PW30), 1);
	W[47] = ROTL32_SSE(( W[44] ^ W[39] ^ W[33] ^ W[31]), 1);
	W[48] = ROTL32_SSE(( W[45] ^ W[40] ^ W[34] ^ W[32]), 1);
	W[49] = ROTL32_SSE(( W[46] ^ W[41] ^ W[35] ^ W[33]), 1);
	W[50] = ROTL32_SSE(( W[47] ^ W[42] ^ W[36] ^ W[34]), 1);
	W[51] = ROTL32_SSE(( W[48] ^ W[43] ^ W[37] ^ W[35]), 1);
	W[52] = ROTL32_SSE(( W[49] ^ W[44] ^ W[38] ^ W[36]), 1);
	W[53] = ROTL32_SSE(( W[50] ^ W[45] ^ W[39] ^ W[37]), 1);
	W[54] = ROTL32_SSE(( W[51] ^ W[46] ^ W[40] ^ W[38]), 1);
	W[55] = ROTL32_SSE(( W[52] ^ W[47] ^ W[41] ^ W[39]), 1);
	W[56] = ROTL32_SSE(( W[53] ^ W[48] ^ W[42] ^ W[40]), 1);
	W[57] = ROTL32_SSE(( W[54] ^ W[49] ^ W[43] ^ W[41]), 1);
	W[58] = ROTL32_SSE(( W[55] ^ W[50] ^ W[44] ^ W[42]), 1);
	W[59] = ROTL32_SSE(( W[56] ^ W[51] ^ W[45] ^ W[43]), 1);
	W[60] = ROTL32_SSE(( W[57] ^ W[52] ^ W[46] ^ W[44]), 1);
	W[61] = ROTL32_SSE(( W[58] ^ W[53] ^ W[47] ^ W[45]), 1);
	W[62] = ROTL32_SSE(( W[59] ^ W[54] ^ W[48] ^ W[46]), 1);
	W[63] = ROTL32_SSE(( W[60] ^ W[55] ^ W[49] ^ W[47]), 1);
	W[64] = ROTL32_SSE(( W[61] ^ W[56] ^ W[50] ^ W[48]), 1);
	W[65] = ROTL32_SSE(( W[62] ^ W[57] ^ W[51] ^ W[49]), 1);
	W[66] = ROTL32_SSE(( W[63] ^ W[58] ^ W[52] ^ W[50]), 1);
	W[67] = ROTL32_SSE(( W[64] ^ W[59] ^ W[53] ^ W[51]), 1);
	W[68] = ROTL32_SSE(( W[65] ^ W[60] ^ W[54] ^ W[52]), 1);
	W[69] = ROTL32_SSE(( W[66] ^ W[61] ^ W[55] ^ W[53]), 1);
	W[70] = ROTL32_SSE(( W[67] ^ W[62] ^ W[56] ^ W[54]), 1);
	W[71] = ROTL32_SSE(( W[68] ^ W[63] ^ W[57] ^ W[55]), 1);
	W[72] = ROTL32_SSE(( W[69] ^ W[64] ^ W[58] ^ W[56]), 1);
	W[73] = ROTL32_SSE(( W[70] ^ W[65] ^ W[59] ^ W[57]), 1);
	W[74] = ROTL32_SSE(( W[71] ^ W[66] ^ W[60] ^ W[58]), 1);
	W[75] = ROTL32_SSE(( W[72] ^ W[67] ^ W[61] ^ W[59]), 1);
}

int crackHash(struct state hash, char *result) {
	/* variables:
	 * res -> saves which one of the four pwd_candidates
	 * tested in parallel is the correctone
	 * dgst -> contains the reversed parts of the hash
	 * W -> holds the array used during sha1
	 * ctr_pwd -> used to count when its time to call hashBlock
	 * (every fourth iteration)
	 * pwd -> used to save W[0], W[1] in uint32_t format
	 * p_pwd -> saves the four pwd candidates
	 * bPtrW -> pointer used for manipulation of single chars in pwd */

	int res[1];
	__m128i dgst[5];
	__m128i W[80];
	int ctr_pwd = 3;
	uint32_t pwd[2];
	uint32_t p_pwd[4];
	uint8_t * const bPtrW = (uint8_t*) pwd;

	/* reverse parts of hash as far as possible by removing last addition
	 * and counter ROTATE_LEFT from sha1 step by rotating right
	 * using this values, we can early exit */

	dgst[0] = _mm_set1_epi32(hash.a-H0);
	dgst[1] = _mm_set1_epi32(hash.b-H1);
	dgst[2] = _mm_set1_epi32(ROTATE_RIGHT(hash.c-H2,30));
	dgst[3] = _mm_set1_epi32(ROTATE_RIGHT(hash.d-H3,30));
	dgst[4] = _mm_set1_epi32(ROTATE_RIGHT(hash.e-H4,30));
	
	/* we assume we're on litte endian for the use of our byte ptr
	 * use 0x80 == 1000 0000 to set first bit = 1 after pwd (according to sha1 specs) */

	bPtrW[4] = 0x00;
	bPtrW[5] = 0x80;

	/* generate all pwds with length 6 in range a-z
	 * whenever W[1] is changed (which is only done after all 
	 * permutations of W[0] for each W[1] have been generated)
	 * precompute_outer() is called to complete word expansion phase as far as possible
	 * sha1_hashBlock() is called for each permutation of the six chars to calculate hash 		 */

	/* set length to 0x30 (in W[15]) because we always have exactly 6 chars */

	W[15] = W15;

	/* naive approach to generate all 6 chars pwds with using pointer which
	 * which manipulate one char each 
	 * W[1] aka pwd[1] is fixed as long as possible and only changed after all
	 * possible permutations with repetition of W[0] for a given W[1] were generated */

	for (bPtrW[6]=0x61; bPtrW[6]<=0x7a; ++bPtrW[6]) {
		for (bPtrW[7]=0x61; bPtrW[7]<=0x7a; ++bPtrW[7]) {		
			W[1] = _mm_set1_epi32(pwd[1]);

			/* we have changed W[1] so precompute now */

			precompute_outer(W);

			/* also precompute W[1]' = F0(a,ROTL(b,30),c)+SHA1_K+W[1] for less
			 * computations in each sha1 hashBlock */
			W[1] = _mm_add_epi32(_mm_set1_epi32(0x567e7897),W[1]);
			for (bPtrW[0]=0x61; bPtrW[0]<=0x7a; ++bPtrW[0]) {
				for (bPtrW[1]=0x61; bPtrW[1]<=0x7a; ++bPtrW[1]) {
					for (bPtrW[2]=0x61; bPtrW[2]<=0x7a; ++bPtrW[2]) {
						for (bPtrW[3]=0x61; bPtrW[3]<=0x7a; ++bPtrW[3]) {				
							p_pwd[ctr_pwd] = pwd[0];			
							if (ctr_pwd!=0) {
								ctr_pwd--;
							}
							else {	
								W[0] = _mm_set_epi32(p_pwd[0], p_pwd[1], p_pwd[2], p_pwd[3]);
								if (sha1_hashBlock(W, 									dgst, res)) {
									/* Found*/
									printf("%x\n",*res);
printf("%x %x %x %x\n",p_pwd[0],p_pwd[1],p_pwd[2],p_pwd[3]);
									if (*res == 0xf) *result = p_pwd[0]; 
									else if (*res == 0xf0) *result = p_pwd[1];
									else if (*res == 0xf00) *result = p_pwd[2];
									else {
result[3] = p_pwd[0] & 0xff;
result[2] = (p_pwd[0]>>8)  & 0xff;
result[1] = (p_pwd[0]>>16) & 0xff;
result[0] = (p_pwd[0]>>24) & 0xff;
}
	    								result[5] = bPtrW[6];
									result[4] = bPtrW[7];
									return(EXIT_SUCCESS);
								}
								ctr_pwd = 3;
							}
							
						}
					}
				}
			}
		}
	}
    /* Not found */
    return(EXIT_FAILURE);
}
