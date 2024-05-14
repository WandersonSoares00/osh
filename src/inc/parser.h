#ifndef PARSER_H
#define PARSER_H

#include "stack.h"
#define MAX_CHAR_INPUT 500
#define MAX_ARGS       10

typedef struct {
    char *cmd;              // COMMAND or FILE_NAME
    char *opts[MAX_ARGS+1]; // command-opts, NULL in the end
    char content_type;      // token content_type
    int io_number;          // IO_NUMBER
} Ast_node;

// Token list
//
#define t_ampersand           0
#define t_pipe                1
#define t_cmd_sep             2
#define t_cmd_sep_semicolon   2
#define t_cmd_sep_doub_emp    3
#define t_cmd_sep_doub_pipe   4
#define t_io_file             5 //
#define t_io_file_l           5 // less             <
#define t_io_file_g           6 // greater          >
#define t_io_file_dg          7 // double greater   >>
#define t_io_file_lg          8 // less greater     <>
#define t_file_cmd            9 // FILE_NAME, COMMAND or IO_NUMBER

typedef struct {
    char type;
    char content_type;
    char *file;
    int io_number;
} Token;

typedef struct {
    Stack *ast;
    Stack *tokens;
} Parsed_input;


void *new_node (Stack *s, size_t element_size);

void parser_init(Parsed_input *p_input);

int parse_io_redirect(Stack *ast, Stack *tokens);

void parse_command_opts(Stack *tokens, Ast_node *node_opts);

int parse_command(Stack *ast, Stack *tokens);

int parse_command_list (Stack *ast, Stack *tokens);

int process_input(char *raw_input, Parsed_input *p_input);

void parser_free(Parsed_input *p_input);

#endif
