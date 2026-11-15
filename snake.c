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

	term_init();
	input_init();

	while (!quit) {
		switch (menu()) {
			case MENUITEM_PLAY:
				game();
				/* go to menu */
				if (game_ctx.quit)
					break;
				postgame();
				break;
			case MENUITEM_SCORE:
				leaderboard();
				break;
			case MENUITEM_QUIT:
			case MENUITEM_END:
				quit = 1;
		}
	}

	term_finish();
	input_finish();

	printf("score: %i\n", game_ctx.score);

	return 0;
}
