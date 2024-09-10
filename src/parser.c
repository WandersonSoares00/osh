#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
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
    p_input->ast    = stack_init(MAX_CHAR_INPUT/2, free_node);
    p_input->tokens = stack_init(MAX_CHAR_INPUT/2, free_node);
}

int parse_io_redirect(Stack *ast, Stack *tokens) {
    if (stack_is_empty(tokens))
        return 0;

    Token *token = stack_top(tokens);
    
    if (token->type == t_io_file) {
        stack_pop(tokens);

        Ast_node *ast_node = new_node(ast, sizeof(Ast_node));
        ast_node->content_type = token->content_type;
        ast_node->io_number    = token->io_number;
        ast_node->cmd          = token->file;

        stack_push(ast, ast_node);
        
        parse_io_redirect(ast, tokens);

        return 1;
    }

    return 0;
}

int parse_command_opts(Stack *tokens, Ast_node *node_opts) {
    Token *token = stack_top(tokens);

    if (token && token->type == t_file_cmd) {
        stack_pop(tokens);
        int arg = parse_command_opts(tokens, node_opts);
        if (arg == MAX_ARGS) {
            return arg;
        }
        node_opts->opts[arg] = token->file;
        return arg + 1;
    }
    
    return 0;
}


int parse_command(Stack *ast, Stack *tokens) {
    if (stack_is_empty(tokens))
        return 0;

    Token *token = stack_top(tokens);
    if (token->type == t_file_cmd) {
        
        Ast_node *ast_node = new_node(ast, sizeof(Ast_node));
        int i = parse_command_opts(tokens, ast_node);

        ast_node->content_type = t_file_cmd;
        ast_node->opts[i] = NULL;
        ast_node->cmd = ast_node->opts[0];

        stack_push(ast, ast_node);       
        return 1;
    }

    return 0;
}

int parse_command_io(Stack *ast, Stack *tokens) {
    int r = parse_io_redirect(ast, tokens);

    if(parse_command(ast, tokens)) {
        Token *token = stack_top(tokens);
        if (token && token->type == t_pipe) {
            stack_pop(tokens);
            Ast_node *ast_node = new_node(ast, sizeof(Ast_node));
            ast_node->content_type = t_pipe;
            stack_push(ast, ast_node);
            return parse_command_io(ast, tokens);
        }

        return 1;
    }

    return r;
}

int parse_command_list (Stack *ast, Stack *tokens) {
    if (parse_command_io(ast, tokens)) {
        Token *token = stack_top(tokens);
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
        stack_pop(tokens);
        Ast_node *ast_node = new_node(ast, sizeof(Ast_node));
        ast_node->content_type = t_ampersand;
        stack_push(ast, ast_node);
        return 1;
    }
    else {
        return 0;
    }
}

int parse (Parsed_input *p_input) {
    parse_ampersand(p_input->ast, p_input->tokens);
    int r = parse_command_list(p_input->ast, p_input->tokens);
    if (stack_is_empty(p_input->tokens)) {
        return r;
    }

    while(!stack_is_empty(p_input->tokens)) {
        stack_pop(p_input->tokens);
    }

    return 0; // syntax error
}

int extract_io_redirect(const char *str, Token *token) {
    int i = 0;
    char number[20] = "1";
    int pos = 0;
    int has_digit = 0;

    while (1) {
        if (isdigit(str[i])) {
            number[pos++] = str[i++];
            has_digit = 1;
        }
        else {
            number[pos+1] = '\0';
            break;
        }
    }
    
    token->io_number = atoi(number);

    while (str[i]) {
        if (!strcmp(str+i, ">>")) {
            if (!has_digit)  token->io_number = STDOUT_FILENO;
            token->content_type = t_io_file_dg;
            return str[i+2] == '\0';
        }
        else if (!strcmp(str+i, "<>")) {
            if (!has_digit)  token->io_number = STDIN_FILENO;
            token->content_type = t_io_file_lg;
            return str[i+2] == '\0';
        }
        else if (str[i] == '>') {
            if (!has_digit)  token->io_number = STDOUT_FILENO;
            token->content_type = t_io_file_g;
            return str[i+1] == '\0';
        }
        else if (str[i] == '<') {
            if (!has_digit)  token->io_number = STDIN_FILENO;
            token->content_type = t_io_file_l;
            return str[i+1] == '\0';
        }
        ++i;
    }

    return 0;
}

void set_token(Token *token, char *str) {
    if (!str) {
        token->content_type = -1;
        return;
    }

    if (*str == ';') {
        token->type         = t_cmd_sep;
        token->content_type = t_cmd_sep_semicolon;
    } else if (!strcmp(str, "&&")) {
        token->type         = t_cmd_sep;
        token->content_type = t_cmd_sep_doub_emp;
    } else if (!strcmp(str, "||")) {
        token->type         = t_cmd_sep;
        token->content_type = t_cmd_sep_doub_pipe;
    } else if (extract_io_redirect(str, token)) {
        token->type         = t_io_file;
    } else if (*str == '&') {
        token->type         = t_ampersand;
        token->content_type = t_ampersand;
    } else if (*str == '|') {
        token->type         = t_pipe;
        token->content_type = t_pipe;
    } else {
        token->type         = t_file_cmd;
        token->content_type = t_file_cmd;
        token->file         = str;
    }
}

int tokenize(char *raw_input, Stack *tokens) {
    
    char *delimiter = " ";
    char *str = strtok(raw_input, delimiter);
    Token *token;

    while (str) {
        token = new_node(tokens, sizeof(Token));
        set_token(token, str);
        if (token->type == t_io_file) {
            Token tmp;
            str = strtok(NULL, delimiter);
            set_token(&tmp, str);
            if (tmp.content_type == t_file_cmd)
                token->file = tmp.file;
            else {
                while(!stack_is_empty(tokens)) { // free stack
                    stack_pop(tokens);
                }
                return 1;
            }
        }
        stack_push(tokens, token);
        str = strtok(NULL, delimiter);
    }

    return 0;
}

int process_input(char *raw_input, Parsed_input *p_input) {
    raw_input[strcspn(raw_input, "\n")] = '\0'; // remove new line
    if (tokenize(raw_input, p_input->tokens) == 1) {
        fputs("osh: syntax error: unexpected token\n", stderr);
        return 1;
    }
    
    if (stack_is_empty(p_input->tokens))
        return 0;

    if (!parse(p_input)) {
        fputs("osh: syntax error\n", stderr);
        return 1;
    }
    
    return 0;
}

void parser_free(Parsed_input *p_input) {
    stack_free(p_input->ast);
    stack_free(p_input->tokens);
}

