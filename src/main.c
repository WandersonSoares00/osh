#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/wait.h>
#include "inc/sh.h"
#include "inc/parser.h"
#include "inc/exec.h"


int main () {
    
    cshell_init();

    char input[MAX_CHAR_INPUT];
    char *prompt;
    int cpid;

    Parsed_input p_input;
    parser_init(&p_input);
    
    while (1) {
        prompt = getenv("PS1");
        fprintf(stdout, prompt);
        fgets(input, MAX_CHAR_INPUT, stdin);

        if (feof(stdin))    break;

        process_input(input, &p_input);

        pid_t pid = fork();
        
        if (pid < 0) {
            fprintf(stderr, "shell: internal error\n");
            exit(EXIT_FAILURE);
        }
        else if (pid != 0) {
            cpid = waitpid(pid, NULL, 0);
        }
        else {
            exec(&p_input);
            if (errno)  perror("shell");
       }
    }

    parser_free(&p_input);

    return EXIT_SUCCESS;
}

