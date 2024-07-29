#ifndef SH_H
#define SH_H

#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 600
#endif

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <termios.h>
#include <sys/select.h>
#include <sys/stat.h>

void cshell_init();

int pty_master_open(char *slave_name);

int pty_fork(int master_fd, char *slave_name);

void forward_to_slave_pty(int master_fd);

#endif
