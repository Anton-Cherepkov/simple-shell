#include "error.h"

void print_error(int err)
{
    if (err == E_OK)
        return;

    switch (err)
    {
        case E_MEMFAIL:
            perror("Memory fail");
            break;
        case E_READFAIL:
            perror("Read fail");
            break;
        case E_WRITEFAIL:
            perror("Write fail");
            break;
        case E_ENVVARSFAIL:
            perror("Setting of environment variables failed");
            break;
        case E_SYNTAX_ERROR:
            fprintf(stderr, "Syntax error\n");
            fflush(stderr);
            break;
        case E_CD_UNEXPECTED_ARGS:
            fprintf(stderr, "cd failed: too many arguments\n");
            break;
        case E_CD_FAIL:
            perror("cd failed");
            break;
        default:
            fprintf(stderr, "Unknown error\n");
            fflush(stderr);
    }
}
