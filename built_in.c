#include "built_in.h"

int cd_handler(const program *p)
{
    char *path;
    char *path_real;

    assert(strcmp(p->arguments[0], "cd") == 0);

    if (p->number_of_arguments == 1)
    {
        path = getenv("HOME");
    }
    else if (p->number_of_arguments == 2)
    {
        path = p->arguments[1];
    }
    else
    {
        return E_CD_UNEXPECTED_ARGS;
    }

    path_real = realpath(path, NULL);
    if (!path_real)
    {
        free(path_real);
        return E_CD_FAIL;
    }

    if (chdir(path_real) == -1)
    {
        free(path_real);
        return E_CD_FAIL;
    }

    if (setenv("OLDPWD", getenv("PWD"), 1) == -1)
    {
        free(path_real);
        return E_CD_FAIL;
    }

    if (setenv("PWD", path_real, 1) == -1)
    {
        free(path_real);
        return E_CD_FAIL;
    }

    free(path_real);
    return E_OK;
}

void jobs_handler(job *j)
{
    job *current;

    if (j->programs[0]->number_of_arguments > 1)
    {
        fprintf(stderr, "jobs: too many arguments\n");
        fflush(stderr);
        return;
    }

    update_all_jobs();
    for (current = job_list; current; current = current->next)
    {
        if (!job_is_completed(current))
        {
            printf("[%d] [%s]\t", current->pgid, (job_is_stopped(current) ? "Stopped" : "Running"));
            print_job(stdout, current);
            printf("\n");
            fflush(stdout);
        }
    }
}

void fg_handler(job *j)
{
    job *z;
    size_t i;
    pid_t pgid = 0;

    if (j->programs[0]->number_of_arguments > 2)
    {
        fprintf(stderr, "fg: too many arguments\n");
        fflush(stderr);
        return;
    }
    if (j->programs[0]->number_of_arguments < 2)
    {
        fprintf(stderr, "fg: too few arguments\n");
        fflush(stderr);
        return;
    }

    for (i = 0; (j->programs[0]->arguments[1])[i]; ++i)
    {
        if (!((j->programs[0]->arguments[1])[i] >= '0' && (j->programs[0]->arguments[1])[i] <= '9'))
        {
            fprintf(stderr, "fg: unsigned integer expected\n");
            fflush(stderr);
            return;
        }
        pgid *= 10;
        pgid += ((j->programs[0]->arguments[1])[i] - '0');
    }

    z = get_job_by_pgid(pgid);
    if (!z || job_is_completed(z))
    {
        fprintf(stderr, "fg: unknown id\n");
        fflush(stderr);
        return;
    }

    if (!job_is_stopped(z))
    {
        kill(-z->pgid, SIGSTOP);
    }
    move_to_foreground(z, 1);
}

void bg_handler(job *j)
{
    job *z;
    size_t i;
    pid_t pgid = 0;

    if (j->programs[0]->number_of_arguments > 2)
    {
        fprintf(stderr, "bg: too many arguments\n");
        fflush(stderr);
        return;
    }
    if (j->programs[0]->number_of_arguments < 2)
    {
        fprintf(stderr, "bg: too few arguments\n");
        fflush(stderr);
        return;
    }

    for (i = 0; (j->programs[0]->arguments[1])[i]; ++i)
    {
        if (!((j->programs[0]->arguments[1])[i] >= '0' && (j->programs[0]->arguments[1])[i] <= '9'))
        {
            fprintf(stderr, "bg: unsigned integer expected\n");
            fflush(stderr);
            return;
        }
        pgid *= 10;
        pgid += ((j->programs[0]->arguments[1])[i] - '0');
    }

    z = get_job_by_pgid(pgid);
    if (!z || job_is_completed(z))
    {
        fprintf(stderr, "bg: unknown id\n");
        fflush(stderr);
        return;
    }
    if (!job_is_stopped(z))
    {
        fprintf(stderr, "bg: job is already in background\n");
        fflush(stderr);
        return;
    }

    move_to_background(z, 1);
}
