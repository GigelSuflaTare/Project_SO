#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

pid_t monitor_pid = -1;
int monitor_exiting = 0;

void send_command_to_monitor(const char *command) {
    if (monitor_pid == -1) {
        printf("Monitor is not running.\n");
        return;
    }

    FILE *fp = fopen("monitor_command.txt", "w");
    if (!fp) {
        perror("Failed to write to monitor_command.txt");
        return;
    }
    fprintf(fp, "%s\n", command);
    fclose(fp);

    kill(monitor_pid, SIGUSR1);
}

void handle_monitor_exit(int sig) {
    int status;
    pid_t pid = waitpid(monitor_pid, &status, 0);
    if (pid == monitor_pid) {
        printf("[treasure_hub] Monitor process exited.\n");
        if (WIFEXITED(status)) {
            printf("[treasure_hub] Exit code: %d\n", WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            printf("[treasure_hub] Killed by signal: %d\n", WTERMSIG(status));
        }
        monitor_pid = -1;
        monitor_exiting = 0;
    }
}

void start_monitor() {
    if (monitor_pid != -1) {
        printf("Monitor already running (PID: %d).\n", monitor_pid);
        return;
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed");
        return;
    } else if (pid == 0) {
        execl("./monitor", "./monitor", NULL);
        perror("Failed to start monitor");
        exit(EXIT_FAILURE);
    } else {
        monitor_pid = pid;
        signal(SIGCHLD, handle_monitor_exit);
        printf("Monitor started (PID: %d).\n", monitor_pid);
    }
}

void stop_monitor() {
    if (monitor_pid == -1) {
        printf("Monitor is not running.\n");
        return;
    }

    kill(monitor_pid, SIGTERM);
    monitor_exiting = 1;
    printf("Sent termination signal to monitor (PID: %d). Waiting for it to exit...\n", monitor_pid);
}

int main() {
    char input[256];

    printf("Welcome to Treasure Hub\n");

    while (1) {
        printf(">> ");
        fflush(stdout);

        if (!fgets(input, sizeof(input), stdin)) {
            printf("Input error or EOF.\n");
            break;
        }

        input[strcspn(input, "\n")] = '\0';

        if (monitor_exiting) {
            printf("Monitor is shutting down. Please wait...\n");
            continue;
        }

        if (strcmp(input, "start_monitor") == 0) {
            start_monitor();
        } else if (strcmp(input, "stop_monitor") == 0) {
            stop_monitor();
        } else if (strcmp(input, "list_hunts") == 0) {
            send_command_to_monitor("list_hunts");
        } else if (strncmp(input, "list_treasures ", 15) == 0) {
            send_command_to_monitor(input);
        } else if (strncmp(input, "view_treasure ", 14) == 0) {
            send_command_to_monitor(input);
        } else if (strcmp(input, "exit") == 0) {
            if (monitor_pid != -1) {
                printf("Cannot exit: monitor is still running.\n");
            } else {
                printf("Goodbye!\n");
                break;
            }
        } else {
            printf("Unknown command: %s\n", input);
        }
    }

    return 0;
}
