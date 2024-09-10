#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

int osh_cd (char **path) {
    if (!path[1]) {
        path[1] = getenv("HOME");
        path[2] = NULL;
    }

    chdir(path[1]);

    if (errno) {
        perror("osh: cd");
        errno = 0;
    }

    return 0;
}

int osh_exit(char **argv) {
    if (!argv[1]) {
        exit(EXIT_SUCCESS);
    }
    exit(EXIT_FAILURE);
}

int osh_set_env(char **argv) {
    if (!argv[1] || !argv[2]) {
        fprintf(stderr, "setenv: usage: setenv [name] [value]\n");
        return 1;
    }

    setenv(argv[1], argv[2], 1);
    return 0;
}

char *builtin_str[] = {
  "cd",
  "exit",
  "setenv",
  NULL
};

int (*builtin_func[]) (char **) = {
  &osh_cd,
  &osh_exit,
  &osh_set_env,
  NULL
};


