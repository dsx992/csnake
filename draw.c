#include <stdio.h>
#include "draw.h"
#include "game.h"
#include "term.h"
#include "leaderboard.h"

#define MAX(X, Y) ((X) > (Y) ? (X) : (Y))
#define MIN(X, Y) ((X) < (Y) ? (X) : (Y))

struct game_ctx;
struct term_ctx;
extern struct game_ctx game_ctx;
extern struct term_ctx term_ctx;

/* TODO: define drawing primitives and use those */

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
 * in the future maybe the leaderboard or other information will be next to the
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
	tx = (term_ctx.w / 2) - (game_ctx.w);
	ty = (term_ctx.h / 2) - (game_ctx.h / 2);

	x = MAX(0, (MIN(tx + x, term_ctx.w)));
	y = MAX(0, (MIN(ty + y, term_ctx.h)));

	return set_cursor(x, y);
}

enum snake_parts {
	SNAKE_HEAD,
	SNAKE_BODY_1,
	SNAKE_BODY_2,
};

enum snake_state {
	SNAKE_ALIVE,
	SNAKE_DEAD,
};

static const char *snake_sprite[SNAKE_BODY_2 + 1][RIGHT + 1][SNAKE_DEAD + 1] = {
	[SNAKE_HEAD] = {
		[UP] = {
			[SNAKE_ALIVE] = ":U",
			[SNAKE_DEAD] = "xp"
		},
		[DOWN] = {
			[SNAKE_ALIVE] = "n:",
			[SNAKE_DEAD] = "dx"
		},
		[LEFT] = {
			[SNAKE_ALIVE] = ")\"",
			[SNAKE_DEAD] = "dx"
		},
		[RIGHT] = {
			[SNAKE_ALIVE] = "\"C",
			[SNAKE_DEAD] = "xp"
		},
	},
	/* currently do not use direction. In the future each snake point may
	 * keep track of their direction, i.e., the direction from when they
	 * were head. */
	[SNAKE_BODY_1] = {
		[UP] = {
			[SNAKE_ALIVE] = "[]",
			[SNAKE_DEAD] = "[]"
		},
	},
	[SNAKE_BODY_2] = {
		[UP] = {
			[SNAKE_ALIVE] = "{}",
			[SNAKE_DEAD] = "{}"
		},
	},
};

void draw_snake()
{
	struct point p;
	enum direction d;
	enum snake_state s;

	/* draw body */
	for(int i = 1; i < RING_COUNT(game_ctx.snake); i++) {
		p = RING_PEEK(i, game_ctx.snake);
		set_cursor_board(p.x * 2, p.y);
		printf("%s", snake_sprite[SNAKE_BODY_1 + i % 2][UP][SNAKE_ALIVE]);
	}

	/* draw head */
	p = RING_PEEK_HEAD(game_ctx.snake);
	d = RING_COUNT(game_ctx.direction)
		? RING_PEEK_HEAD(game_ctx.direction)
		: game_ctx.previous_direction;
	s = game_ctx.alive ? SNAKE_ALIVE : SNAKE_DEAD;

	set_cursor_board(p.x * 2, p.y);
	printf("%s", snake_sprite[SNAKE_HEAD][d][s]);
}

void draw_border()
{
	/* top bar */
	set_cursor_board(-1, -1);
	for (int i = 0; i < game_ctx.w + 1; i++)
		printf("##");

	/* left and right columns */
	for (int i = 0; i < game_ctx.h; i++) {
		set_cursor_board(-1, i);
		printf("#");
		set_cursor_board(game_ctx.w * 2, i);
		printf("#");
	}

	/* bottom bar */
	set_cursor_board(-1, game_ctx.h);
	for (int i = 0; i < game_ctx.w + 1; i++)
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

	set_cursor_board(game_ctx.w - 4, -2);
	printf("score: %i", game_ctx.score);
}

/*
 * lists at most the `l` top entries.
 *
 *
 *	     leaderboard
 * 	1. <name>........20
 *	2. <name>........10
 *	3. <looooooong>..5
 *
 * the scores are placed at column NAME_LEN + 2 after the numbering.
 *
 * TODO: something `min()` to accomodate small spaces.
 * TODO: alignment when l > 9 (10. ).
 * TODO: respect size bounds better.
 */
void draw_leaderboard(struct point tl, struct point br, int l)
{
	int h, w, p;
	const char title[] = "leaderboard";
	struct leaderboard_entry entry;
	FILE *board = leaderboard_open();

	if (!board)
		return;

	w = br.x - tl.x;
	h = br.y - tl.x;

	set_cursor_board(tl.x + ((w - sizeof(title) + 1) / 2), tl.y);
	printf(SET_UNDERLINE "%s" SET_NORMAL, title);

	for (int i = 1; i <= l || l == -1; i++) {
		if (!(tl.y + i < h))
			break;

		if (!leaderboard_entry(board, &entry))
			break;

		set_cursor_board(tl.x, tl.y + i);
		printf("%i. ", i);
		p = printf("%s", entry.name);

		/* - 3 - 3 refers to the first numbers columns assumed size and
		 * the assumed max length of the score.
		 * TODO: make not ugly.
		 */
		for (int j = p; j < MIN(NAME_LEN + 2, w - 3 - 3); j++)
			printf(".");

		printf("%i", entry.score);
	}

	fclose(board);
}
