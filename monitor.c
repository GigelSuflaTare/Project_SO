#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

int running = 1;

void handle_sigusr1(int sig) {
    FILE *fp = fopen("monitor_command.txt", "r");
    if (!fp) {
        perror("Could not open monitor_command.txt");
        return;
    }

    char cmd[128];
    fgets(cmd, sizeof(cmd), fp);
    fclose(fp);

    cmd[strcspn(cmd, "\n")] = 0;

    printf("[MONITOR] Received command: %s\n", cmd);

    if (strcmp(cmd, "list_hunts") == 0) {
        system("ls -d */ | sed 's#/##'");
    } else if (strncmp(cmd, "list_treasures ", 15) == 0) {
        char hunt[128];
        sscanf(cmd + 15, "%s", hunt);
        char exec_cmd[256];
        snprintf(exec_cmd, sizeof(exec_cmd), "./treasure_manager --list %s", hunt);
        system(exec_cmd);
    } else if (strncmp(cmd, "view_treasure ", 14) == 0) {
        char hunt[128];
        int id;
        sscanf(cmd + 14, "%s %d", hunt, &id);
        char exec_cmd[256];
        snprintf(exec_cmd, sizeof(exec_cmd), "./treasure_manager --view %s %d", hunt, id);
        system(exec_cmd);
    } else {
        printf("Unknown command in monitor: %s\n", cmd);
    }
}

void handle_sigterm(int sig) {
    printf("[MONITOR] Received termination signal. Exiting soon...\n");
    usleep(2000000); 
    running = 0;
}

void setup_signal(int sig, void (*handler)(int)) {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(sig, &sa, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
}

int main() {
    printf("[MONITOR] Monitor started with PID %d\n", getpid());

    setup_signal(SIGUSR1, handle_sigusr1);
    setup_signal(SIGTERM, handle_sigterm);

    while (running) {
        pause();
    }

    printf("[MONITOR] Monitor exiting gracefully.\n");
    return 0;
}
