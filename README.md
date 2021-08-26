# Simple shell
This is a project in my school (Operating system course), we will implement a simple shell in Linux.

This project consists of designing a C program to serve as a shell interface that accepts user commands
and then executes each command in a separate process. Your implementation will support input and
output redirection, as well as pipes as a form of IPC between a pair of commands. Completing this project
will involve using the UNIX fork(), exec(), wait(), dup2(), and pipe() system calls and can be completed
on Linux system.

# Features

This project is organized into several parts:
1. Creating the child process and executing the command in the child
 - The separate child process is created using the fork() system call, and the user’s command is executed
using one of the system calls in the exec() family
2. Providing a history feature
- The next task is to modify the shell interface program so that it provides a history feature to allow a user
to execute the most recent command by entering !!. For example, if a user enters the command ls -l, she
can then execute that command again by entering !! at the prompt. 
3. Adding support of input and output redirection
 - Your shell should then be modified to support the ‘>’ and ‘<’ redirection operators, where ‘>’ redirects
the output of a command to a file and ‘<’ redirects the input to a command from a file.
4. Allowing the parent and child processes to communicate via a pipe
- The final modification to your shell is to allow the output of one command to serve as input to another
using a pipe