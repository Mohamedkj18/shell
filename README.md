Shell Program
Overview
This project implements a basic shell program in C. It provides functionality to execute commands, handle input/output redirection, pipe commands, and manage background processes. The implementation includes two main source files:

myshell.c: Implements core functionalities like handling redirection, piping, and background processes.
shell.c: Provides the main program logic, including initialization, user input parsing, and cleanup.
Features
Command Execution: Executes simple commands provided by the user.
Redirection:
Input redirection (<): Reads input from a file.
Output redirection (>): Writes output to a file.
Piping (|): Supports single-pipe operations.
Background Processes (&): Allows commands to run in the background.
Signal Handling:
Handles SIGINT and SIGCHLD appropriately for interactive shell behavior.
File Descriptions
myshell.c
This file contains the core implementation of command processing:

prepare(): Initializes signal handling for the shell.
finalize(): Cleans up any resources when the shell exits.
process_arglist(): Processes and executes user commands, supporting features like piping and redirection.
single_piping(): Handles the execution of two commands connected via a pipe.
shell.c
This file contains the main program loop:

Main Functionality:
Reads user input.
Parses input into commands and arguments.
Calls process_arglist() to handle execution.
Manages memory and ensures proper cleanup.
prepare() and finalize(): Invokes initialization and cleanup routines.
How to Compile
To compile the shell program, use the following command:

bash
Copy code
gcc -o shell shell.c myshell.c
How to Run
Run the compiled program:

bash
Copy code
./shell
You can then input commands like:

ls -l
cat file.txt > output.txt
grep 'pattern' < file.txt | sort
sleep 10 &
Limitations
Supports only single piping (e.g., command1 | command2).
Limited error handling for certain edge cases.
Background processes (&) do not display completion notifications.
Future Enhancements
Add support for multi-pipe commands.
Improve error reporting and handling.
Implement more advanced job control (e.g., fg, bg, jobs).
License
This project is provided as-is without any warranty. Feel free to use and modify for educational purposes.
