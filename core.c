#include "core.h"

int set_environment_vars()
{
    char pidstr[10];
    char symlinkpath[PATH_MAX + 1];
    char shellpath[PATH_MAX + 1];

    {
        struct passwd *passwd = getpwuid(getuid());
        if (!passwd) {
            return E_ENVVARSFAIL;
        }

        if (setenv("HOME", passwd->pw_dir, 1) == -1) {
            return E_ENVVARSFAIL;
        }

        if (setenv("USER", passwd->pw_name, 1) == -1) {
            return E_ENVVARSFAIL;
        }
    }

    sprintf(pidstr, "%d", (int) getpid());
    if (setenv("PID", pidstr, 1) == -1) /* don't forget to reassign PID for child process */
        return E_ENVVARSFAIL;

    if (chdir(getenv("HOME")) == -1)
        return E_ENVVARSFAIL;

    if (setenv("PWD", getenv("HOME"), 1) == -1)
        return E_ENVVARSFAIL;

    if (setenv("OLDPWD", getenv("HOME"), 1) == -1)
        return E_ENVVARSFAIL;

    sprintf(symlinkpath, "/proc/%d/exe", (int)getpid());
    {
        int bytes = readlink(symlinkpath, shellpath, PATH_MAX);
        if (bytes == -1)
            return E_ENVVARSFAIL;
        shellpath[bytes] = 0;
        if (setenv("SHELL", shellpath, 1) == -1)
            return E_ENVVARSFAIL;
    }

    return E_OK;
}

void initialize_shell()
{
    shell_terminal = STDIN_FILENO;
    shell_isatty = isatty(shell_terminal);

    if (shell_isatty)
    {
        shell_pgid = getpgrp();
        if (tcgetpgrp(shell_terminal) != shell_pgid)
        {
            fprintf(stderr, "Shell must be executed as a fg process\n");
            fflush(stderr);
            exit(1);
        }
        signal(SIGINT, SIG_IGN);
        signal(SIGQUIT, SIG_IGN);
        signal(SIGTSTP, SIG_IGN);
        signal(SIGTTIN, SIG_IGN);
        signal(SIGTTOU, SIG_IGN);

        shell_pgid = getpid();
        if (setpgid(shell_pgid, shell_pgid) == -1)
        {
            perror("setpgid");
            exit(1);
        }

        tcsetpgrp(shell_terminal, shell_pgid);
        tcgetattr(shell_terminal, &shell_termios);
    }
}

int handle_waitpid(pid_t pid, int wstatus)
{
    job *j;
    size_t i;

    if (pid == 0 || errno == ECHILD)
    {
        return -1;
    }
    else if (pid > 0)
    {
        for (j = job_list; j; j = j->next)
        {
            for (i = 0; i < j->number_of_programs; ++i)
            {
                if (pid != j->programs[i]->pid)
                    continue;
                j->programs[i]->wstatus = wstatus;
                if (WIFSTOPPED(wstatus))
                    j->programs[i]->stopped = 1;
                else
                    j->programs[i]->completed = 1;
                return 0;
            }
        }
        fprintf(stderr, "handle_waitpid received unknown pid\n");
        fflush(stderr);
        return -1;
    }

    perror("waitpid");
    return -1;
}

void update_all_jobs()
{
    int wstatus;
    pid_t pid;

    do
    {
        errno = 0;
        pid = waitpid(-1, &wstatus, WUNTRACED | WNOHANG);
    } while (!handle_waitpid(pid, wstatus));
}

void wait_job(job *j)
{
    int wstatus;
    pid_t pid;

    do
    {
        errno = 0;
        pid = waitpid(-1, &wstatus, WUNTRACED);
        last_return_code = WEXITSTATUS(wstatus);
    } while (!handle_waitpid(pid, wstatus) && !job_is_completed(j) && !job_is_stopped(j));

    if (job_is_completed(j))
        j->notified = 1;
}

void notify()
{
    job *current = job_list;

    for (; current; current = current->next)
    {
        if (!current->notified && job_is_completed(current))
        {
            fprintf(stderr, "Job [%d] [", current->pgid);
            print_job(stderr, current);
            fprintf(stderr, "] is completed\n");
            fflush(stderr);
            current->notified = 1;
        }
    }
}

void move_to_foreground(job *j, char cont)
{
    size_t i;
    tcsetpgrp(shell_terminal, j->pgid);
    /*tcsetattr(shell_terminal, TCSADRAIN, &j->old_termios);*/
    if (cont)
    {
        if (kill(-(j->pgid), SIGCONT) == -1)
            perror("SIGCONT kill");
    }
    for (i = 0; i < j->number_of_programs; ++i)
        j->programs[i]->stopped = 0, j->programs[i]->completed = 0;
    wait_job(j);
    tcsetpgrp(shell_terminal, shell_pgid);
    tcgetattr(shell_terminal, &(j->old_termios));
    tcsetattr(shell_terminal, TCSADRAIN, &shell_termios);
}

void move_to_background(job *j, char cont)
{
    size_t i = 0;
    for (i = 0; i < j->number_of_programs; ++i)
        j->programs[i]->stopped = 0, j->programs[i]->completed = 0;
    if (cont && kill(-(j->pgid), SIGCONT) == -1)
        perror("SIGCONT kill");
}

void spawn_program(program *p, pid_t pgid, int fd_in, int fd_out, int fg)
{
    pid_t pid;

    if (shell_isatty)
    {
        pid = getpid();
        if (pgid == 0)
        {
            pgid = pid;
        }
        setpgid(pid, pgid);
        if (fg)
        {
            if (tcsetpgrp(shell_terminal, pgid) == -1)
            {
                perror("tcsetpgrp");
                exit(1);
            }
        }

        signal(SIGINT, SIG_DFL);
        signal(SIGQUIT, SIG_DFL);
        signal(SIGTSTP, SIG_DFL);
        signal(SIGTTIN, SIG_DFL);
        signal(SIGTTOU, SIG_DFL);
        signal(SIGCHLD, SIG_DFL);
    }

    if (p->output_file)
    {
        FILE *file = fopen(p->output_file, (p->output_type == 1 ? "w" : "a"));
        if (fd_out != STDOUT_FILENO) close(fd_out);
        if (!file)
        {
            perror("fopen");
            exit(127);
        }
        fd_out = fileno(file);
    }

    if (p->input_file)
    {
        FILE *file = fopen(p->input_file, "r");
        if (fd_in != STDOUT_FILENO) close(fd_in);
        if (!file)
        {
            perror("fopen");
            exit(127);
        }
        fd_in = fileno(file);
    }

    if (fd_in != STDIN_FILENO)
    {
        dup2(fd_in, STDIN_FILENO);
        close(fd_in);
    }
    if (fd_out != STDOUT_FILENO)
    {
        dup2(fd_out, STDOUT_FILENO);
        close(fd_out);
    }

    execvp(p->arguments[0], p->arguments);
    perror("exec");
    exit(127);
}

void spawn_job(job *j)
{
    size_t i;
    pid_t pid;
    int fd_pipe[2];
    int in, out;

    if (!j->number_of_programs)
        return;

    in = STDIN_FILENO;
    for (i = 0; i < j->number_of_programs; ++i)
    {
        if (i != j->number_of_programs - 1)
        {
            if (pipe(fd_pipe) == -1)
            {
                perror("pipe");
                out = STDOUT_FILENO;
            }
            else
            {
                out = fd_pipe[1];
            }
        }
        else
        {
            out = STDOUT_FILENO;
        }

        pid = fork();
        if (pid == -1)
        {
            perror("fork");
            exit(1);
        }

        if (pid)    /* Parent */
        {
            j->programs[i]->pid = pid;
            if (shell_isatty)
            {
                if (j->pgid == 0)
                    j->pgid = pid;
                setpgid(pid, j->pgid);
            }
        }
        else         /* Son */
        {
            spawn_program(j->programs[i], j->pgid, in, out, !j->background);
        }

        if (in != STDIN_FILENO)
            close(in);
        if (out != STDOUT_FILENO)
            close(out);
        in = fd_pipe[0];
    }

    if (!shell_isatty)
        wait_job(j);
    else if (j->background)
        move_to_background(j, 0);
    else
        move_to_foreground(j, 0);
}
