#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 1000
#endif

#ifndef COREH
#define COREH

#include "error.h"
#include "job.h"
#include "program.h"
#include "tokenizer.h"

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pwd.h>
#include <linux/limits.h>
#include <limits.h>
#include <termios.h>
#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>

int argc_s;
char **argv_s;
int last_return_code;
pid_t shell_pgid;
struct termios shell_termios;
int shell_terminal;
char shell_isatty;

void say(int sig);

int set_environment_vars();

void initialize_shell();

int handle_waitpid(pid_t pid, int wstatus);

void update_all_jobs();

void wait_job(job *j);

void notify();

/* specify cont = 1 if u want to continue the job because it was stopped earlier */
void move_to_foreground(job *j, char cont);

/* specify cont = 1 if u want to continue the job because it was stopped earlier */
void move_to_background(job *j, char cont);

void spawn_program(program *p, pid_t pgid, int fd_in, int fd_out, int fg);

void spawn_job(job *j);

#endif
