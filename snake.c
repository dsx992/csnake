#include <stdio.h>

#include "arena.h"
#include "game.h"
#include "input.h"
#include "menu.h"
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

	arena_init(128);

	return 0;

	term_init();
	m = MENUITEM_MENU;

	while (menus[m])
		m = menus[m]();

	term_finish();

	printf("score: %i\n", game_ctx.score);

	arena_finish();
	return 0;
}
