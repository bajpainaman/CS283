#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include "dshlib.h"

/* Global variable to hold the last command's exit code (for "rc" built-in) */
static int last_exit_code = 0;

/*
 * parse_input:
 *   Tokenizes the input string into a cmd_buff_t.
 *   Supports tokens inside double quotes (preserving spaces inside quotes).
 */
static void parse_input(char *input, cmd_buff_t *cmd) {
    cmd->argc = 0;
    cmd->_cmd_buffer = strdup(input);
    if (!cmd->_cmd_buffer) {
        fprintf(stderr, "Memory allocation error\n");
        exit(ERR_MEMORY);
    }
    
    char *p = cmd->_cmd_buffer;
    while (*p != '\0') {
        /* Skip leading whitespace */
        while (*p && isspace((unsigned char)*p)) p++;
        if (*p == '\0') break;
        
        if (*p == '"') {
            p++;  /* Skip opening quote */
            char *start = p;
            while (*p && *p != '"') p++;
            if (*p == '"') {
                *p = '\0';  /* Terminate token */
                p++;        /* Skip closing quote */
            }
            cmd->argv[cmd->argc++] = start;
        } else {
            char *start = p;
            while (*p && !isspace((unsigned char)*p)) p++;
            if (*p) {
                *p = '\0';
                p++;
            }
            cmd->argv[cmd->argc++] = start;
        }
        if (cmd->argc >= CMD_ARGV_MAX - 1) break;
    }
    cmd->argv[cmd->argc] = NULL;
}

/*
 * exec_local_cmd_loop:
 *   Main shell loop.
 */
int exec_local_cmd_loop() {
    char input[ARG_MAX];
    cmd_buff_t cmd;
    int rc = OK;
    bool interactive = isatty(STDIN_FILENO);
    
    if (interactive) {
        printf("%s", SH_PROMPT);
        fflush(stdout);
    }
    
    while (fgets(input, sizeof(input), stdin) != NULL) {
        /* Remove trailing newline */
        input[strcspn(input, "\n")] = '\0';
        
        if (strlen(input) == 0) {
            printf(CMD_WARN_NO_CMD);
        } else if (strcmp(input, EXIT_CMD) == 0) {
            break;
        } else if (strcmp(input, "dragon") == 0) {
            print_dragon();
        } else if (strcmp(input, "rc") == 0) {
            printf("%d\n", last_exit_code);
        } else if (strncmp(input, "cd", 2) == 0) {
            parse_input(input, &cmd);
            if (cmd.argc == 1) {
                /* No argument: do nothing */
            } else if (cmd.argc == 2) {
                if (chdir(cmd.argv[1]) != 0) {
                    /* Silently fail to match expected output */
                    last_exit_code = errno;
                }
            } else {
                fprintf(stderr, "cd: too many arguments\n");
            }
            free(cmd._cmd_buffer);
        } else {
            parse_input(input, &cmd);
            if (cmd.argc == 0) {
                printf(CMD_WARN_NO_CMD);
                free(cmd._cmd_buffer);
            } else {
                pid_t pid = fork();
                if (pid < 0) {
                    perror("fork failed");
                    free(cmd._cmd_buffer);
                } else if (pid == 0) {
                    /* Child process: execute external command */
                    execvp(cmd.argv[0], cmd.argv);
                    int err = errno;
                    if (err == ENOENT)
                        fprintf(stderr, "Command not found in PATH\n");
                    else if (err == EACCES)
                        fprintf(stderr, "Permission denied\n");
                    else
                        perror(CMD_ERR_EXECUTE);
                    exit(err);
                } else {
                    /* Parent process: wait for child */
                    int status;
                    waitpid(pid, &status, 0);
                    if (WIFEXITED(status))
                        last_exit_code = WEXITSTATUS(status);
                    else
                        last_exit_code = 1;
                }
                free(cmd._cmd_buffer);
            }
        }
        
        /* Print prompt on a new line after processing the command */
        printf("%s", SH_PROMPT);
        fflush(stdout);
    }
    
    /* When EOF is reached, print final prompt once */
    printf("%s", SH_PROMPT);
    fflush(stdout);
    
    return rc;
}
