#include <stdio.h>

#include "game.h"
#include "menu.h"
#include "input.h"
#include "term.h"

struct game_ctx;
extern struct game_ctx game_ctx;

int main()
{
	int quit = 0;

	input_init();
	term_init();

	while (!game_ctx.quit && !quit) {
		switch (menu()) {
			case MENUITEM_PLAY:
				game();
				postgame();
				break;
			case MENUITEM_SCORE:
				scoreboard();
				break;
			case MENUITEM_QUIT:
			case MENUITEM_END:
				quit = 1;
		}
	}

	input_finish();
	term_finish();

	printf("score: %i\n", game_ctx.score);

	return 0;
}
