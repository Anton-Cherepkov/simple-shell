#include "program.h"

int initialize_program(program **p)
{
    *p = malloc(sizeof(program));
    if (!(*p))
        return E_MEMFAIL;

    (*p)->number_of_arguments = 0;
    (*p)->arguments = NULL;

    (*p)->input_file = (*p)->output_file = NULL;

    (*p)->completed = 0;
    (*p)->stopped = 0;

    (*p)->pid = 0;

    return E_OK;
}

void destroy_program(program **p)
{
    if (!(*p))
        return;

    {
        size_t i;
        for (i = 0; i < (*p)->number_of_arguments; ++i)
            free((*p)->arguments[i]);
        free((*p)->arguments);
    }

    free((*p)->input_file);
    free((*p)->output_file);

    free(*p);
    *p = NULL;
}
