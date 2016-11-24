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


char* sha1Hash(char* guess) {

    char* hash = "";
    char x[511] = {"\x00"};

    // x0 = m0 m1 m2 m3
    // x1 = m4 m5 01 00
    // x2 = 00 00 00 00
    // x3 = ..
    // x15 = 00 00 00 00
    x[0] = guess[0];
    x[1] = guess[1];
    x[2] = guess[2];
    x[3] = guess[3];
    x[4] = guess[4];
    x[5] = guess[5];
    x[6] = "\x01";

    int i;
    for (i = 16; i < 79; ++i) {
       x[i] = (x[i-3] ^ x[i-8] ^ x[i-14] ^ x[i-16]);
    }
    

    hash = x;
    x[511] = "\x30";

    __asm__("int3");
    return hash;
}


int crackHash(struct state hash, char *result) {


    char alphaNum[] = "abcdefghijklmnopqrstuvwxyz";
    char* hashStr;
    char guess[5];
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

                            hashStr = sha1Hash(guess);

                        }
                    }
                }
            }
        }
    }

    result[0] = 'a';
    result[1] = 'b';
    result[2] = 'c';
    result[3] = 'd';
    result[4] = 'e';
    result[5] = 'f';
    /* Found */
    return(EXIT_SUCCESS);
    /* Not found */
    return(EXIT_FAILURE);
}
