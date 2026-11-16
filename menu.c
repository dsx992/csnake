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

enum menuitem menu()
{
	static enum menuitem m, mm, n;
	enum key k;
	int r;
	int e = 0;

	t.tv_sec = 0;
	t.tv_nsec = 1000000000 / 60;

	if (!n)
		for (enum menuitem i = MENUITEM_PLAY; i < MENUITEM_END; i++)
			if (items[i])
				n = MAX(n, sizeof(items[i]));

	m = MAX(1, m);

	draw_clear();
	draw_border();
	draw_score();

	/* TODO: this should be a draw_* function */
	while (!e) {
		r = 0;
		for (enum menuitem i = 0; i < MENUITEM_END; i++) {
			if (items[i]) {
				set_cursor_board(WIDTH - (n / 2),
						4 + 2 * r);
				if (i == m)
					printf(SET_UNDERLINE "%s" SET_NORMAL,
							items[i]);
				else
					printf("%s", items[i]);
				r++;
			}
		}

		draw_flush();

		switch ((k = input_get_key())) {
			case KEY_ARROW_UP:
				mm = m;
				while (--mm)
					if (items[mm])
						break;
				if (items[mm])
					m = mm;
				break;
			case KEY_ARROW_DOWN:
				mm = m;
				while ((++mm) < MENUITEM_END)
					if (items[mm])
						break;
				if (items[mm])
					m = mm;
				break;
			case KEY_CHAR_Q:
				return MENUITEM_QUIT;
			case KEY_ENTER:
				e = 1;
				break;
			default:
				input_getchar();
		}
	}

	return m;
}

enum menuitem postgame()
{
	int i, commit;
	char c;
	/* remember name between games */
	static char name[NAME_LEN];
	
	draw_clear();
	draw_border();
	draw_snake();
	draw_score();

	commit = 0;
	i = strlen(name);
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

	if (commit)
		leaderboard_commit(name);
	return MENUITEM_MENU;
}

enum menuitem leaderboard()
{
	FILE *leader;
	struct point tl = {4, 2};
	struct point br = {WIDTH * 2 - 1, HEIGHT - 1};

	leader = leaderboard_open();
	if (!leader)
		leaderboard_default();
	else
		fclose(leader);

	draw_clear();
	draw_border();
	draw_leaderboard(tl, br, -1);
	draw_flush();

	getchar();

	return MENUITEM_MENU;
}

static const char helptxt[] =
	"press q to go back to the menu or quit.\n\n"
	"move with arrow keys, and select with enter.\n\n"
	"press escape when entering your name to cancel; "
	"your score will not be committed."
	;

enum menuitem help()
{
	const char title[] = "help";
	char c, word[16];
	int col, row, n, width, newline, i;
	const char *str;

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

	return MENUITEM_MENU;
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
