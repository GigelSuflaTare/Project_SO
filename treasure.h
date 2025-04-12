#ifndef TREASURE_H

#define TREASURE_H
#define USERNAME_SIZE 32
#define CLUE_SIZE 128

typedef struct{
	int treasure_id;
	char username[USERNAME_SIZE];
	float latitude;
	float longitude;
	char clue[CLUE_SIZE];
	int value;
} Treasure;

#endif
