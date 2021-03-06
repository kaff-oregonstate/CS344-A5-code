// keygen.c

// generates and prints to stdout a "random" string of allowed characters of specified length

// include libraries for functions
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <math.h>
#include <sys/time.h>

int main(int argc, char const *argv[]) {
    // printf("argc = %d\n", argc);
    if (argc < 2) {
        fprintf(stderr,"USAGE: %s keylength\n", argv[0]);
        exit(0);
    }

    int key_length = atoi(argv[1]);
    char key[key_length+2];
    int next_char;

    // stackoverflow/10192903, man pages for sys/time.h
    struct timeval start;
    gettimeofday(&start, NULL);
    int current_time = start.tv_sec;
    srand(current_time);

    // generate random key
    for (size_t i = 0; i < key_length; i++) {
        next_char = rand() % 27;
        if (next_char == 0) {next_char = 32;}
        else {next_char = next_char + 64;}

        key[i] = next_char;
    }
    key[key_length] = '\0';

    printf("%s\n", key);

    return EXIT_SUCCESS;
}