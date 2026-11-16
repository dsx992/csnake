#ifndef MENU_
#define MENU_

enum menuitem {
	MENUITEM_MENU,
	MENUITEM_PLAY,
	MENUITEM_POST,
	MENUITEM_SCORE,
	MENUITEM_HELP,
	MENUITEM_QUIT,
	MENUITEM_END,
};


enum menuitem menu();
enum menuitem postgame();
enum menuitem leaderboard();
enum menuitem help();

#endif
