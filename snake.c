#define _POSIX_C_SOURCE 199309L

#include <stdio.h>
#include "input.h"
#include <time.h>
#include <stdlib.h>
#include "rings.h"

#ifndef MAX
#define MAX(X, Y) X > Y ? X : Y
#endif

#define WIDTH 16
#define HEIGHT 16

char * keys[] = {
	"KEY_NULL",
	"KEY_UP",
	"KEY_DOWN",
	"KEY_LEFT",
	"KEY_RIGHT",
	"KEY_Q"
};


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

char *dmap[] = {
	"UP",
	"DOWN",
	"LEFT",
	"RIGHT"
};

RING(, enum direction, 4) direction;

const int speed = 7;
RING(ring, struct point, WIDTH * HEIGHT) snake;
struct point apple;
int points;

struct point tleft = {.x = 10, .y = 10};

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

void move_snake()
{
	struct point h = RING_PEEK_HEAD(snake);
	enum direction d = RING_PEEK_HEAD(direction);

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
	};
	if (RING_COUNT(direction) > 1)
		RING_REMOVE_HEAD(direction);

	RING_REMOVE_TAIL(snake);
	RING_PUSH_HEAD(h, snake);
}

void set_cursor(int x, int y)
{
	printf("\033[%i;%iH", y + tleft.y, x + tleft.x);
}

void draw_snake()
{
	struct point p;

	p = RING_PEEK_HEAD(snake);
	switch (RING_PEEK_HEAD(direction)) {
		case UP:
			set_cursor(p.x * 2, p.y);
			printf("(v)");
			break;
		case DOWN:
			set_cursor(p.x * 2 - 1, p.y);
			printf("(w)");
			break;
		case RIGHT:
			set_cursor(p.x * 2, p.y);
			printf("(v");
			break;
		case LEFT:
			set_cursor(p.x * 2, p.y);
			printf("v)");
			break;
	}

	for(int i = 1; i < RING_COUNT(snake); i++) {
		p = RING_PEEK(i, snake);
		set_cursor(p.x * 2, p.y);
		if (i % 2)
			printf("[]");
		else
			printf("{}");
	}
}

void draw_border()
{
	/* top bar */
	set_cursor(-1, -1);
	for (int i = 0; i < WIDTH + 2; i++)
		printf("##");

	/* left and right columns */
	for (int i = 0; i < HEIGHT + 1; i++) {
		set_cursor(-1, i);
		printf("#");
		set_cursor(WIDTH * 2 + 2, i);
		printf("#");
	}

	/* bottom bar */
	set_cursor(-1, HEIGHT + 1);
	for (int i = 0; i < WIDTH + 2; i++)
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
	/* TODO: should not be apple to move on top of snake
	 * Rerolling on collision could get really bad..
	 */
	apple.x = rand() % WIDTH;
	apple.y = rand() % HEIGHT;
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

		k = input_get();
		if (!handle_key(k))
			return 0;

		if (!clock) {
			move_snake();

			if (die())
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

void menu()
{
	// static char* mitems[] = {
	// 	"retry",
	// 	"quit"
	// };


}

int main()
{
	enum key k;
	input_init();
	// atexit(&input_finish);

	clear_screen();

	while (1) {
		if (!game())
			goto done;
		while (1) {
			k = input_get();
			if (k == KEY_Q)
				goto done;
			else if (k == KEY_ENTER)
				break;
			nanosleep(&rqtp, NULL);
		}
	}
done:
	input_finish();
	printf("score: %i\n", points);
	return 0;
}

int handle_key(enum key k)
{
	switch (k) {
		case KEY_LEFT:
			if (RING_PEEK_TAIL(direction) != RIGHT)
				RING_PUSH_TAIL(LEFT, direction);
			break;
		case KEY_DOWN:
			if (RING_PEEK_TAIL(direction) != UP)
				RING_PUSH_TAIL(DOWN, direction);
			break;
		case KEY_UP:
			if (RING_PEEK_TAIL(direction) != DOWN)
				RING_PUSH_TAIL(UP, direction);
			break;
		case KEY_RIGHT:
			if (RING_PEEK_TAIL(direction) != LEFT)
				RING_PUSH_TAIL(RIGHT, direction);
			break;
		case KEY_ENTER:
		case KEY_NULL:
			break;
		case KEY_Q:
			return 0;
	}
	return 1;
}
