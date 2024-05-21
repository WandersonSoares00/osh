#ifndef BUILTINS_H
#define BUILTINS_H

extern char *builtin_str[];

extern int (*builtin_func[]) (char **);

int cshell_cd (char **path);

int cshell_exit(char **argv);

#endif
