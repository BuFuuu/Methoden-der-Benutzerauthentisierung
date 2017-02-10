#include "sha1.h"
#include <emmintrin.h>

/**
 * THIS IS A TESTBENCH FOR SHA1 PASSWORD CRACKING
 * \author Fabian Kammel <fabian.kammel@rub.de>
 * \author Maximilian Golla <maximilian.golla@rub.de>
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

// Define Constants
#define H_0 0x67452301
#define H_1 0xefcdab89
#define H_2 0x98badcfe
#define H_3 0x10325476
#define H_4 0xc3d2e1f0

#define K_1 0x5a827999
#define K_2 0x6ed9eba1
#define K_3 0x8f1bbcdc
#define K_4 0xca62c1d6

// Define macros for rotations
#define ROTATE_LEFT128(x, n) _mm_or_si128 (_mm_slli_epi32 ((x), (n)), _mm_srli_epi32 ((x), (32 - (n))))
#define ROTATE_LEFT(x,n) (((x) << (n)) | ((x) >> (32-(n))))
#define ROTATE_RIGHT(x,n) (((x) >> (n)) | ((x) << (32-(n))))

// Define macros for loop unrolling
#define FCALC_1(b, c, d) _mm_or_si128(_mm_and_si128(b,c), _mm_andnot_si128(b,d))
#define FCALC_2_4(b, c, d) _mm_xor_si128(_mm_xor_si128(b,c), d)
#define FCALC_3(b, c, d) _mm_or_si128(_mm_or_si128(_mm_and_si128(b,c), _mm_and_si128(b,d)),_mm_and_si128(c,d))
#define MAIN_LOOP(K, i, a, c, d, e, f) \
{ \
    tmp = _mm_add_epi32(_mm_add_epi32(_mm_add_epi32(_mm_add_epi32(ROTATE_LEFT128(a, 5), f), e), K), ws[i]); \
    e = d; \
    d = c; \
    c = _mm_or_si128(_mm_slli_epi32(b, 30), _mm_srli_epi32(b, 2)); \
    b = a; \
    a = tmp; \
    i++; \
}
#define MAIN_LOOP_1(K, i, a, b, c, d, e) MAIN_LOOP(K, i, a, c, d, e, FCALC_1(b, c, d))
#define MAIN_LOOP_2_4(K, i, a, b, c, d, e) MAIN_LOOP(K, i, a, c, d, e, FCALC_2_4(b, c, d))
#define MAIN_LOOP_3(K, i, a, b, c, d, e) MAIN_LOOP(K, i, a, c, d, e, FCALC_3(b, c, d))
#define MAIN_LOOP_1_5(K, i, a, b, c, d, e) MAIN_LOOP_1(K, i, a, b, c, d, e); MAIN_LOOP_1(K, i, a, b, c, d, e); MAIN_LOOP_1(K, i, a, b, c, d, e); MAIN_LOOP_1(K, i, a, b, c, d, e); MAIN_LOOP_1(K, i, a, b, c, d, e)
#define MAIN_LOOP_2_4_5(K, i, a, b, c, d, e) MAIN_LOOP_2_4(K, i, a, b, c, d, e); MAIN_LOOP_2_4(K, i, a, b, c, d, e); MAIN_LOOP_2_4(K, i, a, b, c, d, e); MAIN_LOOP_2_4(K, i, a, b, c, d, e); MAIN_LOOP_2_4(K, i, a, b, c, d, e)
#define MAIN_LOOP_3_5(K, i, a, b, c, d, e) MAIN_LOOP_3(K, i, a, b, c, d, e); MAIN_LOOP_3(K, i, a, b, c, d, e); MAIN_LOOP_3(K, i, a, b, c, d, e); MAIN_LOOP_3(K, i, a, b, c, d, e); MAIN_LOOP_3(K, i, a, b, c, d, e)

void sha1Hash(char* guess, __m128i *res, __m128i hash128b, __m128i hash128c, __m128i hash128d, __m128i hash128e) {
   
    //Multiple Variable Declaration 
    int i = 0; 
    __m128i ws[80] = {_mm_set1_epi32(0x0)}, tmp2, a,b,c,d,e, K1,K2,K3,K4, f, tmp;



    ws[0] = _mm_set_epi32(guess[0], guess[0+6], guess[0+12], guess[0+18]);
    ws[0] = _mm_slli_epi32(ws[0], 8);

    tmp2 = _mm_set_epi32(guess[1], guess[1+6], guess[1+12], guess[1+18]);
    ws[0] = _mm_or_si128(tmp2 , ws[0]);
    ws[0] = _mm_slli_epi32(ws[0], 8);
    
    tmp2 = _mm_set_epi32(guess[2], guess[2+6], guess[2+12], guess[2+18]);
    ws[0] = _mm_or_si128(tmp2 , ws[0]);
    ws[0] = _mm_slli_epi32(ws[0], 8);
    
    tmp2 = _mm_set_epi32(guess[3], guess[3+6], guess[3+12], guess[3+18]);
    ws[0] = _mm_or_si128(tmp2 , ws[0]);


    ws[1] = _mm_set_epi32(guess[4], guess[4+6], guess[4+12], guess[4+18]);
    ws[1] = _mm_slli_epi32(ws[1], 8);

    tmp2 = _mm_set_epi32(guess[5], guess[5+6], guess[5+12], guess[5+18]);
    ws[1] = _mm_or_si128(tmp2 , ws[1]);
    ws[1] = _mm_slli_epi32(ws[1], 1);

    // Append 1 to message
    tmp2 = _mm_set_epi32(1,1,1,1);
    ws[1] = _mm_or_si128(tmp2 , ws[1]);
    ws[1] = _mm_slli_epi32(ws[1], 15);

    ws[15] = _mm_set1_epi32(0x30);

    for (i = 16; i < 80; ++i) {
       ws[i] = _mm_xor_si128(_mm_xor_si128(_mm_xor_si128(ws[i-3], ws[i-8]), ws[i-14]), ws[i-16]);

       ws[i] = ROTATE_LEFT128(ws[i], 1);
    }

    a = _mm_set1_epi32(H_0);
    b = _mm_set1_epi32(H_1);
    c = _mm_set1_epi32(H_2);
    d = _mm_set1_epi32(H_3);
    e = _mm_set1_epi32(H_4);

    K1 = _mm_set1_epi32(K_1);
    K2 = _mm_set1_epi32(K_2);
    K3 = _mm_set1_epi32(K_3);
    K4 = _mm_set1_epi32(K_4);

    // Loop unrolling
    i = 0;
    MAIN_LOOP_1_5(K1, i, a, b, c, d, e);
    MAIN_LOOP_1_5(K1, i, a, b, c, d, e);
    MAIN_LOOP_1_5(K1, i, a, b, c, d, e);
    MAIN_LOOP_1_5(K1, i, a, b, c, d, e);

    MAIN_LOOP_2_4_5(K2, i, a, b, c, d, e);
    MAIN_LOOP_2_4_5(K2, i, a, b, c, d, e);
    MAIN_LOOP_2_4_5(K2, i, a, b, c, d, e);
    MAIN_LOOP_2_4_5(K2, i, a, b, c, d, e);
    
    MAIN_LOOP_3_5(K3, i, a, b, c, d, e);
    MAIN_LOOP_3_5(K3, i, a, b, c, d, e);
    MAIN_LOOP_3_5(K3, i, a, b, c, d, e);
    MAIN_LOOP_3_5(K3, i, a, b, c, d, e);
    
    MAIN_LOOP_2_4_5(K4, i, a, b, c, d, e);
    MAIN_LOOP_2_4_5(K4, i, a, b, c, d, e);
    MAIN_LOOP_2_4_5(K4, i, a, b, c, d, e);

    //Round 75 unrolled Loop
    f = _mm_xor_si128(_mm_xor_si128(b,c), d);
    tmp = _mm_add_epi32(_mm_add_epi32(_mm_add_epi32(_mm_add_epi32(ROTATE_LEFT128(a, 5), f), e), K4), ws[75]);
    //Early-Exit: tmp->a->b->c->e so if tmp has nothing equal to hash128c it can't be the correct one
    if (!_mm_movemask_epi8(_mm_cmpeq_epi32(hash128e, tmp))) { return; }
    e = d;
    d = c;
    c = _mm_or_si128(_mm_slli_epi32(b, 30), _mm_srli_epi32(b, 2));
    b = a;
    a = tmp;

    //Round 76 unrolled Loop
    f = _mm_xor_si128(_mm_xor_si128(b,c), d);
    tmp = _mm_add_epi32(_mm_add_epi32(_mm_add_epi32(_mm_add_epi32(ROTATE_LEFT128(a, 5), f), e), K4), ws[76]);
    //Early-Exit: tmp->a->b->c->d so if tmp has nothing equal to hash128c it can't be the correct one
    if (!_mm_movemask_epi8(_mm_cmpeq_epi32(hash128d, tmp))) { return; }
    e = d;
    d = c;
    c = _mm_or_si128(_mm_slli_epi32(b, 30), _mm_srli_epi32(b, 2));
    b = a;
    a = tmp;

    //Round 77 unrolled Loop
    f = _mm_xor_si128(_mm_xor_si128(b,c), d);
    tmp = _mm_add_epi32(_mm_add_epi32(_mm_add_epi32(_mm_add_epi32(ROTATE_LEFT128(a, 5), f), e), K4), ws[77]);
    //Early-Exit: tmp->a->b->c so if tmp has nothing equal to hash128c it can't be the correct one
    if (!_mm_movemask_epi8(_mm_cmpeq_epi32(hash128c, tmp))) { return; }
    e = d;
    d = c;
    c = _mm_or_si128(_mm_slli_epi32(b, 30), _mm_srli_epi32(b, 2));
    b = a;
    a = tmp;

    //Round 78 unrolled Loop
    f = _mm_xor_si128(_mm_xor_si128(b,c), d);
    tmp = _mm_add_epi32(_mm_add_epi32(_mm_add_epi32(_mm_add_epi32(ROTATE_LEFT128(a, 5), f), e), K4), ws[78]);
    //Early-Exit: tmp->a->b so if tmp has nothing equal to hash128b it can't be the correct one
    if (!_mm_movemask_epi8(_mm_cmpeq_epi32(hash128b, tmp))) { return; }
    e = d;
    d = c;
    c = _mm_or_si128(_mm_slli_epi32(b, 30), _mm_srli_epi32(b, 2));
    b = a;
    a = tmp;

    //Round 79 unrolled Loop
    f = _mm_xor_si128(_mm_xor_si128(b,c), d);
    tmp = _mm_add_epi32(_mm_add_epi32(_mm_add_epi32(_mm_add_epi32(ROTATE_LEFT128(a, 5), f), e), K4), ws[79]);
    e = d;
    d = c;
    c = _mm_or_si128(_mm_slli_epi32(b, 30), _mm_srli_epi32(b, 2)); 
    b = a;
    a = tmp;


    //No need for adding constants because of Early-Exit Optimization
    res[0] = a;
    res[1] = b;
    res[2] = c;
    res[3] = d;
    res[4] = e;
}


int crackHash(struct state hash, char *result) {

    //Subtract Constants to apply Early-exit Optimization
    __m128i hash128a = _mm_set1_epi32(hash.a-H_0);
    __m128i hash128b = _mm_set1_epi32(hash.b-H_1);
    __m128i hash128c = _mm_set1_epi32(hash.c-H_2);
    __m128i hash128d = _mm_set1_epi32(hash.d-H_3);
    __m128i hash128e = _mm_set1_epi32(hash.e-H_4);

    char letters[] = "abcdefghijklmnopqrstuvwxyz";
    char guess[24];
    __m128i shaVal[5];
    int i,j,k,l,m,n;

    for (i = 0; i<=25; i++) {
        for (j = 0; j<=25; j++) {
            for (k = 0; k<=25; k++) {
                for (l = 0; l<=25; l++) {
                    for (m = 0; m<=25; m++) {
                        for (n = 0; n<=25; n+=4) {
                            guess[0] = letters[i];
                            guess[1] = letters[j];
                            guess[2] = letters[k];
                            guess[3] = letters[l];
                            guess[4] = letters[m];
                            guess[5] = letters[n];
                            
                            guess[6] = letters[i];
                            guess[7] = letters[j];
                            guess[8] = letters[k];
                            guess[9] = letters[l];
                            guess[10] = letters[m];
                            guess[11] = letters[n+1];

                            if (n+2 < 26) {
                                guess[12] = letters[i];
                                guess[13] = letters[j];
                                guess[14] = letters[k];
                                guess[15] = letters[l];
                                guess[16] = letters[m];
                                guess[17] = letters[n+2];
                            } else {
                                guess[12] = 0;
                                guess[13] = 0;
                                guess[14] = 0;
                                guess[15] = 0;
                                guess[16] = 0;
                                guess[17] = 0;
                                           }

                            if (n+3 < 26) {
                                guess[18] = letters[i];
                                guess[19] = letters[j];
                                guess[20] = letters[k];
                                guess[21] = letters[l];
                                guess[22] = letters[m];
                                guess[23] = letters[n+3];
                            } else {
                                guess[18] = 0;
                                guess[19] = 0;
                                guess[20] = 0;
                                guess[21] = 0;
                                guess[22] = 0;
                                guess[23] = 0;
                            }


                            ////////////////////////
                            sha1Hash(guess, shaVal, hash128b, _mm_set1_epi32(ROTATE_RIGHT(hash.c-H_2,30)), _mm_set1_epi32(ROTATE_RIGHT(hash.d-H_3,30)), _mm_set1_epi32(ROTATE_RIGHT(hash.e-H_4,30)));
                            ////////////////////////

                            //If cmpeg does find some equal values then it will return 0xffffffff for the correct value
                            //and every other bits of the 128 will be 0x0.
                            //Movemask will find out if there is 0xf in the 128 bit and returns greater 1 if so.
			    int aEq =_mm_movemask_epi8(_mm_cmpeq_epi32(hash128a, shaVal[0]));
			    int bEq =_mm_movemask_epi8(_mm_cmpeq_epi32(hash128b, shaVal[1]));
			    int cEq =_mm_movemask_epi8(_mm_cmpeq_epi32(hash128c, shaVal[2]));
			    int dEq =_mm_movemask_epi8(_mm_cmpeq_epi32(hash128d, shaVal[3]));
			    int eEq =_mm_movemask_epi8(_mm_cmpeq_epi32(hash128e, shaVal[4]));
                            __m128i equalValIsOnes;

                            if (aEq && bEq && cEq && dEq && eEq){
                                equalValIsOnes = _mm_cmpeq_epi32(hash128a, shaVal[0]);

                                if (((unsigned char *)&equalValIsOnes)[13]) {
                                    result[0] = guess[0];
                                    result[1] = guess[1];
                                    result[2] = guess[2];
                                    result[3] = guess[3];
                                    result[4] = guess[4];
                                    result[5] = guess[5];
                                    return(EXIT_SUCCESS);
                                }

                                if (((unsigned char *)&equalValIsOnes)[9]) {
                                    result[0] = guess[6];
                                    result[1] = guess[7];
                                    result[2] = guess[8];
                                    result[3] = guess[9];
                                    result[4] = guess[10];
                                    result[5] = guess[11];
                                    return(EXIT_SUCCESS);
                                }

                                if (((unsigned char *)&equalValIsOnes)[5]) {
                                    result[0] = guess[12];
                                    result[1] = guess[13];
                                    result[2] = guess[14];
                                    result[3] = guess[15];
                                    result[4] = guess[16];
                                    result[5] = guess[17];
                                    return(EXIT_SUCCESS);
                                }

                                //if default
                                    result[0] = guess[18];
                                    result[1] = guess[19];
                                    result[2] = guess[20];
                                    result[3] = guess[21];
                                    result[4] = guess[22];
                                    result[5] = guess[23];
                                    return(EXIT_SUCCESS);

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
