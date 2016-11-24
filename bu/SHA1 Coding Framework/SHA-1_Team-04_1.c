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

int crackHash(struct state hash, char *result) {
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