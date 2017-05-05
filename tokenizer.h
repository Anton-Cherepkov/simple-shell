#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 1000
#endif

#ifndef TOKENIZERH
#define TOKENIZERH

#include "error.h"
#include "program.h"
#include "job.h"
#include "core.h"

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>
#include <string.h>

char *buffer;
int buffer_capacity;
int buffer_position;

void invite();

int initialize_buffer();

void destroy_buffer();

int read_line_to_buffer();

void skip_whitespaces_and_tabs();

int is_spec_symbol(char c);

int next_token1(char **res, char status, char *finish_status);

int next_token2(char **res);

int next_program(program **res);

int next_job(job **res);

void print_program(FILE *stream, const program *p);

void print_job(FILE *stream, const job *j);

int check_built_in_piped(const job *j);

#endif