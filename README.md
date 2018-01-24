# Linux-shell-implemented-in-C
A simple Linux shell implemented in C

## INSTRUCTIONS

Compiling in LINUX using command: gcc -o ve482sh ve482sh.c <br />
Running: ./ve482sh <br />
Exit: input "exit" to stop the program <br />

## FUNCTIONS

1. Single command: ls, echo, grep, cat, mkdir, vi, pwd, sudo, apt-get, etc.
2. Pipe: pipe between 2 processes
3. Single command with file I/O redirection: >, >>, <
4. Error detection when < as the end of command line
5. Handle multiple spaces at the begining, middle, or end of the command line
6. Wait for a complete input if enter is pressed
7. Change working directory (include error detection)
8. When Ctrl+c is detected, press enter to continue
