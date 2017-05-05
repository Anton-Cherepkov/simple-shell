#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 1000
#endif

#ifndef SHELL_BUILT_IN_H
#define SHELL_BUILT_IN_H

#include "error.h"
#include "program.h"
#include "job.h"
#include "core.h"
#include "tokenizer.h"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <limits.h>

int cd_handler(const program *p);

void jobs_handler(job *j);

void fg_handler(job *j);

void bg_handler(job *j);

#endif
