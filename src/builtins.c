#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

void cshell_cd (char *path) {
    if (!path) {
        path = getenv("HOME");
    }
    
    chdir(path);

    if (errno)
        perror("shell: cd");
}

void cshell_exit() {
    exit(EXIT_SUCCESS);
}

void cshell_exec(char *path, char *const *argv) {
    execve(path, argv, NULL);
    if (errno)
        perror("shell: exec");
}

