#include "job.h"

job *job_list = NULL;

job *get_nth_job(size_t n)
{
    size_t i;
    job *current;
    for (current = job_list, i = 0; i < n && current; ++i, current = current->next);
    return current;
}

job *get_job_by_pgid(pid_t pgid)
{
    job *current;
    for (current = job_list; current && current->pgid != pgid; current = current->next);
    return current;
}

int initialize_job(job **j)
{
    *j = malloc(sizeof(job));
    if (!(*j))
        return E_MEMFAIL;

    (*j)->number_of_programs = 0;
    (*j)->programs = NULL;
    (*j)->background = 0;

    (*j)->next = NULL;
    (*j)->pgid = 0;

    (*j)->notified = 0;
    return E_OK;
}

void destroy_job(job **j)
{
    size_t i;

    if (!(*j))
        return;

    for (i = 0; i < (*j)->number_of_programs; ++i)
        destroy_program(&((*j)->programs[i]));
    free((*j)->programs);

    free(*j);
    *j = NULL;
}

void destroy_job_list()
{
    job* current, *next;
    current = job_list;
    while (current)
    {
        next = current->next;
        if (!job_is_completed(current))
        {
            kill(-(current->pgid), SIGKILL);
        }
        destroy_job(&current);
        current = next;
    }
}

char job_is_stopped(job *j)
{
    size_t i;
    for (i = 0; i < j->number_of_programs; ++i)
        if (!j->programs[i]->completed && !j->programs[i]->stopped)
            return 0;
    return 1;
}

char job_is_completed(job *j)
{
    size_t i;
    for (i = 0; i < j->number_of_programs; ++i)
        if (!j->programs[i]->completed)
            return 0;
    return 1;
}
