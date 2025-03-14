#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/un.h>
#include <fcntl.h>

// INCLUDES for extra credit (uncomment if needed)
// #include <signal.h>
// #include <pthread.h>

#include "dshlib.h"
#include "rshlib.h"

/*
 * start_server(ifaces, port, is_threaded)
 * - ifaces: IP address string (e.g., "0.0.0.0") to bind the server to.
 * - port: Port number to listen on (e.g., RDSH_DEF_PORT from rshlib.h).
 * - is_threaded: Flag for multi-threaded server (extra credit, unused here).
 * 
 * Runs the server by booting it, processing client requests, and stopping it
 * when requested via `stop-server`.
 */
int start_server(char *ifaces, int port, int is_threaded) {
    (void)is_threaded; // Silence unused parameter warning (for extra credit)

    int svr_socket = boot_server(ifaces, port);
    if (svr_socket < 0) {
        return svr_socket; // Propagate error code
    }

    int rc = process_cli_requests(svr_socket);
    stop_server(svr_socket);
    return rc;
}

/*
 * stop_server(svr_socket)
 * - svr_socket: The server socket descriptor to close.
 * 
 * Closes the server socket and returns the result of close().
 */
int stop_server(int svr_socket) {
    return close(svr_socket);
}

/*
 * boot_server(ifaces, port)
 * - ifaces: Interface to bind to.
 * - port: Port to listen on.
 * 
 * Sets up the server socket with socket(), bind(), and listen().
 * Returns the socket descriptor on success or ERR_RDSH_COMMUNICATION on failure.
 */
int boot_server(char *ifaces, int port) {
    int svr_socket;
    struct sockaddr_in addr;

    svr_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (svr_socket == -1) {
        perror("socket");
        return ERR_RDSH_COMMUNICATION;
    }

    // Allow port reuse during development
    int enable = 1;
    setsockopt(svr_socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ifaces);
    addr.sin_port = htons(port);

    if (bind(svr_socket, (const struct sockaddr *)&addr, sizeof(addr)) == -1) {
        perror("bind");
        close(svr_socket);
        return ERR_RDSH_COMMUNICATION;
    }

    if (listen(svr_socket, 20) == -1) {
        perror("listen");
        close(svr_socket);
        return ERR_RDSH_COMMUNICATION;
    }

    return svr_socket;
}

/*
 * process_cli_requests(svr_socket)
 * - svr_socket: Server socket to accept client connections.
 * 
 * Accepts client connections in a loop, processes requests, and stops on
 * `stop-server` or error.
 */
int process_cli_requests(int svr_socket) {
    int cli_socket;
    int rc = OK;

    while (1) {
        cli_socket = accept(svr_socket, NULL, NULL);
        if (cli_socket == -1) {
            perror("accept");
            rc = ERR_RDSH_COMMUNICATION;
            break;
        }

        rc = exec_client_requests(cli_socket);
        close(cli_socket);

        if (rc < 0) {
            if (rc == OK_EXIT) {
                break; // Client sent `stop-server`
            }
            break; // Other errors
        }
    }

    stop_server(svr_socket); // Close server socket, not cli_socket
    return rc;
}

/*
 * exec_client_requests(cli_socket)
 * - cli_socket: Client-specific socket for communication.
 * 
 * Receives and executes commands from the client in a loop until `exit` or
 * `stop-server` is received.
 */
int exec_client_requests(int cli_socket) {
    int rc = OK;
    char *io_buff;

    while (1) {
        io_buff = malloc(RDSH_COMM_BUFF_SZ);
        if (io_buff == NULL) {
            return ERR_RDSH_SERVER;
        }

        // Receive command from client
        int total = 0;
        while (total < RDSH_COMM_BUFF_SZ - 1) {
            int bytes = recv(cli_socket, io_buff + total, RDSH_COMM_BUFF_SZ - total - 1, 0);
            if (bytes <= 0) {
                free(io_buff);
                return ERR_RDSH_COMMUNICATION; // Client disconnected or error
            }
            total += bytes;
            io_buff[total] = '\0';
            if (total > 0 && io_buff[total - 1] == RDSH_EOF_CHAR) {
                io_buff[total - 1] = '\0';
                break;
            }
        }

        command_list_t cmd_list;
        build_cmd_list(io_buff, &cmd_list);

        // Handle built-in commands
        if (cmd_list.num > 0) {
            if (strcmp(cmd_list.commands[0].argv[0], "exit") == 0) {
                free(io_buff);
                return OK;
            }
            if (strcmp(cmd_list.commands[0].argv[0], "stop-server") == 0) {
                free(io_buff);
                return OK_EXIT;
            }
            if (strcmp(cmd_list.commands[0].argv[0], "cd") == 0 && cmd_list.commands[0].argc > 1) {
                if (chdir(cmd_list.commands[0].argv[1]) == -1) {
                    send_message_string(cli_socket, "cd failed");
                }
            } else {
                // Execute pipeline for non-built-in commands
                rc = rsh_execute_pipeline(cli_socket, &cmd_list);
                if (rc < 0) {
                    send_message_string(cli_socket, "error");
                }
            }
        }

        send_message_eof(cli_socket);
        free(io_buff);
    }

    return rc; // Shouldn't reach here due to infinite loop, but included for completeness
}

/*
 * send_message_eof(cli_socket)
 * - cli_socket: Client socket to send EOF to.
 * 
 * Sends the EOF character to signal command completion.
 */
int send_message_eof(int cli_socket) {
    int send_len = sizeof(RDSH_EOF_CHAR);
    int sent_len = send(cli_socket, &RDSH_EOF_CHAR, send_len, 0);

    if (sent_len != send_len) {
        return ERR_RDSH_COMMUNICATION;
    }
    return OK;
}

/*
 * send_message_string(cli_socket, buff)
 * - cli_socket: Client socket to send message to.
 * - buff: Null-terminated string to send.
 * 
 * Sends a string followed by an EOF character.
 */
int send_message_string(int cli_socket, char *buff) {
    if (send(cli_socket, buff, strlen(buff), 0) < 0) {
        return ERR_RDSH_COMMUNICATION;
    }
    return send_message_eof(cli_socket);
}

/*
 * rsh_execute_pipeline(cli_sock, clist)
 * - cli_sock: Client socket for I/O.
 * - clist: Command list to execute in a pipeline.
 * 
 * Executes a pipeline of commands, connecting STDIN/STDOUT/STDERR to cli_sock
 * as needed.
 */
int rsh_execute_pipeline(int cli_sock, command_list_t *clist) {
    int pipes[clist->num - 1][2];
    pid_t pids[clist->num];
    int pids_st[clist->num];
    int exit_code;

    // Create pipes
    for (int i = 0; i < clist->num - 1; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("pipe");
            return -1;
        }
    }

    // Fork and execute each command
    for (int i = 0; i < clist->num; i++) {
        pids[i] = fork();
        if (pids[i] == -1) {
            perror("fork");
            return -1;
        }

        if (pids[i] == 0) { // Child process
            if (i == 0) {
                dup2(cli_sock, STDIN_FILENO);
            }
            if (i > 0) {
                dup2(pipes[i - 1][0], STDIN_FILENO);
            }
            if (i < clist->num - 1) {
                dup2(pipes[i][1], STDOUT_FILENO);
            }
            if (i == clist->num - 1) {
                dup2(cli_sock, STDOUT_FILENO);
                dup2(cli_sock, STDERR_FILENO);
            }

            // Close all pipe ends in child
            for (int j = 0; j < clist->num - 1; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }

            execvp(clist->commands[i].argv[0], clist->commands[i].argv);
            perror("execvp");
            exit(EXIT_FAILURE);
        }
    }

    // Parent: close all pipe ends
    for (int i = 0; i < clist->num - 1; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    // Wait for children
    for (int i = 0; i < clist->num; i++) {
        waitpid(pids[i], &pids_st[i], 0);
    }

    // Return exit code of last process
    exit_code = WEXITSTATUS(pids_st[clist->num - 1]);
    for (int i = 0; i < clist->num; i++) {
        if (WEXITSTATUS(pids_st[i]) == EXIT_SC) {
            exit_code = EXIT_SC;
        }
    }
    return exit_code;
}

/* Optional Built-in Command Handling (unchanged) */
Built_In_Cmds rsh_match_command(const char *input) {
    if (strcmp(input, "exit") == 0) return BI_CMD_EXIT;
    if (strcmp(input, "dragon") == 0) return BI_CMD_DRAGON;
    if (strcmp(input, "cd") == 0) return BI_CMD_CD;
    if (strcmp(input, "stop-server") == 0) return BI_CMD_STOP_SVR;
    if (strcmp(input, "rc") == 0) return BI_CMD_RC;
    return BI_NOT_BI;
}

Built_In_Cmds rsh_built_in_cmd(cmd_buff_t *cmd) {
    Built_In_Cmds ctype = rsh_match_command(cmd->argv[0]);

    switch (ctype) {
    case BI_CMD_EXIT:
        return BI_CMD_EXIT;
    case BI_CMD_STOP_SVR:
        return BI_CMD_STOP_SVR;
    case BI_CMD_RC:
        return BI_CMD_RC;
    case BI_CMD_CD:
        if (cmd->argc > 1) chdir(cmd->argv[1]);
        return BI_EXECUTED;
    default:
        return BI_NOT_BI;
    }
}