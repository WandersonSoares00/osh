#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

int cshell_cd (char **path) {
    if (!path[1]) {
        path[1] = getenv("HOME");
        path[2] = NULL;
    }

    chdir(path[1]);

    if (errno) {
        perror("shell: cd");
        errno = 0;
    }

    return 0;
}

int cshell_exit(char **argv) {
    if (!argv[1]) {
        exit(EXIT_SUCCESS);
    }
    exit(EXIT_FAILURE);
}

char *builtin_str[] = {
  "cd",
  "exit",
  NULL
};

int (*builtin_func[]) (char **) = {
  &cshell_cd,
  &cshell_exit,
  NULL
};


