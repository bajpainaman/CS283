#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dshlib.h"

/*
 * Implement your main function by building a loop that prompts the
 * user for input.  Use the SH_PROMPT constant and then
 * use fgets to accept user input. Also handle EOF.
 *
 * Then parse the input using build_cmd_list.
 * If the user typed "exit", we exit the shell.
 * If the user typed "dragon", we print the ASCII dragon (extra credit).
 */
int main()
{
    char cmd_buff[ARG_MAX];
    command_list_t clist;
    int rc;

    while (1)
    {
        printf("%s", SH_PROMPT);

        if (fgets(cmd_buff, ARG_MAX, stdin) == NULL) {
            printf("\n");
            break; // EOF => exit
        }
        cmd_buff[strcspn(cmd_buff, "\n")] = '\0';

        // built-in "exit"
        if (strcmp(cmd_buff, EXIT_CMD) == 0) {
            exit(0);
        }

        // Extra credit "dragon"
        if (strcmp(cmd_buff, "dragon") == 0) {
            print_dragon();
            continue;
        }

        memset(&clist, 0, sizeof(clist));
        rc = build_cmd_list(cmd_buff, &clist);

        switch (rc)
        {
            case OK:
                printf(CMD_OK_HEADER, clist.num);
                for (int i = 0; i < clist.num; i++) {
                    if (strlen(clist.commands[i].args) == 0) {
                        printf("<%d> %s\n", i + 1, clist.commands[i].exe);
                    } else {
                        printf("<%d> %s [%s]\n", i + 1,
                               clist.commands[i].exe,
                               clist.commands[i].args);
                    }
                }
                break;

            case WARN_NO_CMDS:
                // user typed a blank line
                printf(CMD_WARN_NO_CMD);
                break;

            case ERR_TOO_MANY_COMMANDS:
                // more than 8 commands with '|'
                printf(CMD_ERR_PIPE_LIMIT, CMD_MAX);
                break;

            case ERR_CMD_OR_ARGS_TOO_BIG:
                // exe or args too large
                fprintf(stderr, "error: command or args too big\n");
                break;

            default:
                // any other codes
                break;
        }
    }

    return 0;
}