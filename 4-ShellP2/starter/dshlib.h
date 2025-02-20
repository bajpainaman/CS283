#ifndef __DSHLIB_H__
#define __DSHLIB_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

/* Maximum sizes for strings and arrays */
#define EXE_MAX 64
#define ARG_MAX 256
#define CMD_MAX 8
#define CMD_ARGV_MAX 32

/* Shell prompt and built-in command name */
#define EXIT_CMD "exit"
#define SH_PROMPT "dsh2> "

/* Return codes */
#define OK 0
#define WARN_NO_CMDS -1
#define ERR_TOO_MANY_COMMANDS -2
#define ERR_MEMORY -3

/* Console messages */
#define CMD_WARN_NO_CMD "warning: no commands provided\n"
#define CMD_ERR_PIPE_LIMIT "error: piping limited to %d commands\n"
#define CMD_ERR_EXECUTE "error: execution failure\n"

/* Structure for a single command buffer */
typedef struct cmd_buff {
    int argc;
    char *argv[CMD_ARGV_MAX];
    char *_cmd_buffer; /* dynamically allocated copy of the input line */
} cmd_buff_t;

/* Function prototypes */
int exec_local_cmd_loop(void);
void print_dragon(void);

#endif
