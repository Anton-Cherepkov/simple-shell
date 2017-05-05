#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 1000
#endif

#ifndef JOBH
#define JOBH

#include "error.h"
#include "program.h"

#include <stdlib.h>
#include <termios.h>
#include <signal.h>

struct job_s
{
    struct job_s *next;

    int number_of_programs;
    program **programs;

    int background; /* 0 - no, 1 - yes */
    pid_t pgid;
    struct termios old_termios;

    char notified;
};
typedef struct job_s job;
extern job *job_list;

job *get_nth_job(size_t n);

job *get_job_by_pgid(pid_t pgid);

int initialize_job(job **j);

void destroy_job(job **j);

void destroy_job_list();

char job_is_stopped(job *j);

char job_is_completed(job *j);

#endif
