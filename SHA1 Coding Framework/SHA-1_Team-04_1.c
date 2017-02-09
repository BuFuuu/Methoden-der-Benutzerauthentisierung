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

#define H_0 0x67452301
#define H_1 0xefcdab89
#define H_2 0x98badcfe
#define H_3 0x10325476
#define H_4 0xc3d2e1f0

#define K_1 0x5a827999
#define K_2 0x6ed9eba1
#define K_3 0x8f1bbcdc
#define K_4 0xca62c1d6



//#define ROTATE_LEFT128(x,n) _mm_or_si128 (_mm_slli_epi32((x),(n)), _mm_srli_epi32((x), (32-(n))))
#define ROTATE_LEFT128(x, n) _mm_or_si128 (_mm_slli_epi32 ((x), (n)), _mm_srli_epi32 ((x), (32 - (n))))
#define ROTATE_LEFT(x,n) (((x) << (n)) | ((x) >> (32-(n))))

void sha1Hash(char* guess, __m128i *res) {
   
    //Multiple Variable Declaration 
     int i; 
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

    //append 1 to message
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

    for (i = 0; i < 20; i++) {
        f = _mm_or_si128(_mm_and_si128(b,c), _mm_andnot_si128(b,d));
	tmp = _mm_add_epi32(_mm_add_epi32(_mm_add_epi32(_mm_add_epi32(ROTATE_LEFT128(a, 5), f), e), K1), ws[i]);
	e = d;
	d = c;

	c = _mm_or_si128(_mm_slli_epi32(b, 30), _mm_srli_epi32(b, 2));
	b = a;
	a = tmp;
    }
    for (; i < 40; i++) {
        f = _mm_xor_si128(_mm_xor_si128(b,c), d);
	tmp = _mm_add_epi32(_mm_add_epi32(_mm_add_epi32(_mm_add_epi32(ROTATE_LEFT128(a, 5), f), e), K2), ws[i]);
	e = d;
	d = c;
	c = _mm_or_si128(_mm_slli_epi32(b, 30), _mm_srli_epi32(b, 2));
	b = a;
	a = tmp;
    }
    for (; i < 60; i++) {
        f = _mm_or_si128(_mm_or_si128(_mm_and_si128(b,c), _mm_and_si128(b,d)),_mm_and_si128(c,d));
	tmp = _mm_add_epi32(_mm_add_epi32(_mm_add_epi32(_mm_add_epi32(ROTATE_LEFT128(a, 5), f), e), K3), ws[i]);
	e = d;
	d = c;
	c = _mm_or_si128(_mm_slli_epi32(b, 30), _mm_srli_epi32(b, 2));
	b = a;
	a = tmp;
    }
    for (; i < 77; i++) {
        f = _mm_xor_si128(_mm_xor_si128(b,c), d);
	tmp = _mm_add_epi32(_mm_add_epi32(_mm_add_epi32(_mm_add_epi32(ROTATE_LEFT128(a, 5), f), e), K4), ws[i]);
	e = d;
	d = c;
	c = _mm_or_si128(_mm_slli_epi32(b, 30), _mm_srli_epi32(b, 2));
	b = a;
	a = tmp;
    }

    //Round 77 unrolled Loop
    f = _mm_xor_si128(_mm_xor_si128(b,c), d);
    tmp = _mm_add_epi32(_mm_add_epi32(_mm_add_epi32(_mm_add_epi32(ROTATE_LEFT128(a, 5), f), e), K4), ws[77]);
    e = d;
    d = c;
    c = _mm_or_si128(_mm_slli_epi32(b, 30), _mm_srli_epi32(b, 2));
    b = a;
    a = tmp;

    //Round 78 unrolled Loop
    f = _mm_xor_si128(_mm_xor_si128(b,c), d);
    tmp = _mm_add_epi32(_mm_add_epi32(_mm_add_epi32(_mm_add_epi32(ROTATE_LEFT128(a, 5), f), e), K4), ws[78]);
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

    char alphaNum[] = "abcdefghijklmnopqrstuvwxyz";
    char guess[24];
    __m128i shaVal[5];
    int i,j,k,l,m,n;

    for (i = 0; i<=25; i++) {
        for (j = 0; j<=25; j++) {
            for (k = 0; k<=25; k++) {
                for (l = 0; l<=25; l++) {
                    for (m = 0; m<=25; m++) {
                        for (n = 0; n<=25; n+=4) {
                            guess[0] = alphaNum[i];
                            guess[1] = alphaNum[j];
                            guess[2] = alphaNum[k];
                            guess[3] = alphaNum[l];
                            guess[4] = alphaNum[m];
                            guess[5] = alphaNum[n];
                            
                            guess[6] = alphaNum[i];
                            guess[7] = alphaNum[j];
                            guess[8] = alphaNum[k];
                            guess[9] = alphaNum[l];
                            guess[10] = alphaNum[m];
                            guess[11] = alphaNum[n+1];

                            if (n+2 < 26) {
                                guess[12] = alphaNum[i];
                                guess[13] = alphaNum[j];
                                guess[14] = alphaNum[k];
                                guess[15] = alphaNum[l];
                                guess[16] = alphaNum[m];
                                guess[17] = alphaNum[n+2];
                            } else {
                                guess[12] = 0;
                                guess[13] = 0;
                                guess[14] = 0;
                                guess[15] = 0;
                                guess[16] = 0;
                                guess[17] = 0;
                                           }

                            if (n+3 < 26) {
                                guess[18] = alphaNum[i];
                                guess[19] = alphaNum[j];
                                guess[20] = alphaNum[k];
                                guess[21] = alphaNum[l];
                                guess[22] = alphaNum[m];
                                guess[23] = alphaNum[n+3];
                            } else {
                                guess[18] = 0;
                                guess[19] = 0;
                                guess[20] = 0;
                                guess[21] = 0;
                                guess[22] = 0;
                                guess[23] = 0;
                            }


                            ////////////////////////
                            sha1Hash(guess, shaVal);
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
