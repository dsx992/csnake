/*
 * Exposes the core game loop.
 */

/* needed for clock_gettime and nanosleep */
#define _POSIX_C_SOURCE 199309L
#include "game.h"
#include "draw.h"
#include "input.h"
#include <time.h>
#include <stdlib.h>

#define MAX(X, Y) (X) > (Y) ? (X) : (Y)
#define MIN(X, Y) (X) < (Y) ? (X) : (Y)

static const long tickrate = 7;
volatile struct game_ctx game_ctx;

static void grow();
static int eat();
static void move_apple();
static int move_snake();
static int handle_key(enum key);
static void reset_game_ctx();

static struct timespec s, t;

int game()
{
	enum key k;

	reset_game_ctx();

	while(game_ctx.alive && !game_ctx.quit) {
		clock_gettime(CLOCK_REALTIME, &s);

		draw_clear();

		k = input_get_key();

		if (!handle_key(k))
			break;

		if (!move_snake())
			game_ctx.alive = 0;
	
		if (eat()) {
			game_ctx.score++;
			grow();
			move_apple();
		}
		draw_border();
		draw_snake();
		draw_apple();
		draw_score();
		draw_flush();

		clock_gettime(CLOCK_REALTIME, &t);
		t.tv_sec = 0;
		t.tv_nsec = (1000000000 / tickrate) - (t.tv_nsec - s.tv_nsec);
		nanosleep(&t, NULL);
	}

	return 1;
}

static void grow()
{
	struct point t = RING_PEEK_TAIL(game_ctx.snake);
	RING_PUSH_TAIL(t, game_ctx.snake);
}

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

	x = rand() % WIDTH;
	y = rand() % HEIGHT;
	for (i = 0; i < WIDTH; i++)
		for (j = 0; j < HEIGHT; i++)
			if (!game_ctx.collision[(x + i) % WIDTH][(y + j) % HEIGHT])
				goto done;

done:

	game_ctx.apple.x = (x + i) % WIDTH;
	game_ctx.apple.y = (y + j) % HEIGHT;
}

static int move_snake()
{
	struct point h, t, tt;

	h = RING_PEEK_HEAD(game_ctx.snake);

	switch (RING_PEEK_HEAD(game_ctx.direction)) {
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

	if (h.x >= WIDTH || h.x < 0
			|| h.y >= HEIGHT || h.y < 0
			|| (game_ctx.collision[h.x][h.y]))
		return 0;

	t = RING_PEEK_TAIL(game_ctx.snake);

	RING_REMOVE_TAIL(game_ctx.snake);
	RING_PUSH_HEAD(h, game_ctx.snake);

	/* next tail */
	tt = RING_PEEK_TAIL(game_ctx.snake);

	game_ctx.collision[h.x][h.y] = 1;
	game_ctx.collision[t.x][t.y] = 0;
	/* in case of `grow()`, the tail would have two elements, but this
	 * would not be reflected in map without explicitly painting in the
	 * duplicate tail. */
	game_ctx.collision[tt.x][tt.y] = 1;

	if (RING_COUNT(game_ctx.direction) > 1)
		RING_REMOVE_HEAD(game_ctx.direction);

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
		case KEY_ENTER:
		case KEY_INVALID:
		case KEY_UNKNOWN:
			break;
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

	for (int i = 0; i < WIDTH; i++)
		for (int j = 0; j < HEIGHT; j++)
			game_ctx.collision[i][j] = 0;

	RING_PUSH_TAIL(((struct point){WIDTH / 2, HEIGHT / 2}), game_ctx.snake);
	RING_PUSH_TAIL(((struct point){WIDTH / 2 - 1, HEIGHT / 2}), game_ctx.snake);

	RING_PUSH_TAIL(RIGHT, game_ctx.direction);

	game_ctx.score = 0;
	game_ctx.quit = 0;
	game_ctx.alive = 1;
	game_ctx.apple = (struct point){.x = WIDTH / 2 + 4, .y = HEIGHT / 2};
}
