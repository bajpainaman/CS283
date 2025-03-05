# Code Grading Instructions

- dshlib.c must implement a exec_local_cmd_loop() func that:
  - runs a loop collecting user input
  - parses commands and arguments from each line of input
  - parsing must handle pipes - all commands must be split by pipes into multiple commands
  - determines if the parsed command is an "external command"
- the parsed commands must:
  - have trailing and leading whitespace removed
  - must preserve whitepsace between double quotes (i.e. perserve echo "hello     world")
  - must treat double-quoted string as a single argument
- the code must implement "cd" as an internal command
  - if cd is provided with arguments, attmpt to change the current process working directory to the provided argument
  - if no argument is provided to cd, it should do nothing
- the code must implement "exit" as an internal command, exiting with a return code of 0
- for all other commands, they should be treated as external and executed with fork/exec
  - execution must be pipelined if there is more than one command
  - the solution should use the pipe() syscall to create pipes and connect the commands
  - the first command in the pipeline takes input from stdin, and the last command in the pipeline writes to stdout
- students are not required to implement redirection or pipes in this assignment
- extra credit, these are optional requirements:
  - extra credit 1 - implement < and > for redirection bewteen commands
  - extra credit 2 - implement >> append redirection

# Possible Grade

- 50 points: Correct implementation of required functionality
- 5 points:  Code quality (how easy is your solution to follow)
- 10 points: Quality and breadth of BATS unit tests
- 10 points: [EXTRA CREDIT 1] handle < and > redirection
- 5 points: [EXTRA CREDIT 2] handle >> append redirection