#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "treasure.h"

typedef struct {
    char username[64];
    int score;
} UserScore;

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s treasures.dat\n", argv[0]);
        return 1;
    }
    FILE *f = fopen(argv[1], "rb");
    if (!f) {
        perror("fopen");
        return 1;
    }
    UserScore users[100];
    int user_count = 0;
    Treasure t;
    while (fread(&t, sizeof(Treasure), 1, f) == 1) {
        int found = 0;
        for (int i = 0; i < user_count; ++i) {
            if (strcmp(users[i].username, t.username) == 0) {
                users[i].score += t.value;
                found = 1;
                break;
            }
        }
        if (!found && user_count < 100) {
            strcpy(users[user_count].username, t.username);
            users[user_count].score = t.value;
            user_count++;
        }
    }
    fclose(f);
    for (int i = 0; i < user_count; ++i) {
        printf("%s: %d\n", users[i].username, users[i].score);
    }
    return 0;
}