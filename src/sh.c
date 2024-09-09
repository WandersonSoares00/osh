#define _XOPEN_SOURCE 600
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <termios.h>
#include <sys/stat.h>
#include <sys/select.h>

#define BUF_PTY_SIZE_MASTER 256

int pty_master_open(char *slave_name) {
    int master_fd;
    char *tmp_name;

    master_fd = posix_openpt(O_RDWR | O_NOCTTY);
    if (master_fd == -1) return -1;
    if (grantpt(master_fd) == -1) {
        close(master_fd);
        return -1;
    }
    if (unlockpt(master_fd) == -1){
        close(master_fd);
        return -1;
    }

    tmp_name = ptsname(master_fd);
    if (tmp_name == NULL) {
        close(master_fd);
        return -1;
    }
    strncpy(slave_name, tmp_name, 2*L_ctermid);

    return master_fd;
}

int pty_fork(int master_fd, char *slave_name) {
    int slave_fd;
    pid_t pid;
    
    pid = fork();
    if (pid == -1) {
        close(master_fd);
        return -1;
    }
    if (pid != 0){
        return pid;
    }
    
    if (setsid() == -1){
        close(master_fd);
        return -1;
    }
    close(master_fd);
    slave_fd = open(slave_name, O_RDWR);
    if (slave_fd == -1){
        return -1;
    }
    dup2(slave_fd, STDIN_FILENO);
    dup2(slave_fd, STDOUT_FILENO);
    dup2(slave_fd, STDERR_FILENO);
    close(slave_fd);
    return 0;
}

// Aqui deve ficar a logica de controle 
// dos estados do shell como o uso das setas
// e historico de comados
void forward_to_slave_pty(int master_fd) {
    fd_set in_fd;
    char buf[BUF_PTY_SIZE_MASTER];
    ssize_t num_read;
    struct termios tp;

    tcgetattr(master_fd, &tp);
    tp.c_lflag &= ~ECHO; 
    tcsetattr(master_fd, 0, &tp);

    while(1){
        FD_ZERO(&in_fd);
        FD_SET(STDIN_FILENO, &in_fd);
        FD_SET(master_fd, &in_fd);
        if (select(master_fd+1, &in_fd, NULL, NULL, NULL) == -1)
            exit(1);

        if (FD_ISSET(STDIN_FILENO, &in_fd)) {
            num_read = read(STDIN_FILENO, buf, BUF_PTY_SIZE_MASTER);
            if (num_read <= 0)
                exit(0);
            write(master_fd, buf, num_read);
        }
        if (FD_ISSET(master_fd, &in_fd)) {
            num_read = read(master_fd, buf, BUF_PTY_SIZE_MASTER);
            if (num_read <= 0)
                exit(0);
            write(STDOUT_FILENO, buf, num_read);
        }

    }
}

void cshell_init() {
    int master_fd, slave_fd;
    char slave_name[2*L_ctermid];
    setenv("PS1", "$ ", 1);
    
    master_fd = pty_master_open(slave_name);
    if (master_fd == -1){
        perror("init shell");
        exit(1);
    } 

    slave_fd = pty_fork(master_fd, slave_name);
    if (slave_fd == -1) {
        perror("pty fork");
        exit(1);
    }
    
    // processo pai fica cuidando do master pty
    if (slave_fd != 0) {
        forward_to_slave_pty(master_fd);
        close(master_fd);
        exit(0);
    }

}


