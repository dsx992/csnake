#include <stdio.h>
#include "draw.h"
#include "game.h"
#include "term.h"

#define MAX(X, Y) (X) > (Y) ? (X) : (Y)
#define MIN(X, Y) (X) < (Y) ? (X) : (Y)

struct game_ctx;
struct term_ctx;
extern struct game_ctx game_ctx;
extern struct term_ctx term_ctx;

void draw_clear()
{
	printf("\033[2J");
}

void draw_flush()
{
	fflush(stdout);
}

int set_cursor(int x, int y)
{
	int ok;

	/* if cursor could not be set correctly */
	if (x < 1 || y < 1 || x >= term_ctx.w || y >= term_ctx.h)
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
	tx = (term_ctx.w / 2) - (WIDTH);
	ty = (term_ctx.h / 2) - (HEIGHT / 2);

	x = MAX(0, (MIN(tx + x, term_ctx.w)));
	y = MAX(0, (MIN(ty + y, term_ctx.h)));

	return set_cursor(x, y);
}

void draw_snake()
{
	struct point p;

	for(int i = 1; i < RING_COUNT(game_ctx.snake); i++) {
		p = RING_PEEK(i, game_ctx.snake);
		set_cursor_board(p.x * 2, p.y);
		if (i % 2)
			printf("[]");
		else
			printf("{}");
	}

	p = RING_PEEK_HEAD(game_ctx.snake);
	switch (RING_PEEK_HEAD(game_ctx.direction)) {
		case UP:
			set_cursor_board(p.x * 2, p.y);
			if (game_ctx.alive)
				printf("(v)");
			else
				printf("xx");
			break;
		case DOWN:
			if (game_ctx.alive) {
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
			if (game_ctx.alive)
				printf("(v");
			else
				printf("xx");
			break;
		case LEFT:
			set_cursor_board(p.x * 2, p.y);
			if (game_ctx.alive)
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
	set_cursor_board(game_ctx.apple.x * 2, game_ctx.apple.y);
	printf("@^");
}

void draw_score()
{
	if (RING_COUNT(game_ctx.snake) == 0)
		return;

	set_cursor_board(WIDTH - 4, -2);
	printf("score: %i", game_ctx.score);
}

/*
 * lists at most the `l` top entries. As such it needs n + 1 columns of space
 *
 *	     leaderboard
 * 	1. <name>........20
 *	2. <name>........10
 *	3. <looooooong>..5
 *
 * the scores are placed at column NAME_LEN + 2
 */
void draw_leaderboard(struct point tl, struct point br, int n, int l)
{

}
