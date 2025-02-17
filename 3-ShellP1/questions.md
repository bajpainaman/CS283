1. `fgets()` is a good choice because it reads user input safely, preventing buffer overflow, and ensures that the input includes newline termination handling, which is crucial for command-line interfaces.

2. We use `malloc()` for `cmd_buff` because shell commands can vary in length, and dynamic memory allocation allows us to handle varying input sizes flexibly without wasting stack memory.

3. Trimming spaces ensures correct command execution. Extra spaces could lead to incorrect parsing or execution failures when passing arguments to system commands.

4. Three redirection examples:
   - `command > output.txt` (redirects STDOUT to a file)
   - `command < input.txt` (reads input from a file)
   - `command 2> error.log` (redirects STDERR to a file)
   Challenges include properly handling file descriptors and ensuring correct redirection without affecting other shell processes.

5. Piping (`cmd1 | cmd2`) connects output of one command to the input of another, while redirection (`cmd > file`) saves output to a file. Pipes handle inter-process communication while redirection modifies file I/O.

6. Separating STDERR from STDOUT ensures errors are visible and do not interfere with expected output, making debugging easier.

7. Our shell should handle errors by displaying meaningful messages. We should also allow merging STDERR and STDOUT using `2>&1` for debugging purposes.
