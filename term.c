#include "term.h"
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include "point.h"
#include <signal.h>

volatile struct term_ctx term_ctx;

static struct termios original_term;
static struct termios current_term;
static int blocking;

int term_isblocking()
{
	return blocking;
}

void term_setwaitkey(unsigned char num)
{
	current_term.c_cc[VTIME] = 1;
	current_term.c_cc[VMIN] = num;

	tcsetattr(STDIN_FILENO, TCSANOW, &current_term);
}

void term_setnowaitkey()
{
	current_term.c_cc[VTIME] = 0;
	current_term.c_cc[VMIN] = 1;

	tcsetattr(STDIN_FILENO, TCSANOW, &current_term);
}

void term_setnotblocking()
{
	current_term.c_cc[VTIME] = 1;
	current_term.c_cc[VMIN] = 0;

	tcsetattr(STDIN_FILENO, TCSANOW, &current_term);
	blocking = 0;
}

void term_setblocking()
{
	current_term.c_cc[VTIME] = 0;
	current_term.c_cc[VMIN] = 1;

	tcsetattr(STDIN_FILENO, TCSANOW, &current_term);
	blocking = 1;

}

// When the window size changes, a SIGWINCH signal is sent to the foreground process group.
static void term_get_size()
{
	static struct winsize ws;
	ioctl(STDIN_FILENO, TIOCGWINSZ, &ws);
	term_ctx.w = ws.ws_col;
	term_ctx.h = ws.ws_row;
}

int term_init()
{
	tcgetattr(STDIN_FILENO, &original_term);

	current_term = original_term;
	current_term.c_lflag &= ~(ECHO | ICANON);

	tcsetattr(STDIN_FILENO, TCSAFLUSH, &current_term);
	/* CSI ? 1049 h
	 * CSI ? 1049 l
	 */
	/* alternate buffer */
	printf("\033[?1049h");
	/* hides cursor. ?25h to show */
	printf("\033[?25l");

	blocking = 1;

	term_get_size();
	/* TODO: signal */

	return 1;
}

int term_finish()
{
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_term);
	printf("\033[?1049l");
	/* hides cursor. ?25l to show */
	printf("\033[?25h");

	return 1;
}
