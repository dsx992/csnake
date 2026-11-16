#ifndef GAME_
#define GAME_

#include "point.h"
#include "rings.h"
#include "draw.h"
#include "menu.h"

struct draw_ctx;

/* TODO: these should be set at runtime. Needs changing of RING as well. */
#define WIDTH 16
#define HEIGHT 16

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

enum menuitem game();

#endif
