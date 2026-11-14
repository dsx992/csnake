#define _POSIX_C_SOURCE 199309L

#include <stdio.h>
#include "input.h"
#include <time.h>
#include <stdlib.h>
#include "rings.h"
#include "term.h"
#include <string.h>
#include <ctype.h>
#include "point.h"

#define MAX(X, Y) (X) > (Y) ? (X) : (Y)
#define MIN(X, Y) (X) < (Y) ? (X) : (Y)

#define LEADERBOARD "leaderboard"
#define WIDTH 12
#define HEIGHT 12

/* 20 fps -> 1 / 20 seconds -> (1 / 20) / 1'000'000'000 nanoseconds
 * -> 1'000'000'000 / 20 -> 100'000'000 / 2 */
const long tickrate = 7;
struct timespec r, s, t;

extern struct point termsz;

// const struct timespec rqtp = {
// 	.tv_sec = 0,
// 	.tv_nsec = 1000000000 / tickrate,
// };

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

RING(, enum direction, 2) direction;

static RING(, struct point, WIDTH * HEIGHT) snake;
/* keeps track of the snake so collisions is constant time */
static char map[WIDTH][HEIGHT];
static int alive;
static struct point apple;
static int score;

struct point tleft = {.x = 4, .y = 4};

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
		return 0;

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

#define SET_UNDERLINE "\033[4m"
#define SET_NORMAL "\033[0m"

int set_cursor(int x, int y)
{
	int ok;

	/* if cursor could not be set correctly */
	if (x < 1 || y < 1 || x >= termsz.x || y >= termsz.y)
		ok = 0;
	else
		ok = 1;

	printf("\033[%i;%iH", y, x);

	return ok;
}

/*
 * sets the cursor relative to the game board.
 * The board shall be centered in the terminal, therefore the top left corner
 * is:
 * 	x := (termsz.x / 2) - (WIDTH / 2)
 * 	y := (termsz.y / 2) - (HEIGHT / 2)
 * in the future maybe the scoreboard or other information will be next to the
 * game board, so the logic may be required to shift the relative top left
 * further.
 *
 * note: The board treats one unit as 1y and 2x.
 *
 * note: termsz is set in term.c. At some point it will be set on terminal size
 * change, so there may be a race condition.
 */
int set_cursor_board(int x, int y)
{
	int tx, ty;
	tx = (termsz.x / 2) - (WIDTH);
	ty = (termsz.y / 2) - (HEIGHT / 2);

	x = MAX(0, (MIN(tx + x, termsz.x)));
	y = MAX(0, (MIN(ty + y, termsz.y)));

	return set_cursor(x, y);

}

void draw_snake()
{
	struct point p;

	for(int i = 1; i < RING_COUNT(snake); i++) {
		p = RING_PEEK(i, snake);
		set_cursor_board(p.x * 2, p.y);
		if (i % 2)
			printf("[]");
		else
			printf("{}");
	}

	p = RING_PEEK_HEAD(snake);
	switch (RING_PEEK_HEAD(direction)) {
		case UP:
			set_cursor_board(p.x * 2, p.y);
			if (alive)
				printf("(v)");
			else
				printf("xx");
			break;
		case DOWN:
			if (alive) {
				set_cursor_board(p.x * 2 - 1, p.y);
				printf("(w)");
			}
			else {
				set_cursor_board(p.x * 2, p.y);
				printf("xx");
			}
			break;
		case RIGHT:
			set_cursor_board(p.x * 2, p.y);
			if (alive)
				printf("(v");
			else
				printf("xx");
			break;
		case LEFT:
			set_cursor_board(p.x * 2, p.y);
			if (alive)
				printf("v)");
			else
				printf("xx");
			break;
	}
}

void draw_border()
{
	/* top bar */
	set_cursor_board(-1, -1);
	for (int i = 0; i < WIDTH + 1; i++)
		printf("##");

	/* left and right columns */
	for (int i = 0; i < HEIGHT; i++) {
		set_cursor_board(-1, i);
		printf("#");
		set_cursor_board(WIDTH * 2, i);
		printf("#");
	}

	/* bottom bar */
	set_cursor_board(-1, HEIGHT);
	for (int i = 0; i < WIDTH + 1; i++)
		printf("##");
}

void draw_apple()
{
	set_cursor_board(apple.x * 2, apple.y);
	printf("@^");
}

void draw_score()
{
	if (RING_COUNT(snake) == 0)
		return;

	set_cursor_board(WIDTH - 4, -2);
	printf("score: %i", score);
}

void draw_leaderboard()
{

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

	while (RING_COUNT(snake))
		RING_REMOVE_TAIL(snake);

	while (RING_COUNT(direction))
		RING_REMOVE_HEAD(direction);

	for (int i = 0; i < WIDTH; i++)
		for (int j = 0; j < HEIGHT; j++)
			map[i][j] = 0;

	RING_PUSH_TAIL(((struct point){WIDTH / 2, HEIGHT / 2}), snake);
	RING_PUSH_TAIL(((struct point){WIDTH / 2 - 1, HEIGHT / 2}), snake);

	RING_PUSH_TAIL(RIGHT, direction);
	
	apple = (struct point){.x = WIDTH / 2 + 4, .y = HEIGHT / 2};

	score = 0;
	alive = 1;
	while(alive) {
		clock_gettime(CLOCK_REALTIME, &s);

		clear_screen();

		k = input_get_key();
		if (!handle_key(k))
			return 0;

		if (!move_snake())
			alive = 0;
	
		if (ate()) {
			score++;
			grow();
			move_apple();
		}
		draw_border();
		draw_snake();
		draw_apple();
		draw_score();
		set_cursor(0, HEIGHT + 2);
		fflush(stdout);

		clock_gettime(CLOCK_REALTIME, &t);
		t.tv_sec = 0;
		t.tv_nsec = (1000000000 / tickrate) - (t.tv_nsec - s.tv_nsec);
		nanosleep(&t, NULL);
	}

	return 1;
}

/* returns 1 if play, 0 if quit */
int menu()
{
	int m = 0;
	int e = 0;
	static const char *items[] = {
		"play",
		"quit",
	};

	t.tv_sec = 0;
	t.tv_nsec = (1000000000 / (tickrate * 4));

	while (!e) {
		clear_screen();
		draw_border();
		draw_score();

		for (int i = 0; i < LENGTH(items); i++) {
			set_cursor_board(WIDTH - strlen(items[i] + 1),
					(HEIGHT / 2) - 2 * (LENGTH(items)) + 2 * i);
			if (i == m)
				printf(SET_UNDERLINE "%s" SET_NORMAL, items[i]);
			else
				printf("%s", items[i]);
		}

		switch (input_get_key()) {
			case KEY_ARROW_UP:
				m = MAX(0, m - 1);
				break;
			case KEY_ARROW_DOWN:
				m = MIN(m + 1, LENGTH(items) - 1);
				break;
			case KEY_CHAR_Q:
				return 0;
			case KEY_ENTER:
				e = 1;
			default:
				break;
		}

		fflush(stdout);
		nanosleep(&t, NULL);
	}

	switch (m) {
		case 0:
			return 1;
		case 1:
			return 0;
	}

	return 0;
}

static size_t getline(char *buf, size_t n, FILE *s)
{
	size_t i;
	for(i = 0; i < n; i++) {
		if (!fread(buf + i, sizeof(char), 1, s)) {
			break;
		}
		if (buf[i] == '\n') {
			i++;
			break;
		}
	}
	return i;
}

/* leaderboard format:
 * 	name	50	unix epoch
 * 	name	30	unix epoch
 * 	name	10	unix epoch
 * 	is sorted by score
 */
int commit_score(const char *name)
{
	static char buf[128];
	static char new[128];
	int m, bn, nn, s, ok;
	time_t t;
	FILE *leader, *tmp;

	ok = 0;
	tmp = fopen("." LEADERBOARD, "w");
	leader = fopen(LEADERBOARD, "r");

	t = time(NULL);
	nn = name[0] == 0
		? snprintf(new, sizeof(new), "%s\t%i\t%lu\n", "<empty>", score, t)
		: snprintf(new, sizeof(new), "%s\t%i\t%lu\n", name, score, t);


	if (!tmp)
		goto done;

	/* copies and inserts into tmp (sorted) */
	if (leader) {
		while ((bn = getline(buf, sizeof(buf), leader))) {
			m = sscanf(buf, "%*s\t%i", &s);

			/* assert only one match */
			if (m != 1) {
				ok = 0;
				goto done;
			}

			if (s < score)
				break;
			
			fwrite(buf, bn, sizeof(char), tmp);
		}

		fwrite(new, nn, sizeof(char), tmp);
		/* may be a missing score from previous break. */
		if (bn)
			fwrite(buf, bn, sizeof(char), tmp);

		while ((bn = getline(buf, sizeof(buf), leader))) {
			m = sscanf(buf, "%*s\t%i", &s);

			/* assert only one match */
			if (m != 1) {
				ok = 0;
				goto done;
			}

			fwrite(buf, bn, sizeof(char), tmp);
		}
		fclose(leader);
	} else {
		/* if leaderboard does not exist, just write the current
		 * score. */
		fwrite(new, nn, sizeof(char), tmp);
	}

	fclose(tmp);
	
	tmp = fopen("." LEADERBOARD, "r");
	leader = fopen(LEADERBOARD, "w");

	if (tmp && leader) {
		while ((nn = getline(buf, sizeof(buf), tmp)))
			fwrite(buf, nn, sizeof(char), leader);
		ok = 1;
	}

done:
	if (tmp)
		fclose(tmp);
	if (leader)
		fclose(leader);

	remove("." LEADERBOARD);

	return ok;
}

void postgame()
{
	int e, i;
	char c;
	static char name[8];


	/* TODO: maybe not the prettiest. Maybe need some more robust
	 * modal mode? E.g.: input_set_text() and input_set_key()?
	 * `readline` maybe?
	 */
	input_finish();

	e = 0;
	i = 0;
	while (!e) {
		clear_screen();
		draw_border();
		draw_score();
		draw_snake();

		name[i] = 0;
		set_cursor_board(WIDTH / 2 - (int)sizeof(name) , HEIGHT / 2);
		printf("name: %s", name);

		/* TODO: backspace */
		c = getchar();

		/* still want the drawing, so no break */
		if (c == '\n')
			e = 1;
		else if (isalnum(c) && i < sizeof(name) - 1)
			name[i++] = c;

		fflush(stdout);
	}
	input_init();

	commit_score(name);

}

int main()
{
	input_init();
	term_init();

	clear_screen();

	while (1) {
		if (!menu())
			break;
		game();
		postgame();
	}

	input_finish();
	term_finish();
	printf("score: %i\n", score);
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
