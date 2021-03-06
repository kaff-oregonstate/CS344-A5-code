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
    // printf("key_length = %d\n", key_length);

    // printf("%d\n", 'A'); // 65
    // printf("%d\n", 'Z'); // 90
    // printf("%d\n", ' '); // 32
    // printf("%d\n", '\n'); // ?

    // int an_int = 65;
    // char a_char = an_int;
    // printf("%c\n", a_char);

    char key[key_length+2];
    int next_char;

    // stackoverflow/10192903
    struct timeval start;
    gettimeofday(&start, NULL);
    int current_time = start.tv_sec;

    srand(current_time);

    for (size_t i = 0; i < key_length; i++) {
        next_char = rand() % 27;
        if (next_char == 0) {next_char = 32;}
        else {next_char = next_char + 64;}

        key[i] = next_char;
    }
    key[key_length] = '\n';
    key[key_length] = '\0';

    printf("%s\n", key);

    return EXIT_SUCCESS;
}