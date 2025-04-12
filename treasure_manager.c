#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "treasure.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>
#include <limits.h>

#define DATAFILE "treasures.dat"
#define LOGFILE "logged_hunt"

void add_treasure(const char *hunt_id){
    char path[256];
    snprintf(path, sizeof(path), "%s", hunt_id);
    mkdir(path, 0777);

    char filepath[256];
    snprintf(filepath, sizeof(filepath), "%s/%s", path, DATAFILE);
    int fd = open(filepath, O_CREAT | O_WRONLY | O_APPEND, 0644);
    if(fd < 0){
        perror("Error opening file");
        return;
    }

    Treasure treasure;
    printf("Enter treasure ID: ");
    scanf("%d", &treasure.treasure_id);
    printf("Enter username: "); 
    scanf("%s", treasure.username);
    printf("Enter latitude: ");
    scanf("%f", &treasure.latitude);
    printf("Enter longitude: ");
    scanf("%f", &treasure.longitude);
    printf("Enter clue: ");
    getchar();
    fgets(treasure.clue, sizeof(treasure.clue), stdin);
    printf("Enter value: ");
    scanf("%d", &treasure.value);

    write(fd, &treasure, sizeof(Treasure));
    close(fd);

    char logpath[256];
    snprintf(logpath, sizeof(logpath), "%s/%s", path, LOGFILE);
    FILE *logfile = fopen(logpath, "a");
    if(logfile){
        time_t now = time(NULL);
        fprintf(logfile, "[%s] ADD: treasure %d by %s\n", ctime(&now), treasure.treasure_id, treasure.username);
        fclose(logfile);
    }

    char symlink_path[256];
    snprintf(symlink_path, sizeof(symlink_path), "logged_hunt-%s", hunt_id);
    symlink(logpath, symlink_path);
}

void list_treasures(const char* hunt_id){
    char filepath[256];
    snprintf(filepath, sizeof(filepath), "%s/%s", hunt_id, DATAFILE);
    
    struct stat st;
    if(stat(filepath, &st) == -1){
        perror("stat");
        return;
    }

    printf("Hunt name: %s\n", hunt_id);
    printf("FIle Size: %ld bytes\n", st.st_size);
    printf("Last modified: %s", ctime(&st.st_mtime));

    int fd = open(filepath, O_RDONLY);
    if(fd < 0){
        perror("open");
        return;
    }

    Treasure treasure;
    while(read(fd, &treasure, sizeof(Treasure)) > 0){
        printf("Treasure ID: %d\n", treasure.treasure_id);
        printf("Username: %s\n", treasure.username);
        printf("Latitude: %.2f\n", treasure.latitude);
        printf("Longitude: %.2f\n", treasure.longitude);
        printf("Clue: %s\n", treasure.clue);
        printf("Value: %d\n", treasure.value);
    }

    close(fd);
}

void view_treasure(const char* hunt_id, int treasure_id){
    char filepath[256];
    snprintf(filepath, sizeof(filepath), "%s/%s", hunt_id, DATAFILE);
    
    int fd = open(filepath, O_RDONLY);
    if(fd < 0){
        perror("open");
        return;
    }

    Treasure treasure;
    while(read(fd, &treasure, sizeof(Treasure)) > 0){
        if(treasure.treasure_id == treasure_id){
            printf("Treasure ID: %d\n", treasure.treasure_id);
            printf("Username: %s\n", treasure.username);
            printf("Latitude: %.2f\n", treasure.latitude);
            printf("Longitude: %.2f\n", treasure.longitude);
            printf("Clue: %s\n", treasure.clue);
            printf("Value: %d\n", treasure.value);
            close(fd);
            return;
        }
    }

    printf("Treasure with ID %d not found.\n", treasure_id);
    close(fd);
}

void remove_treasure(const char* hunt_id, int treasure_id){
    char filepath[256];
    snprintf(filepath, sizeof(filepath), "%s/%s", hunt_id, DATAFILE);
    
    int fd = open(filepath, O_RDWR);
    if(fd < 0){
        perror("open");
        return;
    }

    char tempfile[256];
    snprintf(tempfile, sizeof(tempfile), "%s/tempfile", hunt_id);
    int temp_fd = open(tempfile, O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if(temp_fd < 0){
        perror("open");
        close(fd);
        return;
    }

    Treasure treasure;
    int found = 0;
    while(read(fd, &treasure, sizeof(Treasure)) == sizeof(Treasure)){
        if(treasure.treasure_id == treasure_id){
            found = 1;
            continue;
        }
        write(temp_fd, &treasure, sizeof(Treasure));
    }

    close(fd);
    close(temp_fd);

    if(found){
        rename(tempfile, filepath);

        char logpath[256];
        snprintf(logpath, sizeof(logpath), "%s/%s", hunt_id, LOGFILE);
        FILE *logfile = fopen(logpath, "a");
        if(logfile){
            time_t now = time(NULL);
            fprintf(logfile, "[%s] REMOVE: treasure %d\n", ctime(&now), treasure_id);
            fclose(logfile);
        }
        printf("Treasure with ID %d removed.\n", treasure_id);
    }else{
        printf("Treasure with ID %d not found.\n", treasure_id);
        remove(tempfile);
    }
}

void remove_hunt(const char *hunt_id){
    char filepath[256];
    snprintf(filepath, sizeof(filepath), "%s/%s", hunt_id, DATAFILE);
    unlink(filepath);
    snprintf(filepath, sizeof(filepath), "%s/%s", hunt_id, LOGFILE);
    unlink(filepath);
    snprintf(filepath, sizeof(filepath), "logged_hunt-%s", hunt_id);
    unlink(filepath);
    rmdir(hunt_id);
    printf("Hunt %s removed.\n", hunt_id);
}

int main(int argc, char *argv[]){
    if (argc < 3) {
        fprintf(stderr, "Usage:\n");
        fprintf(stderr, "  %s --add <hunt_id>\n", argv[0]);
        fprintf(stderr, "  %s --list <hunt_id>\n", argv[0]);
        fprintf(stderr, "  %s --view <hunt_id> <treasure_id>\n", argv[0]);
        fprintf(stderr, "  %s --remove <hunt_id> <treasure_id>\n", argv[0]);
        fprintf(stderr, "  %s --remove-hunt <hunt_id>\n", argv[0]);
        return 1;
    }

    if(strcmp(argv[1], "--add") == 0 && argc == 3){
        add_treasure(argv[2]);
    }else if(strcmp(argv[1], "--list") == 0 && argc == 3){
        list_treasures(argv[2]);
    }else if(strcmp(argv[1], "--view") == 0 && argc == 4){
        int treasure_id = atoi(argv[3]);
        view_treasure(argv[2], treasure_id);
    } else if(strcmp(argv[1], "--remove") == 0 && argc == 4){
        int treasure_id = atoi(argv[3]);
        remove_treasure(argv[2], treasure_id);
    }else if(strcmp(argv[1], "--remove-hunt") == 0 && argc == 3){
        remove_hunt(argv[2]);
    }

    return 0;
}