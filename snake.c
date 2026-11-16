#include <stdio.h>

#include "game.h"
#include "menu.h"
#include "input.h"
#include "term.h"

#define MAX(X, Y) ((X) > (Y) ? (X) : (Y))

struct game_ctx;
extern struct game_ctx game_ctx;

static enum menuitem (*menus[MENUITEM_END])() = {
	[MENUITEM_MENU] = &menu,
	[MENUITEM_PLAY] = &game,
	[MENUITEM_POST] = &postgame,
	[MENUITEM_SCORE] = &leaderboard,
	[MENUITEM_HELP] = &help,
};

int main()
{
	enum menuitem m;

	term_init();
	m = MENUITEM_MENU;

	while (menus[m])
		m = menus[m]();

	term_finish();

	printf("score: %i\n", game_ctx.score);

	return 0;
}
