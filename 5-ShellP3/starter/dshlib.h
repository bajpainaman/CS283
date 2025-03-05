#ifndef __DSHLIB_H__
#define __DSHLIB_H__

// Constants for command structure sizes
#define EXE_MAX 64
#define ARG_MAX 256
#define CMD_MAX 8
#define CMD_ARGV_MAX (CMD_MAX + 1)
// Maximum size for a command line
#define SH_CMD_MAX (EXE_MAX + ARG_MAX)

// Command buffer structure – holds a single command’s tokens and redirection info.
typedef struct cmd_buff {
    int argc;
    char *argv[CMD_ARGV_MAX];
    char *_cmd_buffer;  // Stores the original command line (optional)
    // Extra Credit: redirection support
    char *in_file;   // for input redirection (<)
    char *out_file;  // for output redirection (> or >>)
    int  append;     // 0 for '>', 1 for '>>'
} cmd_buff_t;

// Command list structure – for handling piped commands.
typedef struct command_list {
    int num;
    cmd_buff_t commands[CMD_MAX];
} command_list_t;

// Special character defines
#define SPACE_CHAR  ' '
#define PIPE_CHAR   '|'
#define PIPE_STRING "|"

// Shell prompt and exit definitions
#define SH_PROMPT "dsh3> "
#define EXIT_CMD "exit"
#define EXIT_SC 99

// Standard return codes
#define OK                       0
#define WARN_NO_CMDS            -1
#define ERR_TOO_MANY_COMMANDS   -2
#define ERR_CMD_OR_ARGS_TOO_BIG -3
#define ERR_CMD_ARGS_BAD        -4  // for extra credit
#define ERR_MEMORY              -5
#define ERR_EXEC_CMD            -6
#define OK_EXIT                 -7

// Function prototypes for command buffer management
int alloc_cmd_buff(cmd_buff_t *cmd_buff);
int free_cmd_buff(cmd_buff_t *cmd_buff);
int clear_cmd_buff(cmd_buff_t *cmd_buff);
int build_cmd_buff(char *cmd_line, cmd_buff_t *cmd_buff);
int close_cmd_buff(cmd_buff_t *cmd_buff);
int build_cmd_list(char *cmd_line, command_list_t *clist);
int free_cmd_list(command_list_t *cmd_lst);

// Built-in command handling
typedef enum {
    BI_CMD_EXIT,
    BI_CMD_DRAGON,
    BI_CMD_CD,
    BI_NOT_BI,
    BI_EXECUTED,
} Built_In_Cmds;
Built_In_Cmds match_command(const char *input);
Built_In_Cmds exec_built_in_cmd(cmd_buff_t *cmd);

// Main execution functions
int exec_local_cmd_loop();
int exec_cmd(cmd_buff_t *cmd);
int execute_pipeline(command_list_t *clist);

// Output constants
#define CMD_OK_HEADER       "PARSED COMMAND LINE - TOTAL COMMANDS %d\n"
#define CMD_WARN_NO_CMD     "warning: no commands provided\n"
#define CMD_ERR_PIPE_LIMIT  "error: piping limited to %d commands\n"

#endif
