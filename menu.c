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

#define MAX(X, Y) ((X) > (Y) ? (X) : (Y))
#define MIN(X, Y) ((X) < (Y) ? (X) : (Y))

struct game_ctx;
extern struct game_ctx game_ctx;

static const char *items[MENUITEM_END] = {
	[MENUITEM_PLAY] = "play",
	[MENUITEM_SCORE] = "leaderboard",
	[MENUITEM_HELP] = "help",
	[MENUITEM_QUIT] = "quit",
};

static struct timespec t;

static int longestline(const char *str);

/* returns 1 if play, 0 if quit */
enum menuitem menu()
{
	static enum menuitem m, n;
	enum key k;
	int e = 0;

	t.tv_sec = 0;
	t.tv_nsec = 1000000000 / 60;

	if (!n)
		for (enum menuitem i = MENUITEM_PLAY; i < MENUITEM_END; i++)
			n = MAX(n, sizeof(items[i]));

	input_finish();

	draw_clear();
	draw_border();
	draw_score();

	while (!e) {
		for (enum menuitem i = 0; i < MENUITEM_END; i++) {
			set_cursor_board(WIDTH - (n / 2),
					(HEIGHT / (2 * MENUITEM_END)) + 2 * i);
			if (i == m)
				printf(SET_UNDERLINE "%s" SET_NORMAL, items[i]);
			else
				printf("%s", items[i]);
		}

		switch ((k = input_get_key())) {
			case KEY_ARROW_UP:
				m = m ? m - 1 : m;
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
				break;
			default:
				input_getchar();
		}

		draw_flush();
	}

	input_init();

	return m;
}

/* TODO: bug, does not always add entry? */
void postgame()
{
	int i, commit;
	char c;
	char name[NAME_LEN];

	/* TODO: maybe not the prettiest. Maybe need some more robust
	 * modal mode? E.g.: input_set_text() and input_set_key()?
	 * `readline` maybe?
	 */
	input_finish();

	commit = 0;
	i = 0;
	while (i < sizeof(name) - 1) {
		name[i] = 0;

		set_cursor_board(2, HEIGHT / 2);
		printf("name: "
			SET_UNDERLINE "%-*s" SET_NORMAL
			, NAME_LEN, name);

		c = getchar();

		/* still want the drawing, so no break */
		if (c == '\n') {
			commit = 1;
			break;
		} else if (c == '\033') {
			break;
		} else if (c == 0x7f) {
			/* delete */
			i--;
		} else if (isalnum(c)) {
			name[i++] = c;
		}

		draw_flush();
	}

	input_init();

	if (commit)
		leaderboard_commit(name);
}

void leaderboard()
{
	FILE *leader;
	struct point tl = {4, 2};
	struct point br = {WIDTH * 2 - 1, HEIGHT - 1};

	leader = leaderboard_open();
	if (!leader)
		leaderboard_default();
	else
		fclose(leader);

	input_finish();
	
	draw_clear();
	draw_border();
	draw_leaderboard(tl, br, -1);
	draw_flush();

	getchar();

	input_init();
}

static const char helptxt[] =
	"press q to go back to the menu or quit.\n\n"
	"move with arrow keys, and select with enter.\n\n"
	"press escape when entering your name to cancel; "
	"your score will not be committed."
	;

void help()
{
	const char title[] = "help";
	char c, word[16];
	int col, row, n, width, newline, i;
	const char *str;

	input_finish();

	draw_clear();
	draw_border();

	width = MIN(WIDTH * 2 - 4, longestline(helptxt));

	set_cursor_board((WIDTH * 2 - sizeof(title)) / 2, 2);
	printf(SET_UNDERLINE "%s" SET_NORMAL, title);

	str = helptxt;
	col = (WIDTH * 2 - width) / 2;
	row = 4;
	n = 0;
	set_cursor_board(col, row);
	do {
		newline = 0;
		/* get word */
		for (i = 0; i < sizeof(word) - 1 && (c = str[i]); i++) {
			if (c == '\n') {
				word[i++] = 0;
				newline = 1;
				break;
			} else if (isspace(c)) {
				word[i] = c;
				word[++i] = 0;
				break;
			}
			word[i] = c;
		}
		word[i] = 0;

		n += i;
		if (!(n - 1 < width)) {
			set_cursor_board(col, ++row);
			n = i;
			printf("%s", word);
		} else if (newline) {
			printf("%s", word);
			n = 0;
			set_cursor_board(col, ++row);
		} else {
			printf("%s", word);
		}

	} while (*(str+=i));

	draw_flush();

	getchar();
	input_init();
}

static int longestline(const char *str)
{
	int i, l;
	char c;
	const char *s;

	s = str;
	l = 0;
	do {
		i = 0;
		while ((c = s[i]) && c != '\n')
			i++;
		l = MAX(l, i);
	} while (*(s+=(++i)));

	return l;
}
