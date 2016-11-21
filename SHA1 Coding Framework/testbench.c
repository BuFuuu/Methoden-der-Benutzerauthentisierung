/* 
 * DO NOT CHANGE ANYTHING HERE
 */

#include "testbench.h"

int main(int argc, char** argv) {

    if(argc != 2) {
        printf("Please provide a 160-bit hash.\nUsage: ./crackSHA1 <HASH>\n");
        return -1;
    }

    if(strlen(argv[1]) != 40) {
        printf ("Please provide a 160-bit hash in hex-representation.\n");
        return -1;
    }

    char subbuffer[9] = {0};
    struct state hash;
    char result[] = "FAILED";
    int error;

    /* Convert the input string to a hexadecimal state */
    memcpy(subbuffer, argv[1] + 0 , 8);
    hash.a = strtoul(subbuffer, NULL, 16);
    memcpy(subbuffer, argv[1] + 8 , 8);
    hash.b = strtoul(subbuffer, NULL, 16);
    memcpy(subbuffer, argv[1] + 16, 8);
    hash.c = strtoul(subbuffer, NULL, 16);
    memcpy(subbuffer, argv[1] + 24, 8);
    hash.d = strtoul(subbuffer, NULL, 16);
    memcpy(subbuffer, argv[1] + 32, 8);
    hash.e = strtoul(subbuffer, NULL, 16);

    printf("Hash: 0x%08X %08X %08X %08X %08X\n", hash.a, hash.b, hash.c, hash.d, hash.e);

    clock_t start = clock();
    uint64_t cnt = rdtsc();
    error = crackHash(hash, result);
    cnt = rdtsc() - cnt;
    clock_t stop = clock();
    double elapsed = (double)(stop - start) * 1000.0 / CLOCKS_PER_SEC;

    if(error == EXIT_SUCCESS) {
        result[6] = '\0';
        printf("Preimge: %s\n", result);
    } else {
        printf("Preimge: %s\n", "NOT FOUND");
    }
    printf("Cycles: %" PRIu64 "\n", cnt);
    printf("Time: %f\n", elapsed);

    return 0;

}