#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>

#define STRING_SIZE 256
#define CMD_FILE "monitor_cmd.txt"

volatile sig_atomic_t got_signal = 0;

void sigusr1_handler(int sig) {
    got_signal = 1;
}

void setup_sigusr1() {
    struct sigaction sa;
    sa.sa_handler = sigusr1_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(SIGUSR1, &sa, NULL) < 0) {
        fprintf(stderr, "ERROR: %s\n", strerror(errno));
        exit(1);
    }
}

void process_command() {
    char buf[STRING_SIZE];
    FILE *f = fopen(CMD_FILE, "r");
    if (!f) {
        fprintf(stderr, "ERROR: %s\n", strerror(errno));
        return;
    }

    if (fgets(buf, sizeof(buf), f) == NULL) {
        fclose(f);
        return;
    }
    fclose(f);

    buf[strcspn(buf, "\n")] = '\0';

    if (strcmp(buf, "list_hunts") == 0) {
        printf("[Monitor] Listing hunts.\n");
    } else if (strcmp(buf, "list_treasures") == 0) {
        printf("[Monitor] Listing treasures.\n");
    } else if (strcmp(buf, "view_treasure") == 0) {
        printf("[Monitor] Viewing treasure.\n");
    } else if (strcmp(buf, "stop") == 0) {
        printf("[Monitor] Stopping...\n");
        usleep(2000000);
        exit(0);
    } else {
        printf("[Monitor] Unknown command: %s\n", buf);
    }
}

int main() {
    setup_sigusr1();

    while (1) {
        pause();
        if (got_signal) {
            process_command();
            got_signal = 0;
        }
    }

    return 0;
}
