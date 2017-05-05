#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 1000
#endif

#ifndef ERRORH
#define ERRORH

#include <errno.h>
#include <stdio.h>

#define E_OK 0
#define E_MEMFAIL 1 /* check errno */
#define E_EOF 2
#define E_READFAIL 3 /* check errno */
#define E_WRITEFAIL 4 /* check errno */
#define E_ENVVARSFAIL 5 /* check errno */
#define E_WRONG_QUOTE_BALANCE 6
#define E_SYNTAX_ERROR 7
#define E_CD_UNEXPECTED_ARGS 9
#define E_CD_FAIL 10 /* check errno */
#define E_BUILT_IN_PIPED 11 /* ... | exit(cd, etc.) | ... */

void print_error(int err);

#endif
