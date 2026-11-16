#ifndef MENU_
#define MENU_

enum menuitem {
	MENUITEM_PLAY = 0,
	MENUITEM_SCORE,
	MENUITEM_HELP,
	MENUITEM_QUIT,
	MENUITEM_END,
};


enum menuitem menu();
void postgame();
void leaderboard();
void help();

#endif
