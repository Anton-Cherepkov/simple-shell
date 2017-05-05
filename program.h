#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 1000
#endif

#ifndef PROGRAMH
#define PROGRAMH

#include "error.h"

#include <stdlib.h>

typedef struct
{
    int number_of_arguments;
    char **arguments;

    char *input_file, *output_file;
    int output_type; /* 1 - rewrite, 2 - append */

    pid_t pid;
    char completed;
    char stopped;
    int wstatus;
} program;

int initialize_program(program **p);

void destroy_program(program **p);

#endif
