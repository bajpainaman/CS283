#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

#include "dshlib.h"



#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

#include "dshlib.h"

/**** 
 **** FOR REMOTE SHELL USE YOUR SOLUTION FROM SHELL PART 3 HERE
 **** THE MAIN FUNCTION CALLS THIS ONE AS ITS ENTRY POINT TO
 **** EXECUTE THE SHELL LOCALLY
 ****
 */


 void print_dragon();

 int build_cmd_buff(char *cmd_line, cmd_buff_t *cmd_buff)
 {
     char *token;
     char *cmd = malloc(SH_CMD_MAX);
     char *md = malloc(SH_CMD_MAX);
     //int number = 0;
     token = cmd_line;
     
     // removes leading white space 
     while (isspace((unsigned char)*token)) {
         token++;
     }
     
     // removes trailing white space 
     cmd = token + strlen(token) - 1;
     while (cmd > token && isspace((unsigned char)*cmd)) {
         cmd--;
     }
     cmd[1] = '\0';
     
     // removes all the extra white space betweeen the commands but not in quotes
     bool in_quote_mode = false;
     bool in_black_mode = false; 
     int index = 0; 
     for (size_t i =0; i < strlen(token); i++ ){
         
         if (in_quote_mode){
             // end of a quote
             if(token[i] == '"'){
                 in_quote_mode = !in_quote_mode;
                 md[index] = token[i];
                 index += 1;
                 in_black_mode = false; 
 
             }else{
                 md[index] = token[i];
                 index += 1;
             }
             
         }else{
             // checks for quotes
             if(token[i] == '"'){
                 in_quote_mode = !in_quote_mode;
                 md[index] = token[i];
                 index += 1; 
             }else {
                 // checks for black space 
                 if (token[i] == SPACE_CHAR){
                     if (!in_black_mode){
                         in_black_mode = true; 
                         md[index] = ' ';
                         index += 1; 
                     }          
                 // check for non black space 
                 }else{
                     in_black_mode = false;
                     md[index] = token[i];
                     index += 1; 
                 }
             }
         }
     }
    
     // copy the string into _cmd_buffer
     cmd_buff->_cmd_buffer = malloc(SH_CMD_MAX);
     strcpy(cmd_buff->_cmd_buffer, md);
 
     int start = 0;
     in_quote_mode = false;
     in_black_mode = false;
     bool just_exit = false;
     cmd_buff->argc = 0;
 
     // copys the args from _cmd_buffer 
     for (size_t i = 0; i < strlen(md); i++){
         if (in_quote_mode){
             // end of a quote 
             if(md[i] == '"'){
                 in_quote_mode = !in_quote_mode;
                 // copy to arg 
                 cmd_buff->argv[cmd_buff->argc] = malloc(SH_CMD_MAX);
                 strncpy(cmd_buff->argv[cmd_buff->argc], md+start+1, i-start-1);
                 start = i + 1;
                 cmd_buff->argc += 1;
                 in_black_mode = false;
                 just_exit = true;
 
             }
     
         }else{
             if(md[i] == '"'){
                 in_quote_mode = !in_quote_mode;
                
             }else {
                 if (md[i] == SPACE_CHAR){
                     in_black_mode = true;
                     if (!just_exit){
 
                         // copy to arg 
                         cmd_buff->argv[cmd_buff->argc] = malloc(SH_CMD_MAX);
                         strncpy(cmd_buff->argv[cmd_buff->argc], md+start, i-start);
                         cmd_buff->argv[cmd_buff->argc][i-start] = '\0';
                         start = i + 1;
                         cmd_buff->argc += 1;
                         
                     }
                     start = i + 1;
                              
                 }
             }
         }
     }
     // last item in the string does to have a black space after it so we need to copy it here 
     if (in_black_mode || cmd_buff->argc == 0){
         cmd_buff->argv[cmd_buff->argc] = malloc(SH_CMD_MAX);
         strncpy(cmd_buff->argv[cmd_buff->argc], md+start, strlen(md)-start);
         cmd_buff->argv[cmd_buff->argc][ strlen(md)-start] = '\0';
         cmd_buff->argc += 1; 
     
     }
     cmd_buff->argv[cmd_buff->argc] = NULL; 
     
     
     return OK;
     
 }
 
 
 int build_cmd_list(char *cmd_line, command_list_t *clist)
 {
     char *token;
     char *cmd;
     char *prev_pos = malloc(strlen(cmd_line) + 1);
     strcpy(prev_pos, cmd_line);
  
   
     clist->num = 0;
     if (strlen(cmd_line) >= SH_CMD_MAX) {
         return ERR_CMD_OR_ARGS_TOO_BIG;
     }
 
     token = strtok(cmd_line, "|");
     //token = strtok_r(cmd_line,PIPE_STRING, &end_str);
     while (token != NULL){
      
         clist->num += 1; 
         
         
         // remove leading white space 
         while (isspace((unsigned char)*token)) {
             token++;
            
             
         }
         // remove tailing white space 
         cmd = token + strlen(token) - 1;
         while (cmd > token && isspace((unsigned char)*cmd)) cmd--;
         cmd[1] = '\0';
 
         if (clist->num > CMD_MAX) {
             return ERR_TOO_MANY_COMMANDS;
         }
             
         build_cmd_buff(token, &clist->commands[ clist->num -1]);
        
         token = strtok(NULL, "|" );
       
 
     }
     return OK;
    
 }
 
 int execute_pipeline(command_list_t *clist) {
         int pipes[clist->num - 1][2]; // File descriptors for pipes
         pid_t pids[clist->num];      // Track child PIDs
       
         // Create all necessary pipes
     for (int i = 0; i < clist->num - 1; i++) {
         if (pipe(pipes[i]) == -1) {
             perror("pipe");
             exit(EXIT_FAILURE);
         }
     }
 
     // Create processes for each command
     for (int i = 0; i < clist->num; i++) {
         pids[i] = fork();
         if (pids[i] == -1) {
             perror("fork");
             exit(EXIT_FAILURE);
         }
 
         if (pids[i] == 0) {  // Child process
             // Set up input pipe for all except first process
             if (i > 0) {
                 dup2(pipes[i-1][0], STDIN_FILENO);
             }
 
             // Set up output pipe for all except last process
             if (i < clist->num - 1) {
                 dup2(pipes[i][1], STDOUT_FILENO);
             }
 
             // Close all pipe ends in child
             for (int j = 0; j < clist->num - 1; j++) {
                 close(pipes[j][0]);
                 close(pipes[j][1]);
             }
 
             // Execute command
             
             execvp(clist->commands[i].argv[0], clist->commands[i].argv);
             perror("execvp");
             exit(EXIT_FAILURE);
         }
     }
 
     // Parent process: close all pipe ends
     for (int i = 0; i < clist->num - 1; i++) {
         close(pipes[i][0]);
         close(pipes[i][1]);
     }
 
     // Wait for all children
     for (int i = 0; i < clist->num; i++) {
         waitpid(pids[i], NULL, 0);
     }
     return OK;
 }
 
 
 int exec_local_cmd_loop()
 {
     char *cmd_buff = malloc(SH_CMD_MAX);
     //int rc = 0;
     
     command_list_t command;
 
     while(1){
         //cmd_buff_t cmd;
         int return_code;
         printf("%s", SH_PROMPT);
         if (fgets(cmd_buff, ARG_MAX, stdin) == NULL){
             printf("\n");
             break;
         }
         //remove the trailing \n from cmd_buff
         cmd_buff[strcspn(cmd_buff,"\n")] = '\0';
     
         //IMPLEMENT THE REST OF THE REQUIREMENTS
         if(*cmd_buff == 0){
             printf(CMD_WARN_NO_CMD);
         }else{
             
             return_code = build_cmd_list(cmd_buff, &command);
 
             
             if (return_code == -2 ){
                 printf( CMD_ERR_PIPE_LIMIT , CMD_MAX);
             }else if (return_code == 0){
                 // exit 
                 if (strcmp(command.commands[0].argv[0], EXIT_CMD) == 0 ) {
                     free(cmd_buff); 
                     printf("exiting");
                     return OK;
                 }
                 //dragon
                 /*
                 else if (strcmp(command.commands[0].argv[0], "dragon") == 0 ) {
                     print_dragon();
 
                 }
                 */
                 // runs cd 
                 else if (strcmp(command.commands[0].argv[0], "cd") == 0){
 
                     if (command.num >= 1){
 
                         chdir(command.commands[0].argv[1]);
                     }
                     
                 }else{
                     pid_t supervisor = fork();
                     if (supervisor == -1) {
                         perror("fork supervisor");
                         exit(EXIT_FAILURE);
                     }
 
                     if (supervisor == 0) {  // Supervisor process
                         execute_pipeline(&command);
                         exit(EXIT_SUCCESS);
                     }
 
                     // Main parent just waits for supervisor
                     waitpid(supervisor, NULL, 0);
                    
                 }
 
        
             }
 
         
 
 
 
         }
 
                 
         
         
     }
 
     
     return OK;
 }
  