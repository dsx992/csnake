#ifndef GAME_
#define GAME_

#include "point.h"
#include "rings.h"
#include "draw.h"
struct draw_ctx;

#define WIDTH 12
#define HEIGHT 12

enum direction {
	UP,
	DOWN,
	LEFT,
	RIGHT
};

struct game_ctx {
	int alive;
	int score;
	int quit;
	struct point apple;
	RING( , struct point, WIDTH * HEIGHT) snake;
	RING( , enum direction, 2) direction;
	char collision[WIDTH][HEIGHT];
};

int game();

#endif
