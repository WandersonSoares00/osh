#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/wait.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "inc/sh.h"
#include "inc/parser.h"
#include "inc/exec.h"
#include "inc/reader.h"


int main () {
    
    cshell_init();

    char input[MAX_CHAR_INPUT];

    Parsed_input p_input;
    parser_init(&p_input);
    
    while (1) {
        fflush(stdout);

        get_input(getenv("PS1"), input);

        if (feof(stdin))    break;

        if (process_input(input, &p_input) > 0) {
            continue;
        }

        exec_commands(&p_input);
        
        if (errno) {
            perror("shell");
        }
    }

    parser_free(&p_input);

    return EXIT_SUCCESS;
}

