#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "inc/stack.h"
#include "inc/parser.h"

void get_path(char *cmd, char *path) {
    char delimiter[] = ":";
    char *p = strtok(getenv("PATH"), delimiter);
    
    if (*cmd == '.' && *cmd+1 == '/'){
        strcat(path, getenv("PWD"));
        strcat(path, cmd+1);
        return;
    }

    while (p) {
        strcat(path, p);
        strcat(path, "/");
        strcat(path, cmd);

        if (access(path, F_OK) != -1)
            break;
        p = strtok(NULL, delimiter);
        *path = '\0';
    }
}

void exec(Parsed_input *p_input) {
    Stack *ast = p_input->ast;
    
    Ast_node *node = stack_pop(ast);

    if (node->content_type == t_file_cmd) {
        char path[50] = "";
        get_path(node->cmd, path);
        execve(path, node->opts, NULL);
    }

    while(!stack_is_empty(p_input->ast)) {
        stack_pop(p_input->ast);
    }
}


