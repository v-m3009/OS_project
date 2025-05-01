#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <errno.h>

#define STRING_SIZE 256
#define CMD_FILE "monitor_cmd.txt"

pid_t monitor_pid = -1;
int monitor_running = 0;
int waiting_stop = 0;

void sigchld_handler(int sig) {
    int status;
    pid_t pid = wait(&status);
    if (pid == monitor_pid) {
        printf("Monitor process %d terminated with status %d.\n", pid, WEXITSTATUS(status));
        monitor_running = 0;
        waiting_stop = 0;
    }
}

void setup_sigchld() {
    struct sigaction sa;
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(SIGCHLD, &sa, NULL) < 0) {
        fprintf(stderr, "ERROR: %s\n", strerror(errno));
        exit(1);
    }
}

void write_command(char *cmd) {
    int fd = open(CMD_FILE, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        fprintf(stderr, "ERROR: %s\n", strerror(errno));
        return;
    }
    dprintf(fd, "%s\n", cmd);
    close(fd);
}

void send_signal() {
    if (kill(monitor_pid, SIGUSR1) < 0) {
        fprintf(stderr, "ERROR: %s\n", strerror(errno));
    }
}

int main() {
    setup_sigchld();
    char input[STRING_SIZE];

    while (1) {
        printf(">> ");
        fflush(stdout);

        if (fgets(input, STRING_SIZE, stdin) == NULL) break;
        input[strcspn(input, "\n")] = '\0';

        if (waiting_stop) {
            printf("Monitor is stopping. Please wait...\n");
            continue;
        }

        if (strcmp(input, "start_monitor") == 0) {
            if (monitor_running) {
                printf("Monitor already running.\n");
                continue;
            }

            pid_t pid = fork();
            if (pid < 0) {
                fprintf(stderr, "ERROR: %s\n", strerror(errno));
                continue;
            }

            if (pid == 0) {
                execl("./monitor", "monitor", NULL);
                fprintf(stderr, "ERROR: %s\n", strerror(errno));
                exit(1);
            }

            monitor_pid = pid;
            monitor_running = 1;
            printf("Monitor started with PID %d\n", monitor_pid);

        } else if (strcmp(input, "list_hunts") == 0) {
            if (!monitor_running) {
                printf("Monitor not running.\n");
                continue;
            }
            write_command("list_hunts");
            send_signal();
            usleep(100000);

        } else if (strcmp(input, "list_treasures") == 0) {
            if (!monitor_running) {
                printf("Monitor not running.\n");
                continue;
            }
            write_command("list_treasures");
            send_signal();
            usleep(100000);

        } else if (strcmp(input, "view_treasure") == 0) {
            if (!monitor_running) {
                printf("Monitor not running.\n");
                continue;
            }
            write_command("view_treasure");
            send_signal();
            usleep(100000);

        } else if (strcmp(input, "stop_monitor") == 0) {
            if (!monitor_running) {
                printf("Monitor not running.\n");
                continue;
            }
            write_command("stop");
            send_signal();
            usleep(100000);
            waiting_stop = 1;

        } else if (strcmp(input, "exit") == 0) {
            if (monitor_running) {
                printf("ERROR: Monitor still running.\n");
            } else {
                break;
            }

        } else {
            printf("Unknown command.\n");
        }
    }

    return 0;
}
