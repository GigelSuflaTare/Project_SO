#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include "common.h"

void handle_command() {
    char buffer[MAX_COMMAND_LEN];
    FILE *fp = fopen(COMMAND_FILE, "r");
    if (!fp) return;

    if (fgets(buffer, sizeof(buffer), fp)) {
        buffer[strcspn(buffer, "\n")] = 0;

        if (strncmp(buffer, "list_hunts", 10) == 0) {
            printf("Monitor: listing hunts...\n");
        } else if (strncmp(buffer, "list_treasures", 14) == 0) {
            char hunt[64];
            sscanf(buffer, "list_treasures %s", hunt);
            char cmd[128];
            snprintf(cmd, sizeof(cmd), "./treasure_manager --list %s", hunt);
            system(cmd);
        } else if (strncmp(buffer, "view_treasure", 13) == 0) {
            char hunt[64];
            int id;
            sscanf(buffer, "view_treasure %s %d", hunt, &id);
            char cmd[128];
            snprintf(cmd, sizeof(cmd), "./treasure_manager --view %s %d", hunt, id);
            system(cmd);
        } else if (strcmp(buffer, "stop") == 0) {
            printf("Monitor: stopping...\n");
            usleep(1000000);
            exit(0);
        } else {
            printf("Monitor: unknown command '%s'\n", buffer);
        }
    }

    fclose(fp);
}

void setup_signal(int sig, void (*handler)(int)) {
    struct sigaction sa;
    sa.sa_handler = handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(sig, &sa, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
}

void signal_handler(int signo) {
    handle_command();
}

int main() {
    setup_signal(SIG_LIST_HUNTS, signal_handler);
    setup_signal(SIG_LIST_TREASURES, signal_handler);
    setup_signal(SIG_VIEW_TREASURE, signal_handler);
    setup_signal(SIG_STOP_MONITOR, signal_handler);

    printf("Monitor started with PID %d. Waiting for commands...\n", getpid());

    while (1) {
        pause();
    }

    return 0;
}
