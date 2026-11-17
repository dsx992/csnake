#ifndef GAME_
#define GAME_

#include "point.h"
#include "rings.h"
#include "draw.h"
#include "menu.h"

struct draw_ctx;

enum direction {
	UP,
	DOWN,
	LEFT,
	RIGHT
};

struct game_ctx {
	/* set in main by argv*/
	int w, h;
	int alive;
	int score;
	int quit;
	int tickrate;
	struct point apple;
	// RING( , struct point, WIDTH * HEIGHT) snake;
	// RING( , enum direction, 2) direction;
	RING( , struct point) snake;
	RING( , enum direction) direction;
	enum direction previous_direction;
	// char collision[WIDTH][HEIGHT];
	char *collision;
};
void initialize_game_ctx();

enum menuitem game();

#endif
