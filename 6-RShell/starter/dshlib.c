#include "dshlib.h"

void print_dragon() {
    // Placeholder; implement if needed
    printf("Dragon placeholder\n");
}

int build_cmd_buff(char *cmd_line, cmd_buff_t *cmd_buff) {
    char *token;
    char *cmd = malloc(SH_CMD_MAX);
    char *md = malloc(SH_CMD_MAX);
    if (!cmd || !md) {
        free(cmd);
        free(md);
        return -1; // Memory allocation failure
    }

    token = cmd_line;
    while (isspace((unsigned char)*token)) token++;

    cmd = token + strlen(token) - 1;
    while (cmd > token && isspace((unsigned char)*cmd)) cmd--;
    cmd[1] = '\0';

    bool in_quote_mode = false;
    bool in_black_mode = false;
    int index = 0;
    for (size_t i = 0; i < strlen(token); i++) {
        if (in_quote_mode) {
            if (token[i] == '"') {
                in_quote_mode = false;
                md[index++] = token[i];
                in_black_mode = false;
            } else {
                md[index++] = token[i];
            }
        } else {
            if (token[i] == '"') {
                in_quote_mode = true;
                md[index++] = token[i];
            } else if (token[i] == SPACE_CHAR) {
                if (!in_black_mode) {
                    in_black_mode = true;
                    md[index++] = ' ';
                }
            } else {
                in_black_mode = false;
                md[index++] = token[i];
            }
        }
    }
    md[index] = '\0';

    cmd_buff->_cmd_buffer = malloc(SH_CMD_MAX);
    if (!cmd_buff->_cmd_buffer) {
        free(cmd);
        free(md);
        return -1;
    }
    strcpy(cmd_buff->_cmd_buffer, md);

    int start = 0;
    in_quote_mode = false;
    in_black_mode = false;
    bool just_exit = false;
    cmd_buff->argc = 0;

    for (size_t i = 0; i < strlen(md); i++) {
        if (in_quote_mode) {
            if (md[i] == '"') {
                in_quote_mode = false;
                cmd_buff->argv[cmd_buff->argc] = malloc(SH_CMD_MAX);
                strncpy(cmd_buff->argv[cmd_buff->argc], md + start + 1, i - start - 1);
                cmd_buff->argv[cmd_buff->argc][i - start - 1] = '\0';
                start = i + 1;
                cmd_buff->argc++;
                in_black_mode = false;
                just_exit = true;
            }
        } else {
            if (md[i] == '"') {
                in_quote_mode = true;
            } else if (md[i] == SPACE_CHAR) {
                in_black_mode = true;
                if (!just_exit) {
                    cmd_buff->argv[cmd_buff->argc] = malloc(SH_CMD_MAX);
                    strncpy(cmd_buff->argv[cmd_buff->argc], md + start, i - start);
                    cmd_buff->argv[cmd_buff->argc][i - start] = '\0';
                    start = i + 1;
                    cmd_buff->argc++;
                }
                just_exit = false;
                start = i + 1;
            }
        }
    }

    if (in_black_mode || cmd_buff->argc == 0) {
        cmd_buff->argv[cmd_buff->argc] = malloc(SH_CMD_MAX);
        strncpy(cmd_buff->argv[cmd_buff->argc], md + start, strlen(md) - start);
        cmd_buff->argv[cmd_buff->argc][strlen(md) - start] = '\0';
        cmd_buff->argc++;
    }
    cmd_buff->argv[cmd_buff->argc] = NULL;

    free(cmd);
    free(md);
    return OK;
}

int build_cmd_list(char *cmd_line, command_list_t *clist) {
    char *token;
    char *cmd_line_copy = malloc(strlen(cmd_line) + 1);
    if (!cmd_line_copy) return -1;
    strcpy(cmd_line_copy, cmd_line);

    clist->num = 0;
    if (strlen(cmd_line) >= SH_CMD_MAX) {
        free(cmd_line_copy);
        return ERR_CMD_OR_ARGS_TOO_BIG;
    }

    token = strtok(cmd_line_copy, "|");
    while (token != NULL) {
        clist->num++;
        while (isspace((unsigned char)*token)) token++;

        char *cmd = token + strlen(token) - 1;
        while (cmd > token && isspace((unsigned char)*cmd)) cmd--;
        cmd[1] = '\0';

        if (clist->num > CMD_MAX) {
            free(cmd_line_copy);
            return ERR_TOO_MANY_COMMANDS;
        }

        build_cmd_buff(token, &clist->commands[clist->num - 1]);
        token = strtok(NULL, "|");
    }

    free(cmd_line_copy);
    return OK;
}

int execute_pipeline(command_list_t *clist) {
    int pipes[clist->num - 1][2];
    pid_t pids[clist->num];
    int status[clist->num];

    for (int i = 0; i < clist->num - 1; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("pipe");
            return -1;
        }
    }

    for (int i = 0; i < clist->num; i++) {
        pids[i] = fork();
        if (pids[i] == -1) {
            perror("fork");
            return -1;
        }

        if (pids[i] == 0) {
            if (i > 0) dup2(pipes[i - 1][0], STDIN_FILENO);
            if (i < clist->num - 1) dup2(pipes[i][1], STDOUT_FILENO);

            for (int j = 0; j < clist->num - 1; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }

            execvp(clist->commands[i].argv[0], clist->commands[i].argv);
            perror("execvp");
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < clist->num - 1; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    for (int i = 0; i < clist->num; i++) {
        waitpid(pids[i], &status[i], 0);
    }

    return WEXITSTATUS(status[clist->num - 1]);
}

int exec_local_cmd_loop(void) {
    char *cmd_buff = malloc(SH_CMD_MAX);
    if (!cmd_buff) return -1;

    command_list_t command;

    while (1) {
        printf("%s", SH_PROMPT);
        if (fgets(cmd_buff, SH_CMD_MAX, stdin) == NULL) {
            printf("\n");
            break;
        }
        cmd_buff[strcspn(cmd_buff, "\n")] = '\0';

        if (*cmd_buff == 0) {
            printf(CMD_WARN_NO_CMD);
        } else {
            int return_code = build_cmd_list(cmd_buff, &command);
            if (return_code == ERR_TOO_MANY_COMMANDS) {
                printf(CMD_ERR_PIPE_LIMIT, CMD_MAX);
            } else if (return_code == OK) {
                if (strcmp(command.commands[0].argv[0], EXIT_CMD) == 0) {
                    free(cmd_buff);
                    printf("exiting\n");
                    return OK;
                } else if (strcmp(command.commands[0].argv[0], "cd") == 0) {
                    if (command.commands[0].argc > 1) {
                        if (chdir(command.commands[0].argv[1]) == -1) {
                            perror("cd");
                        }
                    }
                } else {
                    pid_t supervisor = fork();
                    if (supervisor == -1) {
                        perror("fork supervisor");
                        free(cmd_buff);
                        return -1;
                    }

                    if (supervisor == 0) {
                        execute_pipeline(&command);
                        exit(EXIT_SUCCESS);
                    }

                    waitpid(supervisor, NULL, 0);
                }
            }
        }
    }

    free(cmd_buff);
    return OK;
}