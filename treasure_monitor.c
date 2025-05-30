#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include "common.h"

int result_fd = 1;

void handle_command(int result_fd) {
    char buffer[MAX_COMMAND_LEN];
    FILE *fp = fopen(COMMAND_FILE, "r");
    if (!fp) return;

    if (fgets(buffer, sizeof(buffer), fp)) {
        buffer[strcspn(buffer, "\n")] = 0;

        if (strncmp(buffer, "list_hunts", 10) == 0) {
            FILE *cmd_fp = popen("ls -d */ | sed 's#/##'", "r");
            if (cmd_fp) {
                char line[256];
                while (fgets(line, sizeof(line), cmd_fp)) {
                    write(result_fd, line, strlen(line));
                }
                pclose(cmd_fp);
            }
        } else if (strncmp(buffer, "list_treasures", 14) == 0) {
            char hunt[64];
            sscanf(buffer, "list_treasures %s", hunt);
            char cmd[128];
            snprintf(cmd, sizeof(cmd), "./treasure_manager --list %s", hunt);
            FILE *cmd_fp = popen(cmd, "r");
            if (cmd_fp) {
                char line[256];
                while (fgets(line, sizeof(line), cmd_fp)) {
                    write(result_fd, line, strlen(line));
                }
                pclose(cmd_fp);
            }
        } else if (strncmp(buffer, "view_treasure", 13) == 0) {
            char hunt[64];
            int id;
            sscanf(buffer, "view_treasure %s %d", hunt, &id);
            char cmd[128];
            snprintf(cmd, sizeof(cmd), "./treasure_manager --view %s %d", hunt, id);
            FILE *cmd_fp = popen(cmd, "r");
            if (cmd_fp) {
                char line[256];
                while (fgets(line, sizeof(line), cmd_fp)) {
                    write(result_fd, line, strlen(line));
                }
                pclose(cmd_fp);
            }
        } else if (strcmp(buffer, "stop") == 0) {
            char msg[] = "Monitor: stopping...\n";
            write(result_fd, msg, strlen(msg));
            usleep(1000000);
            exit(0);
        } else if (strncmp(buffer, "calculate_score ", 16) == 0) {
            char hunt[64];
            sscanf(buffer, "calculate_score %s", hunt);
            char dat_path[128];
            snprintf(dat_path, sizeof(dat_path), "%s/treasures.dat", hunt);
            char cmd[256];
            snprintf(cmd, sizeof(cmd), "./calculate_score %s", dat_path);
            FILE *cmd_fp = popen(cmd, "r");
            if (cmd_fp) {
                char line[256];
                while (fgets(line, sizeof(line), cmd_fp)) {
                    write(result_fd, line, strlen(line));
                }
                pclose(cmd_fp);
            }
        } else {
            char msg[256];
            snprintf(msg, sizeof(msg), "Monitor: unknown command '%s'\n", buffer);
            write(result_fd, msg, strlen(msg));
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
    handle_command(result_fd);
}

int main(int argc, char *argv[]) {
    if (argc > 1) {
        result_fd = atoi(argv[1]);
    }

    setup_signal(SIG_LIST_HUNTS, signal_handler);
    setup_signal(SIG_LIST_TREASURES, signal_handler);
    setup_signal(SIG_VIEW_TREASURE, signal_handler);
    setup_signal(SIG_STOP_MONITOR, signal_handler);

    fprintf(stderr, "Monitor started with PID %d. Waiting for commands...\n", getpid());

    while (1) {
        pause();
    }

    return 0;
}
