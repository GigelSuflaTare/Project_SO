#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

pid_t monitor_pid = -1;
int monitor_exiting = 0;

int monitor_pipe[2] = {-1, -1};

void handle_monitor_exit(int sig);

void start_monitor() {
    if (monitor_pid != -1) {
        printf("Monitor already running (PID: %d).\n", monitor_pid);
        return;
    }
    if (pipe(monitor_pipe) == -1) {
        perror("pipe");
        return;
    }
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed");
        return;
    } else if (pid == 0) {
        close(monitor_pipe[0]); 
        char fd_arg[16];
        snprintf(fd_arg, sizeof(fd_arg), "%d", monitor_pipe[1]);
        execl("./monitor", "./monitor", fd_arg, NULL);
        perror("Failed to start monitor");
        exit(EXIT_FAILURE);
    } else {
        monitor_pid = pid;
        close(monitor_pipe[1]); 
        signal(SIGCHLD, handle_monitor_exit);
        printf("Monitor started (PID: %d).\n", monitor_pid);
    }
}
void calculate_scores() {
    DIR *d = opendir(".");
    if (!d) {
        perror("opendir");
        return;
    }
    struct dirent *entry;
    while ((entry = readdir(d))) {
        if (entry->d_name[0] != '.') {
            struct stat st;
            if (stat(entry->d_name, &st) == 0 && S_ISDIR(st.st_mode)) {
                char dat_path[256];
                snprintf(dat_path, sizeof(dat_path), "%s/treasures.dat", entry->d_name);
                if (access(dat_path, F_OK) == 0) {
                    int fd[2];
                    if (pipe(fd) == -1) continue;
                    fcntl(fd[0], F_SETFD, fcntl(fd[0], F_GETFD) | FD_CLOEXEC);
                    fcntl(fd[1], F_SETFD, fcntl(fd[1], F_GETFD) | FD_CLOEXEC);
                    pid_t pid = fork();
                    if (pid == 0) {
                        close(fd[0]); 
                        dup2(fd[1], 1); 
                        close(fd[1]);
                        execl("./calculate_score", "./calculate_score", dat_path, NULL);
                        perror("execl");
                        exit(1);
                    } else if (pid > 0) {
                        close(fd[1]);
                        char buf[1024];
                        ssize_t n;
                        printf("Scores for hunt %s:\n", entry->d_name);
                        while ((n = read(fd[0], buf, sizeof(buf)-1)) > 0) {
                            buf[n] = 0;
                            printf("%s", buf);
                        }
                        close(fd[0]);
                        waitpid(pid, NULL, 0);
                    }
                }
            }
        }
    }
    closedir(d);
}

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
    char buf[4096];
    ssize_t n = read(monitor_pipe[0], buf, sizeof(buf)-1);
    if (n > 0) {
        buf[n] = 0;
        printf("%s", buf);
    }
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
        
        if (strcmp(input, "calculate_score") == 0) {
            calculate_scores();
        } else if (strcmp(input, "start_monitor") == 0) {
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
