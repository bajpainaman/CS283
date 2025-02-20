### **1. Why use `fork()` and `execvp()` instead of just `execvp()` directly? What value does `fork()` provide?**  
> **Answer**: `fork()` allows the shell to create a child process that runs the command separately while keeping the shell itself running. If we used `execvp()` directly, the shell process would be replaced by the new program, preventing the user from executing further commands. Using `fork()` ensures that the shell remains active, waiting for the child process to complete before accepting new commands.

---

### **2. What happens if the `fork()` system call fails? How does your implementation handle this scenario?**  
> **Answer**: If `fork()` fails, it returns `-1`, which means the system was unable to create a new process, often due to resource limitations. My implementation checks for this error and prints an error message using `perror("fork failed")`, ensuring that the shell does not crash and continues to accept commands.

---

### **3. How does `execvp()` find the command to execute? What system environment variable plays a role in this process?**  
> **Answer**: `execvp()` searches for the specified command in the directories listed in the `PATH` environment variable. The `PATH` variable contains a colon-separated list of directories where executable files are located (e.g., `/bin`, `/usr/bin`). If `execvp()` finds the command in one of these directories, it executes it; otherwise, it returns an error indicating that the command was not found.

---

### **4. Why call `wait()` in the parent process after forking? What happens if we don’t call it?**  
> **Answer**: `wait()` ensures that the parent process (the shell) waits for the child process to complete execution before continuing. If we didn’t call `wait()`, the child process would become a **zombie process**, remaining in the process table until the parent collects its exit status. This could lead to resource exhaustion if many zombie processes accumulate.

---

### **5. What information does `WEXITSTATUS()` provide, and why is it important?**  
> **Answer**: `WEXITSTATUS()` extracts the exit status of a terminated child process from the value returned by `wait()`. This is important because it allows the shell to check whether the executed command was successful or failed. Typically, an exit status of `0` indicates success, while a nonzero value indicates an error.

---

### **6. How does `build_cmd_buff()` handle quoted arguments? Why is this necessary?**  
> **Answer**: My implementation treats quoted arguments as a single entity, preserving spaces inside quotes. It identifies quoted strings and ensures they are stored as one argument instead of being split by spaces. This is necessary for handling commands like `echo "hello world"`, ensuring `"hello world"` is passed as a single argument rather than two separate ones.

---

### **7. What changes did you make to your parsing logic compared to the previous assignment? Any unexpected challenges in refactoring?**  
> **Answer**: I modified the parsing logic to focus on handling individual commands without supporting pipes (since pipes are introduced in a later assignment). One of the main challenges was refactoring the logic to properly handle quoted arguments and eliminate redundant spaces while maintaining compatibility with `execvp()`. Another challenge was ensuring that commands were parsed correctly without breaking existing functionality.

---

### **8. What is the purpose of signals in a Linux system, and how do they differ from other forms of interprocess communication (IPC)?**  
> **Answer**: Signals provide a mechanism for processes to receive asynchronous notifications about events such as termination requests (`SIGTERM`), interrupts (`SIGINT`), or segmentation faults (`SIGSEGV`). Unlike other forms of interprocess communication (IPC) like pipes or shared memory, signals do not require explicit message passing and are handled directly by the kernel.

---

### **9. Describe three commonly used signals (`SIGKILL`, `SIGTERM`, `SIGINT`). What are their typical use cases?**  
> **Answer**:  
> - **SIGKILL (`kill -9 <pid>`)**: Immediately terminates a process and cannot be caught or ignored. Used when a process is unresponsive.  
> - **SIGTERM (`kill <pid>`)**: Gracefully asks a process to terminate, allowing it to clean up resources before exiting.  
> - **SIGINT (`Ctrl+C`)**: Sent by the terminal to interrupt a running process, often used to stop programs running in the foreground.

---

### **10. What happens when a process receives `SIGSTOP`? Can it be caught or ignored like `SIGINT`? Why or why not?**  
> **Answer**: When a process receives `SIGSTOP`, it is paused (suspended) and cannot continue execution until resumed with `SIGCONT`. Unlike `SIGINT`, `SIGSTOP` **cannot be caught, ignored, or blocked** because it is handled directly by the kernel. This ensures that processes can be forcefully stopped for debugging or administrative purposes.

---