#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include "dshlib.h"

// Helper: Trim leading and trailing whitespace.
static char *trim_whitespace(char *str) {
    while (isspace((unsigned char)*str)) str++;
    if (*str == '\0') return str;
    char *end = str + strlen(str) - 1;
    while(end > str && isspace((unsigned char)*end)) end--;
    *(end + 1) = '\0';
    return str;
}
Built_In_Cmds match_command(const char *input) {
    if (!input) return BI_NOT_BI;
    if (strcmp(input, EXIT_CMD) == 0) return BI_CMD_EXIT;
    if (strcmp(input, "cd") == 0) return BI_CMD_CD;
    if (strcmp(input, "dragon") == 0) return BI_CMD_DRAGON;
    return BI_NOT_BI;
}


//---------------------------
// Built-In Commands
//---------------------------
Built_In_Cmds exec_built_in_cmd(cmd_buff_t *cmd) {
    if (!cmd || !cmd->argv[0]) return BI_NOT_BI;
    switch(match_command(cmd->argv[0])) {
        case BI_CMD_EXIT:
            printf("exiting...\n");
            exit(EXIT_SC);
        case BI_CMD_CD: {
            char *dir = cmd->argv[1] ? cmd->argv[1] : getenv("HOME");
            if (chdir(dir) != 0)
                perror("cd failed");
            return BI_EXECUTED;
        }
        case BI_CMD_DRAGON: {
            // Fork a new process to run the external program "dragon"
            pid_t pid = fork();
            if (pid < 0) {
                perror("fork failed");
                return ERR_EXEC_CMD;
            }
            if (pid == 0) {
                // In child process, execute the "dragon" executable.
                // Adjust "./dragon" to "dragon" if it's in your PATH.
                execlp("./dragon", "dragon", (char *)NULL);
                perror("execlp failed");
                exit(ERR_EXEC_CMD);
            } else {
                int status;
                waitpid(pid, &status, 0);
            }
            return BI_EXECUTED;
        }
        default:
            return BI_NOT_BI;
    }
}


//---------------------------
// Memory Management
//---------------------------
int alloc_cmd_buff(cmd_buff_t *cmd_buff) {
    if (!cmd_buff) return ERR_MEMORY;
    cmd_buff->_cmd_buffer = malloc(SH_CMD_MAX * sizeof(char));
    if (!cmd_buff->_cmd_buffer) return ERR_MEMORY;
    cmd_buff->argc = 0;
    memset(cmd_buff->argv, 0, sizeof(cmd_buff->argv));
    cmd_buff->in_file = NULL;
    cmd_buff->out_file = NULL;
    cmd_buff->append = 0;
    return OK;
}

int free_cmd_buff(cmd_buff_t *cmd_buff) {
    if (!cmd_buff) return ERR_MEMORY;
    // Free each token stored in argv.
    for (int i = 0; i < cmd_buff->argc; i++) {
        if (cmd_buff->argv[i]) {
            free(cmd_buff->argv[i]);
            cmd_buff->argv[i] = NULL;
        }
    }
    cmd_buff->argc = 0;
    if (cmd_buff->_cmd_buffer) {
        free(cmd_buff->_cmd_buffer);
        cmd_buff->_cmd_buffer = NULL;
    }
    if (cmd_buff->in_file) {
        free(cmd_buff->in_file);
        cmd_buff->in_file = NULL;
    }
    if (cmd_buff->out_file) {
        free(cmd_buff->out_file);
        cmd_buff->out_file = NULL;
    }
    return OK;
}

int clear_cmd_buff(cmd_buff_t *cmd_buff) {
    if (!cmd_buff) return ERR_MEMORY;
    if (cmd_buff->_cmd_buffer)
        memset(cmd_buff->_cmd_buffer, 0, SH_CMD_MAX);
    // Free previously stored tokens.
    for (int i = 0; i < cmd_buff->argc; i++) {
        if (cmd_buff->argv[i]) {
            free(cmd_buff->argv[i]);
            cmd_buff->argv[i] = NULL;
        }
    }
    cmd_buff->argc = 0;
    if (cmd_buff->in_file) { free(cmd_buff->in_file); cmd_buff->in_file = NULL; }
    if (cmd_buff->out_file) { free(cmd_buff->out_file); cmd_buff->out_file = NULL; }
    cmd_buff->append = 0;
    return OK;
}

int close_cmd_buff(cmd_buff_t *cmd_buff) {
    return free_cmd_buff(cmd_buff);
}

//---------------------------
// Command Parsing (with Redirection)
//---------------------------
int build_cmd_buff(char *cmd_line, cmd_buff_t *cmd_buff) {
    if (!cmd_line || !cmd_buff) return ERR_MEMORY;
    if (cmd_buff->_cmd_buffer == NULL) {
        int rc = alloc_cmd_buff(cmd_buff);
        if (rc != OK) return rc;
    }
    clear_cmd_buff(cmd_buff);
    
    // Store the original command line.
    cmd_buff->_cmd_buffer = strdup(cmd_line);
    if (!cmd_buff->_cmd_buffer) return ERR_MEMORY;
    
    // Remove trailing newline.
    cmd_line[strcspn(cmd_line, "\n")] = '\0';
    if (strlen(cmd_line) == 0) return WARN_NO_CMDS;
    
    // Duplicate the line for tokenizing.
    char *line_dup = strdup(cmd_line);
    if (!line_dup) return ERR_MEMORY;
    
    char *rest = line_dup, *token;
    while ((token = strtok_r(rest, " \t", &rest))) {
        // Handle input redirection.
        if (strcmp(token, "<") == 0) {
            token = strtok_r(rest, " \t", &rest);
            if (!token) {
                fprintf(stderr, "error: no input file specified\n");
                free(line_dup);
                return ERR_CMD_ARGS_BAD;
            }
            cmd_buff->in_file = strdup(token);
            if (!cmd_buff->in_file) { free(line_dup); return ERR_MEMORY; }
        }
        // Handle output redirection (overwrite or append).
        else if (strcmp(token, ">") == 0 || strcmp(token, ">>") == 0) {
            cmd_buff->append = (strcmp(token, ">>") == 0) ? 1 : 0;
            token = strtok_r(rest, " \t", &rest);
            if (!token) {
                fprintf(stderr, "error: no output file specified\n");
                free(line_dup);
                return ERR_CMD_ARGS_BAD;
            }
            cmd_buff->out_file = strdup(token);
            if (!cmd_buff->out_file) { free(line_dup); return ERR_MEMORY; }
        }
        else {
            if (cmd_buff->argc >= CMD_ARGV_MAX - 1) {
                fprintf(stderr, "error: too many arguments\n");
                free(line_dup);
                return ERR_CMD_OR_ARGS_TOO_BIG;
            }
            // Duplicate token so that argv[] holds its own copy.
            char *dup_tok = strdup(token);
            if (!dup_tok) { free(line_dup); return ERR_MEMORY; }
            cmd_buff->argv[cmd_buff->argc++] = dup_tok;
        }
    }
    cmd_buff->argv[cmd_buff->argc] = NULL;
    free(line_dup);
    return OK;
}

int build_cmd_list(char *cmd_line, command_list_t *clist) {
    if (!cmd_line || !clist) return ERR_MEMORY;
    memset(clist, 0, sizeof(command_list_t));
    cmd_line[strcspn(cmd_line, "\n")] = '\0';
    if (strlen(cmd_line) == 0) return WARN_NO_CMDS;
    
    char *line_dup = strdup(cmd_line);
    if (!line_dup) return ERR_MEMORY;
    
    char *rest = line_dup, *token;
    while ((token = strtok_r(rest, PIPE_STRING, &rest)) && clist->num < CMD_MAX) {
        token = trim_whitespace(token);
        if (strlen(token) == 0) continue;
        int rc = build_cmd_buff(token, &clist->commands[clist->num]);
        if (rc != OK) {
            free(line_dup);
            return rc;
        }
        clist->num++;
    }
    free(line_dup);
    if (clist->num == 0) return WARN_NO_CMDS;
    return OK;
}

int free_cmd_list(command_list_t *cmd_lst) {
    if (!cmd_lst) return ERR_MEMORY;
    for (int i = 0; i < cmd_lst->num; i++) {
        free_cmd_buff(&cmd_lst->commands[i]);
    }
    memset(cmd_lst, 0, sizeof(command_list_t));
    return OK;
}

//---------------------------
// Command Execution Functions
//---------------------------
int exec_cmd(cmd_buff_t *cmd) {
    if (!cmd || !cmd->argv[0]) return ERR_CMD_OR_ARGS_TOO_BIG;
    
    // Handle built-in commands.
    if (match_command(cmd->argv[0]) != BI_NOT_BI)
        return exec_built_in_cmd(cmd);
    
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed");
        return ERR_EXEC_CMD;
    }
    if (pid == 0) {  // Child process
        if (cmd->in_file) {
            int fd_in = open(cmd->in_file, O_RDONLY);
            if (fd_in < 0) {
                perror("open input file failed");
                exit(ERR_EXEC_CMD);
            }
            dup2(fd_in, STDIN_FILENO);
            close(fd_in);
        }
        if (cmd->out_file) {
            int fd_out;
            if (cmd->append)
                fd_out = open(cmd->out_file, O_WRONLY | O_CREAT | O_APPEND, 0644);
            else
                fd_out = open(cmd->out_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd_out < 0) {
                perror("open output file failed");
                exit(ERR_EXEC_CMD);
            }
            dup2(fd_out, STDOUT_FILENO);
            close(fd_out);
        }
        execvp(cmd->argv[0], cmd->argv);
        perror("execvp failed");
        exit(ERR_EXEC_CMD);
    }
    else {
        int status;
        waitpid(pid, &status, 0);
    }
    return OK;
}

int execute_pipeline(command_list_t *clist) {
    if (!clist || clist->num == 0) return WARN_NO_CMDS;
    if (clist->num == 1)
        return exec_cmd(&clist->commands[0]);
    
    int num_cmds = clist->num, prev_fd = -1;
    pid_t pids[CMD_MAX];
    
    for (int i = 0; i < num_cmds; i++) {
        int pipe_fd[2] = { -1, -1 };
        if (i < num_cmds - 1) {
            if (pipe(pipe_fd) < 0) {
                perror("pipe failed");
                return ERR_EXEC_CMD;
            }
        }
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork failed");
            return ERR_EXEC_CMD;
        }
        if (pid == 0) {  // Child process
            if (prev_fd != -1) {
                dup2(prev_fd, STDIN_FILENO);
                close(prev_fd);
            }
            if (i < num_cmds - 1) {
                dup2(pipe_fd[1], STDOUT_FILENO);
                close(pipe_fd[0]);
                close(pipe_fd[1]);
            }
            // Redirection for individual command.
            cmd_buff_t *cmd = &clist->commands[i];
            if (cmd->in_file) {
                int fd_in = open(cmd->in_file, O_RDONLY);
                if (fd_in < 0) {
                    perror("open input file failed");
                    exit(ERR_EXEC_CMD);
                }
                dup2(fd_in, STDIN_FILENO);
                close(fd_in);
            }
            if (cmd->out_file) {
                int fd_out;
                if (cmd->append)
                    fd_out = open(cmd->out_file, O_WRONLY | O_CREAT | O_APPEND, 0644);
                else
                    fd_out = open(cmd->out_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (fd_out < 0) {
                    perror("open output file failed");
                    exit(ERR_EXEC_CMD);
                }
                dup2(fd_out, STDOUT_FILENO);
                close(fd_out);
            }
            execvp(cmd->argv[0], cmd->argv);
            perror("execvp failed");
            exit(ERR_EXEC_CMD);
        }
        else {
            pids[i] = pid;
            if (prev_fd != -1)
                close(prev_fd);
            if (i < num_cmds - 1) {
                close(pipe_fd[1]);
                prev_fd = pipe_fd[0];
            }
        }
    }
    for (int i = 0; i < num_cmds; i++) {
        int status;
        waitpid(pids[i], &status, 0);
    }
    return OK;
}

//---------------------------
// Main Shell Loop
//---------------------------
int exec_local_cmd_loop() {
    char cmd_line[SH_CMD_MAX];
    command_list_t cmd_list;
    
    while (1) {
        printf("%s", SH_PROMPT);
        if (fgets(cmd_line, sizeof(cmd_line), stdin) == NULL) {
            printf("\n");
            break;
        }
        if (strlen(trim_whitespace(cmd_line)) == 0) {
            fprintf(stderr, CMD_WARN_NO_CMD);
            continue;
        }
        char tmp[SH_CMD_MAX];
        strncpy(tmp, cmd_line, SH_CMD_MAX);
        tmp[strcspn(tmp, "\n")] = '\0';
        if (strcmp(trim_whitespace(tmp), EXIT_CMD) == 0) {
            printf("exiting...\n");
            break;
        }
        int rc = build_cmd_list(cmd_line, &cmd_list);
        if (rc == WARN_NO_CMDS) {
            fprintf(stderr, CMD_WARN_NO_CMD);
            continue;
        }
        if (rc == ERR_TOO_MANY_COMMANDS) {
            fprintf(stderr, CMD_ERR_PIPE_LIMIT, CMD_MAX);
            free_cmd_list(&cmd_list);
            continue;
        }
        if (cmd_list.num == 1)
            rc = exec_cmd(&cmd_list.commands[0]);
        else
            rc = execute_pipeline(&cmd_list);
        free_cmd_list(&cmd_list);
    }
    return OK;
}
