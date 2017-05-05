#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 1000
#endif

#include "error.h"
#include "program.h"
#include "job.h"
#include "tokenizer.h"
#include "core.h"
#include "built_in.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    int next_job_res;
    job *j;

    initialize_shell();
    {
        int res = set_environment_vars();
        if (res != E_OK)
        {
            print_error(res);
            return 1;
        }

        argc_s = argc;
        argv_s = argv;
    }
    initialize_buffer();

    while (1)
    {
        update_all_jobs();
        notify();

        if (shell_isatty)
            invite();
        if (read_line_to_buffer() == E_EOF)
            break;

        next1: next_job_res = next_job(&j);

        if (next_job_res != E_OK)
        {
            print_error(next_job_res);
        }
        else if (!j->number_of_programs)
        {
            destroy_job(&j);
        }
        else if (j->number_of_programs > 1 && check_built_in_piped(j) == E_BUILT_IN_PIPED)
        {
            fprintf(stderr, "built-in commands can not be piped\n");
            fflush(stderr);
            destroy_job(&j);
        }
        else if (!strcmp(j->programs[0]->arguments[0], "exit"))
        {
            destroy_job(&j);
            break;
        }
        else if (!strcmp(j->programs[0]->arguments[0], "cd"))
        {
            int cd_handler_res = cd_handler(j->programs[0]);
            print_error(cd_handler_res);
            destroy_job(&j);
        }
        else if (!strcmp(j->programs[0]->arguments[0], "jobs"))
        {
            jobs_handler(j);
            destroy_job(&j);
        }
        else if (!strcmp(j->programs[0]->arguments[0], "fg"))
        {
            fg_handler(j);
            destroy_job(&j);
        }
        else if (!strcmp(j->programs[0]->arguments[0], "bg"))
        {
            bg_handler(j);
            destroy_job(&j);
        }
        else
        {
            j->next = job_list;
            job_list = j;
            spawn_job(j);
        }

        if (buffer[buffer_position] == ';')
        {
            ++buffer_position;
            goto next1;
        }
    }

    update_all_jobs();
    notify();

    destroy_buffer();
    destroy_job_list();
    return 0;
}
