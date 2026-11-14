#define _POSIX_C_SOURCE 199309L
#include "menu.h"
#include "game.h"
#include "draw.h"
#include "input.h"
#include "leaderboard.h"
#include <time.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define NAME_LEN 8

#define MAX(X, Y) (X) > (Y) ? (X) : (Y)
#define MIN(X, Y) (X) < (Y) ? (X) : (Y)

struct game_ctx;
extern struct game_ctx game_ctx;

static const char *items[MENUITEM_END] = {
	[MENUITEM_PLAY] = "play",
	[MENUITEM_SCORE] = "scoreboard",
	[MENUITEM_QUIT] = "quit",
};

static struct timespec t;

/* returns 1 if play, 0 if quit */
enum menuitem menu()
{
	enum menuitem m = MENUITEM_PLAY;
	int e = 0;

	t.tv_sec = 0;
	t.tv_nsec = 1000000000 / 60;

	while (!e) {
		draw_clear();
		draw_border();
		draw_score();

		for (enum menuitem i = 0; i < MENUITEM_END; i++) {
			set_cursor_board(WIDTH - strlen(items[i] + 1),
					(HEIGHT / (2 * MENUITEM_END)) + 2 * i);
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
				m = MIN(m + 1, MENUITEM_END - 1);
				break;
			case KEY_CHAR_Q:
				m = MENUITEM_QUIT;
				e = 1;
				break;
			case KEY_ENTER:
				e = 1;
			default:
				break;
		}

		draw_flush();
		nanosleep(&t, NULL);
	}

	return m;
}

void postgame()
{
	int e, i;
	char c;
	static char name[NAME_LEN];

	/* TODO: maybe not the prettiest. Maybe need some more robust
	 * modal mode? E.g.: input_set_text() and input_set_key()?
	 * `readline` maybe?
	 */
	input_finish();

	e = 0;
	i = 0;
	while (!e) {
		draw_clear();
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

		draw_flush();
	}
	input_init();

	leaderboard_commit(name);
}

void scoreboard()
{
}

