#ifndef EXEC_H
#define EXEC_H

#include "stack.h"
#include "parser.h"
#include "builtins.h"

struct Redirects {
    char *filename;
    int fd;
};

void exec(Stack *ast);

void exec_commands(Parsed_input *p_input);

#endif
