#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/wait.h>

#define MAX_CHAR_INPUT 500

int main () {
    
    char input[MAX_CHAR_INPUT];
    char **params = NULL;
    int cpid;
    
    while (1) {
        fprintf(stdout, "prompt - ");
        fscanf(stdin, "%s", input);
        pid_t pid = fork();
        
        if (pid < 0) {
            fprintf(stderr, "shell: internal error\n");
            exit(EXIT_FAILURE);
        }
        else if (pid != 0) {
            cpid = waitpid(pid, NULL, 0);
        }
        else {
            execve(input, params, NULL);
            if (errno)  perror("shell");
       }
    }

    return EXIT_SUCCESS;
}
