#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>
#include "inc/stack.h"
#include "inc/parser.h"
#include "inc/builtins.h"
#include "inc/darray.h"

void get_path(char *cmd, char *path) {
    char delimiter[] = ":";
    char *p = strtok(getenv("PATH"), delimiter);

    if (*cmd == '.' && *cmd+1 == '/'){
        strcat(path, cmd+2);
        return;
    }

    if (access(cmd, F_OK) != -1) // complete path
        return;

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

int exec_builtin (Ast_node *node, Darray **redirects) {
    for (int i = 0; builtin_str[i]; ++i) {
        if (!strcmp(node->cmd, builtin_str[i])) {
            (*builtin_func[i])(node->opts);
            return 1;
        }
    }
    return 0;
}

int is_io_redirect (Ast_node *node) {
    if (!node)  return 0;
    if (node->content_type == t_io_file_l)
        return 1;
    if (node->content_type == t_io_file_g)
        return 2;
    if (node->content_type == t_io_file_dg)
        return 3;
    if (node->content_type == t_io_file_lg)
        return 4;
    return 0;
}

int open_file (Ast_node *node, int *fd) {
    int flags;
    switch(is_io_redirect(node)) {
        case 1: flags = O_RDONLY; break;
        case 2: flags = O_CREAT | O_TRUNC  | O_WRONLY; break;
        case 3: flags = O_CREAT | O_APPEND | O_WRONLY; break;
        case 4: flags = O_CREAT | O_RDWR; break;
        case 0: return 1;
    }

    if((*fd = open(node->cmd, flags, S_IRUSR+S_IWUSR+S_IRGRP+S_IWGRP+S_IROTH)) < 0)
        return 1;

    return 0;
}

void exec_cmd(Ast_node *node, Darray **redirects) {
    if (redirects && *redirects) {
        int fd;
        Ast_node *file = darray_get_front(*redirects);
        while (file) {
            if (open_file(file, &fd) == 0) {
                if(dup2(fd, file->io_number) == -1)
                    perror("shell");
                close(fd);
            }
            darray_pop_front(*redirects);
            file = darray_get_front(*redirects);
        }
    }
    char path[50] = "";
    get_path(node->cmd, path);
    execv(path, node->opts);
}

int exec(Ast_node *node, Darray **redirects) {
    if (exec_builtin(node, redirects)) {
        return 0;
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return 1;
    }
    
    if (pid > 0) {
        return waitpid(pid, NULL, 0) == -1;
    }
    else {
        exec_cmd(node, redirects);
        perror("shell");
        exit(1);
    }

    return 0;
}


void dealloc_arr(void *i) {}

void get_redirects (Stack *ast, Darray **redirects) {
    if (!redirects)
        return;

    if (*redirects == NULL && is_io_redirect(stack_top(ast)))
        *redirects = darray_init(1, dealloc_arr);

    for(;!stack_is_empty(ast);) {
        if (is_io_redirect(stack_top(ast))) {
            darray_push_back(*redirects, stack_pop(ast));
        }
        else    break;
    }
}

int exec_pipe (Stack *ast, Darray **redirects, Ast_node *cmd1, Ast_node *cmd2) {
    int p[2];
    pid_t pid;

    if (pipe(p) < 0) {
        perror("pipe");
        return 1;
    }
    
    if ((pid = fork()) > 0) { // writing process
        close(p[0]);         // close read end
        dup2(p[1], STDOUT_FILENO); // stdout for write end of the pipe
        close(p[1]);
        int ret = exec(cmd1, redirects);
        close(STDOUT_FILENO);
        wait(NULL);
        return ret;
    }
    else {           // reading process
        close(p[1]); // close write end
        dup2(p[0], STDIN_FILENO); // stdin for read end of the pipe
        close(p[0]);
        Ast_node *node = stack_top(ast);
        if (node && node->content_type == t_pipe) { // pipe list
            stack_pop(ast);
            node = stack_pop(ast);
            get_redirects(ast, redirects);
            exec_pipe(ast, redirects, cmd2, node);
            exit(0);
        }
        
        get_redirects(ast, redirects);
        exec_cmd(cmd2, redirects);
        perror("shell");
        exit(1);
    }
}


void exec_commands(Parsed_input *p_input) {
    Stack *ast = p_input->ast;
    Ast_node *node;
    Ast_node *cmd_left;
    Ast_node *cmd_right;
    int ret = 2;
    Darray *redirects = NULL;

    while (!stack_is_empty(ast)) {
        node = stack_pop(ast);

        if (is_io_redirect(node)) { // io-redirect list
            int fd;
            ret = open_file(node, &fd);
            if (ret==0)     close (fd);
            ret = 2;
            continue;
        }

        cmd_left = node;
        node = stack_top(ast);
        if (!node) {
            exec(cmd_left, NULL);
            break;
        }
        
        get_redirects(ast, &redirects);

        node = stack_top(ast);

        if (!node) {
            exec(cmd_left, &redirects);
            break;
        }
        
        if (node->content_type == t_pipe) {
            stack_pop(ast);
            cmd_right = stack_pop(ast);
            int tmpin  = dup(STDIN_FILENO);
            int tmpout = dup(STDOUT_FILENO);
            ret = exec_pipe(ast, &redirects, cmd_left, cmd_right);
            dup2(tmpin, STDIN_FILENO);
            dup2(tmpout, STDOUT_FILENO);
            close(tmpin);
            close(tmpout);
            
            //int pid;
            //while((pid = wait(NULL)) > 0)   printf("%d -- ", pid);
            //printf("\n");

            while(!stack_is_empty(ast)) {
                Ast_node *tmp = stack_top(ast);
                if (tmp->content_type == t_pipe) {
                    stack_pop(ast); // pipe
                    stack_pop(ast); // command
                    if (is_io_redirect(stack_top(ast))) stack_pop(ast);
                } else  break;
            }
        }
        
        node = stack_pop(ast);

        if (ret == 2)
            ret = exec(cmd_left, &redirects);

        if (!node)  break;

        if (node->content_type == t_cmd_sep_doub_emp)
            if (ret)    break; // exit status failed

        if (node->content_type == t_cmd_sep_doub_pipe)
            if (!ret)    break; // exit status is success

        ret = 2;
    }

    darray_free(redirects);

    while(!stack_is_empty(ast)) {
        stack_pop(ast);
    }
}


