/*
 * Exposes the core game loop.
 */

/* needed for clock_gettime and nanosleep */
#include "common.h"
#include "arena.h"
#include "game.h"
#include "draw.h"
#include "input.h"
#include "menu.h"
#include <time.h>
#include <stdlib.h>

#define DEFAULT_WIDTH 16
#define DEFAULT_HEIGHT 16
#define DEFAULT_TICKRATE 7

volatile struct game_ctx game_ctx;

enum key {
	KEY_CHAR_Q,
	KEY_ARROW_UP,
	KEY_ARROW_DOWN,
	KEY_ARROW_LEFT,
	KEY_ARROW_RIGHT,
	KEY_ESC,
};

static void grow();
static int eat();
static void move_apple();
static int move_snake();
static int handle_key(enum key);
static void reset_game_ctx();

static struct timespec s, t;

static struct keymap keymap[] = {
	{"\033[A"	, KEY_ARROW_UP},
	{"\033[B"	, KEY_ARROW_DOWN},
	{"\033[C"	, KEY_ARROW_RIGHT},
	{"\033[D"	, KEY_ARROW_LEFT},
	{"\033"		, KEY_ESC},
	{"q"		, KEY_CHAR_Q},
};

enum menuitem game()
{
	enum key k;
	enum menuitem m = MENUITEM_POST;

	reset_game_ctx();
	input_setreal(keymap, LENGTH(keymap));

	while(game_ctx.alive && !game_ctx.quit) {
		clock_gettime(CLOCK_REALTIME, &s);

		draw_clear();
		draw_border();
		draw_snake();
		draw_apple();
		draw_score();
		draw_flush();

		clock_gettime(CLOCK_REALTIME, &t);
		t.tv_sec = 0;
		t.tv_nsec = (1000000000 / game_ctx.tickrate) - (t.tv_nsec - s.tv_nsec);
		nanosleep(&t, NULL);

		k = input_get();

		if (!handle_key(k)) {
			m = MENUITEM_MENU;
			break;
		}

		if (!move_snake())
			game_ctx.alive = 0;
	
		if (eat()) {
			game_ctx.score++;
			grow();
			move_apple();
		}
	}

	input_setnoreal();
	return m;
}

void initialize_game_ctx()
{
	static int initialized;

	if (initialized)
		return;

	if (!game_ctx.w)
		game_ctx.w = DEFAULT_WIDTH;
	if (!game_ctx.h)
		game_ctx.h = DEFAULT_HEIGHT;
	if (!game_ctx.tickrate)
		game_ctx.tickrate = DEFAULT_TICKRATE;

	 RING_INITIALIZE(game_ctx.snake, game_ctx.w * game_ctx.h);
	 RING_INITIALIZE(game_ctx.direction, 2);
	 game_ctx.collision = arena_malloc(game_ctx.w * game_ctx.h);

	initialized = 1;
}

static void grow()
{
	struct point t = RING_PEEK_TAIL(game_ctx.snake);
	RING_PUSH_TAIL(t, game_ctx.snake);
}

/* TODO: snake doesn't eat right anymore */
static int eat()
{
	struct point h = RING_PEEK_HEAD(game_ctx.snake);
	return (h.x == game_ctx.apple.x && h.y == game_ctx.apple.y);
}

static void move_apple()
{
	/* finds space not occupied by the snake in O(H * W) ~ O(n^2) time.
	 * Maybe there is a more efficient approach?
	 *
	 * note: maybe return bool; if no more room then won?
	 */
	int i, j, x, y;

	x = rand() % game_ctx.w;
	y = rand() % game_ctx.h;
	for (i = 0; i < game_ctx.w; i++)
		for (j = 0; j < game_ctx.h; i++)
			// if (!game_ctx.collision[(x + i) % WIDTH][(y + j) % HEIGHT])
			if (!game_ctx.collision[(x + i) % game_ctx.w +
					((y + j) % game_ctx.h) * game_ctx.w])
				goto done;

done:

	game_ctx.apple.x = (x + i) % game_ctx.w;
	game_ctx.apple.y = (y + j) % game_ctx.h;
}

static int move_snake()
{
	struct point h, t, tt;
	enum direction d;

	h = RING_PEEK_HEAD(game_ctx.snake);
	if (RING_COUNT(game_ctx.direction)) {
		d = RING_PEEK_HEAD(game_ctx.direction);
		RING_REMOVE_HEAD(game_ctx.direction);
	} else {
		d = game_ctx.previous_direction;
	}

	switch (d) {
		case UP:
			h.y--;
			break;
		case DOWN:
			h.y++;
			break;
		case LEFT:
			h.x--;
			break;
		case RIGHT:
			h.x++;
			break;
	}

	if (h.x >= game_ctx.w || h.x < 0
			|| h.y >= game_ctx.h || h.y < 0
			|| (game_ctx.collision[h.x + (h.y * game_ctx.w)]))
		return 0;

	t = RING_PEEK_TAIL(game_ctx.snake);

	RING_REMOVE_TAIL(game_ctx.snake);
	RING_PUSH_HEAD(h, game_ctx.snake);

	/* next tail */
	tt = RING_PEEK_TAIL(game_ctx.snake);

	game_ctx.collision[h.x + (h.y * game_ctx.w)] = 1;
	game_ctx.collision[t.x + (t.y * game_ctx.w)] = 0;
	/* in case of `grow()`, the tail would have two elements, but this
	 * would not be reflected in map without explicitly painting in the
	 * duplicate tail. */
	game_ctx.collision[tt.x + (tt.y * game_ctx.w)] = 1;

	game_ctx.previous_direction = d;

	return 1;
}

static int handle_key(enum key k)
{
	switch (k) {
		case KEY_ARROW_LEFT:
			if (RING_PEEK_TAIL(game_ctx.direction) != RIGHT)
				RING_PUSH_TAIL(LEFT, game_ctx.direction);
			break;
		case KEY_ARROW_DOWN:
			if (RING_PEEK_TAIL(game_ctx.direction) != UP)
				RING_PUSH_TAIL(DOWN, game_ctx.direction);
			break;
		case KEY_ARROW_UP:
			if (RING_PEEK_TAIL(game_ctx.direction) != DOWN)
				RING_PUSH_TAIL(UP, game_ctx.direction);
			break;
		case KEY_ARROW_RIGHT:
			if (RING_PEEK_TAIL(game_ctx.direction) != LEFT)
				RING_PUSH_TAIL(RIGHT, game_ctx.direction);
			break;
		case KEY_ESC:
		case KEY_CHAR_Q:
			game_ctx.quit = 1;
			return 0;
	}
	return 1;
}

static void reset_game_ctx()
{
	while (RING_COUNT(game_ctx.snake))
		RING_REMOVE_TAIL(game_ctx.snake);

	while (RING_COUNT(game_ctx.direction))
		RING_REMOVE_TAIL(game_ctx.direction);

	for (int i = 0; i < game_ctx.w; i++)
		for (int j = 0; j < game_ctx.h; j++)
			// game_ctx.collision[i][j] = 0;
			game_ctx.collision[i + (j * game_ctx.w)] = 0;

	RING_PUSH_TAIL(((struct point){game_ctx.w / 2, game_ctx.h / 2}), game_ctx.snake);
	RING_PUSH_TAIL(((struct point){game_ctx.w / 2 - 1, game_ctx.h / 2}), game_ctx.snake);

	game_ctx.previous_direction = RIGHT;

	game_ctx.score = 0;
	game_ctx.quit = 0;
	game_ctx.alive = 1;
	game_ctx.apple = (struct point){.x = game_ctx.w / 2 + 4, .y = game_ctx.h / 2};
}
