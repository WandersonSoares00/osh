#include "inc/reader.h"
#ifdef HAVE_READLINE
#include <readline/readline.h>
#include <readline/history.h>

void get_input(char *prompt, char *str) {
    char *buff = readline(prompt);
    if (buff && *buff) {
        add_history(buff);
    }
    strcpy(str, buff);
}

#else
#warning "Readline library not found. Some features will be disabled."

void get_input(char *prompt, char *str) {
    fputs(prompt, stdout);
    fgets(str, MAX_CHAR_INPUT, stdin);
}


#endif

