#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include "inc/parser.h"
#include "inc/stack.h"

void free_node(void *node) {
    free(node);
}

void *new_node (Stack *s, size_t element_size) {
    void *new_node = stack_get_cached_element(s);
    if (!new_node) {
        new_node = malloc(element_size);
    }
    return new_node;
}

void parser_init(Parsed_input *p_input) {
    p_input->ast    = stack_init(MAX_ARGS/2, free_node);
    p_input->tokens = stack_init(MAX_ARGS/2, free_node);
}

int parse_io_redirect(Stack *ast, Stack *tokens) {
    if (stack_is_empty(tokens))
        return 0;

    Token *token = stack_top(tokens);
    
    if (token->type == t_io_file) {
        stack_pop(tokens);
        Ast_node *ast_node = new_node(ast, sizeof(Ast_node));
        ast_node->content_type = token->content_type;
        ast_node->io_number = token->io_number;

        token = stack_top(tokens);
        if (token->type == t_file_cmd) {
            stack_pop(tokens);
            ast_node->cmd = token->file;
            stack_push(ast, ast_node);
            return 1;
        }
        return 2; //perror();
    }

    return 0;
}

void parse_command_opts(Stack *tokens, Ast_node *node_opts) {
    Token *token = stack_top(tokens);
    int arg = 1;

    while (arg < MAX_ARGS && token && token->type == t_file_cmd) {
        stack_pop(tokens);
        node_opts->opts[arg] = token->file;
        token = stack_top(tokens);
        if (!token || token->type != t_file_cmd) {
            node_opts->cmd = node_opts->opts[arg];
            node_opts->opts[0] = node_opts->cmd; // argv[0]
            break;
        }
        ++arg;
    }

    node_opts->opts[arg] = NULL;
}

int parse_command(Stack *ast, Stack *tokens) {
    if (stack_is_empty(tokens))
        return 0;

    Token *token = stack_top(tokens);

    if (token->type == t_file_cmd) {
        
        Ast_node *ast_node = new_node(ast, sizeof(Ast_node));
        parse_command_opts(tokens, ast_node);

        ast_node->content_type = t_file_cmd;

        stack_push(ast, ast_node);
        
        token = stack_top(tokens);
        if (token && token->type == t_pipe) {
            stack_pop(tokens);
            ast_node = new_node(ast, sizeof(Ast_node));
            ast_node->content_type = t_pipe;
            stack_push(ast, ast_node);
            return parse_command(ast, tokens);
        }

        return 1;
    }

    return 0;
}

int parse_command_list (Stack *ast, Stack *tokens) {
    if (parse_command(ast, tokens)) {
        if(parse_io_redirect(ast, tokens) == 2) {
            return 2;
        }

        Token *token = stack_top(tokens);
        //printf("peg %s\n", token->file);
        if (token && token->type == t_cmd_sep) {
            stack_pop(tokens);
            Ast_node *ast_node = new_node(ast, sizeof(Ast_node));
            ast_node->content_type = token->content_type;
            stack_push(ast, ast_node);
            return parse_command_list(ast, tokens);
        }

        return 1;
    }

    return 0;
}

int parse_ampersand(Stack *ast, Stack *tokens) {
    if (stack_is_empty(tokens))
        return 0;

    Token *token = stack_top(tokens);
    if (token->content_type == t_ampersand) {
        Ast_node *ast_node = new_node(ast, sizeof(Ast_node));
        ast_node->content_type = token->content_type;
        stack_push(ast, ast_node);
        return 1;
    }
    else {
        return 0;
    }
}

int parse (Parsed_input *p_input) {
    
    switch(parse_command_list(p_input->ast, p_input->tokens)) {
        case 0:     parse_io_redirect(p_input->ast, p_input->tokens);
                    break;
        case 1:     parse_ampersand(p_input->ast, p_input->tokens);
                    break;
        case 2:     return 2;
    }

    if (stack_is_empty(p_input->tokens)) {
        return 0;
    }

    while(!stack_is_empty(p_input->tokens)) {
        stack_pop(p_input->tokens);
    }

    return 1; // syntax error
}

int extract_io_redirect(const char *str, Token *token) {
    int i = 0;
    char number[20] = "1";  // default stdin
    int pos = 0;

    while (str[i]) {
        if (isdigit(str[i])) {
            number[pos++] = str[i];            
        }
        else if (!strcmp(str+i, ">>")) {
            number[pos] = '\0';
            token->io_number = atoi(number);
            token->content_type = t_io_file_dg;
            return str[i+2] == '\0';
        }
        else if (!strcmp(str+i, "<>")) {
            number[pos] = '\0';
            token->io_number = atoi(number);
            token->content_type = t_io_file_lg;
            return str[i+2] == '\0';
        }
        else if (str[i] == '>') {
            number[pos] = '\0';
            token->io_number = atoi(number);
            token->content_type = t_io_file_g;
            return str[i+1] == '\0';
        }
        else if (str[i] == '<') {
            number[pos] = '\0';
            token->io_number = atoi(number);
            token->content_type = t_io_file_l;
            return str[i+1] == '\0';
        }
        ++i;
    }

    return 0;
}


void tokenize(char *raw_input, Stack *tokens) {
    
    char *delimiter = " ";
    char *token = strtok(raw_input, delimiter);
    Token *tmp;

    while (token) {
        tmp = new_node(tokens, sizeof(Token));
        if (*token == ';') {
            tmp->type         = t_cmd_sep;
            tmp->content_type = t_cmd_sep_semicolon;
        } else if (!strcmp(token, "&&")) {
            tmp->type         = t_cmd_sep;
            tmp->content_type = t_cmd_sep_doub_emp;
        } else if (!strcmp(token, "||")) {
            tmp->type         = t_cmd_sep;
            tmp->content_type = t_cmd_sep_doub_pipe;
        } else if (extract_io_redirect(token, tmp)) {
            tmp->type         = t_io_file;
        } else if (*token == '&') {
            tmp->type         = t_ampersand;
            tmp->content_type = t_ampersand;
        } else if (*token == '|') {
            tmp->type         = t_pipe;
            tmp->content_type = t_pipe;
        } else {
            tmp->type         = t_file_cmd;
            tmp->content_type = t_file_cmd;
            tmp->file         = token;
        }
        
        stack_push(tokens, tmp);
        token = strtok(NULL, delimiter);
    }
}

int process_input(char *raw_input, Parsed_input *p_input) {
    raw_input[strcspn(raw_input, "\n")] = '\0'; // remove new line
    tokenize(raw_input, p_input->tokens);
    int r = parse(p_input);
    if (r == 2) {
        fprintf(stderr, "shell: syntax error near unexpected token `newline'\n");
    }
    else if (r == 1) {
        fprintf(stderr, "shell: syntax error\n");
    }
    else
        return 0;

    return 1;        
}

void parser_free(Parsed_input *p_input) {
    stack_free(p_input->ast);
    stack_free(p_input->tokens);
}


