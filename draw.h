#ifndef DRAW_
#define DRAW_

#include "game.h"

#define SET_UNDERLINE "\033[4m"
#define SET_NORMAL "\033[0m"

struct draw_ctx {
	int w;
	int h;
};

void draw_clear();
void draw_flush();
int set_cursor(int x, int y);
int set_cursor_board(int x, int y);
void draw_snake();
void draw_border();
void draw_apple();
void draw_score();
void draw_leaderboard(struct point tl, struct point br, int l);

#endif
