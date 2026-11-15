#ifndef LEADERBOARD_
#define LEADERBOARD_

#define NAME_LEN 16

#include <stdio.h>

struct leaderboard_entry {
	int score;
	char name[NAME_LEN];
	long unsigned int unix;
};

int leaderboard_commit(const char *name);
int leaderboard_entry(FILE *, struct leaderboard_entry *);
FILE *leaderboard_open();
int leaderboard_default();

#endif
