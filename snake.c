#define _POSIX_C_SOURCE 199309L

#include <stdio.h>
#include "input.h"
#include <time.h>
#include <stdlib.h>
#include "rings.h"
#include "term.h"

#define MAX(X, Y) (X) > (Y) ? (X) : (Y)
#define MIN(X, Y) (X) < (Y) ? (X) : (Y)

#define WIDTH 16
#define HEIGHT 16

struct point{
	int x, y;
};

/* 20 fps -> 1 / 20 seconds -> (1 / 20) / 1'000'000'000 nanoseconds
 * -> 1'000'000'000 / 20 -> 100'000'000 / 2 */
const long tickrate = 100000000 / 6;
const struct timespec rqtp = {
	.tv_sec = 0,
	.tv_nsec = tickrate,
};

void clear_screen()
{
	printf("\033[2J");
}

int handle_key(enum key);

enum direction {
	UP,
	DOWN,
	LEFT,
	RIGHT
};

RING(, enum direction, 4) direction;

const int speed = 7;
static RING(, struct point, WIDTH * HEIGHT) snake;
/* keeps track of the snake so collisions is constant time */
static char map[WIDTH][HEIGHT];
static int dead;
static struct point apple;
static int points;

struct point tleft = {.x = 2, .y = 4};

int die()
{
	struct point h, p;

	h = RING_PEEK_HEAD(snake);

	if (h.x > WIDTH || h.x < 0
			|| h.y > HEIGHT || h.y < 0)
		return 1;

	for (int i = 1; i < RING_COUNT(snake); i++) {
		p = RING_PEEK(i, snake);
		if (h.x == p.x && h.y == p.y)
			return 1;
	}

	return 0;
}

int move_snake()
{
	struct point h, t, tt;

	enum direction d = RING_PEEK_HEAD(direction);
	h = RING_PEEK_HEAD(snake);

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

	if (h.x >= WIDTH || h.x < 0
			|| h.y >= HEIGHT || h.y < 0
			|| (map[h.x][h.y]))
		return 1;

	t = RING_PEEK_TAIL(snake);

	RING_REMOVE_TAIL(snake);
	RING_PUSH_HEAD(h, snake);

	/* next tail */
	tt = RING_PEEK_TAIL(snake);

	map[h.x][h.y] = 1;
	map[t.x][t.y] = 0;
	/* in case of `grow()`, the tail would have two elements, but this
	 * would not be reflected in map without explicitly painting in the
	 * duplicate tail. */
	map[tt.x][tt.y] = 1;

	if (RING_COUNT(direction) > 1)
		RING_REMOVE_HEAD(direction);

	return 1;
}

void set_cursor(int x, int y)
{
	printf("\033[%i;%iH", y + tleft.y, x + tleft.x);
}

void draw_snake()
{
	struct point p;

	for(int i = 1; i < RING_COUNT(snake); i++) {
		p = RING_PEEK(i, snake);
		set_cursor(p.x * 2, p.y);
		if (i % 2)
			printf("[]");
		else
			printf("{}");
	}

	p = RING_PEEK_HEAD(snake);
	switch (RING_PEEK_HEAD(direction)) {
		case UP:
			set_cursor(p.x * 2, p.y);
			if (dead)
				printf("xx");
			else
				printf("(v)");
			break;
		case DOWN:
			if (dead) {
				set_cursor(p.x * 2, p.y);
				printf("xx");

			}
			else {
				set_cursor(p.x * 2 - 1, p.y);
				printf("(w)");
			}
			break;
		case RIGHT:
			set_cursor(p.x * 2, p.y);
			if (dead)
				printf("xx");
			else
				printf("(v");
			break;
		case LEFT:
			set_cursor(p.x * 2, p.y);
			if (dead)
				printf("xx");
			else
				printf("v)");
			break;
	}
}

void draw_border()
{
	/* top bar */
	set_cursor(-1, -1);
	for (int i = 0; i < WIDTH + 1; i++)
		printf("##");

	/* left and right columns */
	for (int i = 0; i < HEIGHT; i++) {
		set_cursor(-1, i);
		printf("#");
		set_cursor(WIDTH * 2, i);
		printf("#");
	}

	/* bottom bar */
	set_cursor(-1, HEIGHT);
	for (int i = 0; i < WIDTH + 1; i++)
		printf("##");
}

void draw_apple()
{
	set_cursor(apple.x * 2, apple.y);
	printf("@^");
}

void draw_points()
{
	set_cursor(WIDTH - 4, -2);
	printf("points: %i", points);
}

void move_apple()
{
	/* finds space not occupied by the snake in O(H * W) ~ O(n^2) time.
	 * Maybe there is a more efficient approach?
	 */
	int i, j, x, y;

	x = rand() % WIDTH;
	y = rand() % HEIGHT;
	for (i = 0; i < WIDTH; i++)
		for (j = 0; j < HEIGHT; i++)
			if (!map[(x + i) % WIDTH][(y + j) % HEIGHT])
				goto done;

done:



	apple.x = (x + i) % WIDTH;
	apple.y = (y + j) % HEIGHT;
}

int ate()
{
	struct point h = RING_PEEK_HEAD(snake);
	return (h.x == apple.x && h.y == apple.y);
}

void grow()
{
	struct point t = RING_PEEK_TAIL(snake);
	RING_PUSH_TAIL(t, snake);
}

int game()
{
	enum key k;
	int clock;

	while (RING_COUNT(snake))
		RING_REMOVE_TAIL(snake);

	while (RING_COUNT(direction))
		RING_REMOVE_HEAD(direction);

	RING_PUSH_TAIL(((struct point){WIDTH / 2, HEIGHT / 2}), snake);
	RING_PUSH_TAIL(((struct point){WIDTH / 2 - 1, HEIGHT / 2}), snake);

	RING_PUSH_TAIL(RIGHT, direction);
	
	apple = (struct point){.x = WIDTH / 2 + 4, .y = HEIGHT / 2};

	points = 0;
	clock = 0;
	while(1) {
		clear_screen();
		set_cursor(-tleft.x, HEIGHT + 4);
		clock = (clock + 1) % speed;

		k = input_get_key();
		if (!handle_key(k))
			return 0;

		if (!clock) {
			if (!move_snake())
				break;
		
			if (ate()) {
				points++;
				grow();
				move_apple();
			}

		}
		draw_border();
		draw_snake();
		draw_apple();
		draw_points();
		set_cursor(0, HEIGHT + 2);
		fflush(stdout);

		nanosleep(&rqtp, NULL);
	}

	return 1;
}

int main()
{
	enum key k;

	input_init();
	term_init();

	clear_screen();

	while (1) {
		if (!game())
			goto done;
		while (1) {
			k = input_get_key();
			if (k == KEY_CHAR_Q)
				goto done;
			else if (k == KEY_ENTER)
				break;
			nanosleep(&rqtp, NULL);
		}
	}
done:
	input_finish();
	term_finish();
	printf("score: %i\n", points);
	return 0;
}

int handle_key(enum key k)
{
	switch (k) {
		case KEY_ARROW_LEFT:
			if (RING_PEEK_TAIL(direction) != RIGHT)
				RING_PUSH_TAIL(LEFT, direction);
			break;
		case KEY_ARROW_DOWN:
			if (RING_PEEK_TAIL(direction) != UP)
				RING_PUSH_TAIL(DOWN, direction);
			break;
		case KEY_ARROW_UP:
			if (RING_PEEK_TAIL(direction) != DOWN)
				RING_PUSH_TAIL(UP, direction);
			break;
		case KEY_ARROW_RIGHT:
			if (RING_PEEK_TAIL(direction) != LEFT)
				RING_PUSH_TAIL(RIGHT, direction);
			break;
		case KEY_ENTER:
		case KEY_INVALID:
		case KEY_UNKNOWN:
			break;
		case KEY_CHAR_Q:
			return 0;
	}
	return 1;
}
