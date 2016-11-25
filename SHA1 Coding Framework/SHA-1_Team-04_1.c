#include "sha1.h"

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

#define H0 0x67452301
#define H1 0xefcdab89
#define H2 0x98badcfe
#define H3 0x10325476
#define H4 0xc3d2e1f0

#define K1 0x5a827999
#define K2 0x6ed9eba1
#define K3 0x8f1bbcdc
#define K4 0xca62c1d6


void sha1Hash(char* guess, unsigned int* res) {
    
    unsigned int ws[80] = {0};
    int i; 

    ws[0] = guess[0];
    ws[0] <<= 8;
    ws[0] |= guess[1];
    ws[0] <<= 8;
    ws[0] |= guess[2];
    ws[0] <<= 8;
    ws[0] |= guess[3];
    ws[1] = guess[4];
    ws[1] <<= 8;
    ws[1] |= guess[5];
    ws[1] <<= 1;
    ws[1] |= 1;
    ws[1] <<= 15;
    ws[15] = 0x30;
    
    for (i = 16; i < 80; ++i) {
       ws[i] = ws[i-3] ^ ws[i-8] ^ ws[i-14] ^ ws[i-16];
       ws[i] = (ws[i] << 1) | (ws[i] >> 31);
    }

    unsigned int a, b, c, d, e;
    a = H0;
    b = H1;
    c = H2;
    d = H3;
    e = H4;

    unsigned int f, tmp;
    for (i = 0; i < 20; i++) {
        f = (b & c) | ((~b) & d);
	tmp = ((a << 5) | (a >> 27)) + f + e + K1 + ws[i];
	e = d;
	d = c;
	c = (b << 30) | (b >> 2);
	b = a;
	a = tmp;
    }
    for (; i < 40; i++) {
        f = b ^ c ^ d;
	tmp = ((a << 5) | (a >> 27)) + f + e + K2 + ws[i];
	e = d;
	d = c;
	c = (b << 30) | (b >> 2);
	b = a;
	a = tmp;
    }
    for (; i < 60; i++) {
        f = (b & c) | (b & d) | (c & d);
	tmp = ((a << 5) | (a >> 27)) + f + e + K3 + ws[i];
	e = d;
	d = c;
	c = (b << 30) | (b >> 2);
	b = a;
	a = tmp;
    }
    for (; i < 80; i++) {
        f = b ^ c ^ d;
	tmp = ((a << 5) | (a >> 27)) + f + e + K4 + ws[i];
	e = d;
	d = c;
	c = (b << 30) | (b >> 2);
	b = a;
	a = tmp;
    }

    res[0] = H0 + a;
    res[1] = H1 + b;
    res[2] = H2 + c;
    res[3] = H3 + d;
    res[4] = H4 + e;
}


int crackHash(struct state hash, char *result) {


    char alphaNum[] = "abcdefghijklmnopqrstuvwxyz";
    char* hashStr;
    char guess[6];
    unsigned int shaVal[5];
    int i,j,k,l,m,n;

    for (i = 0; i<=25; i++) {
        for (j = 0; j<=25; j++) {
            for (k = 0; k<=25; k++) {
                for (l = 0; l<=25; l++) {
                    for (m = 0; m<=25; m++) {
                        for (n = 0; n<=25; n++) {
                            guess[0] = alphaNum[i];
                            guess[1] = alphaNum[j];
                            guess[2] = alphaNum[k];
                            guess[3] = alphaNum[l];
                            guess[4] = alphaNum[m];
                            guess[5] = alphaNum[n];
                            printf("Hier: %s \n", guess);

                            sha1Hash(guess, shaVal);
                            if (hash.a == shaVal[0] && hash.b == shaVal[1] && hash.c == shaVal[2] && hash.d == shaVal[3] && hash.e == shaVal[4]) {

                                result[0] = guess[0];
                                result[1] = guess[1];
                                result[2] = guess[2];
                                result[3] = guess[3];
                                result[4] = guess[4];
                                result[5] = guess[5];
                                /* Found */
                                return(EXIT_SUCCESS);
                                /* Not found */
                                return(EXIT_FAILURE);
                                
                            }

                        }
                    }
                }
            }
        }
    }

}
