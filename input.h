#ifndef INPUT_
#define INPUT_

#include <stddef.h>

/*
 * the `input` set of functions read characters from stdin into an internal
 * buffer. User can request the next key press in terms of the `enum key`.
 *
 * This file exposes:
 *	`input_get_key()`: returns the next key parsed from the internal buffer.
 * 	`input_init()`: sets up a listener thread that continuously runs `getchar()` and places the result in the internal buffer.
 *	`input_finish()`: kills the listener thread.
 */

enum key {
	/* parser error */
	KEY_INVALID,
	/* unknown symbol */
	KEY_UNKNOWN,
	KEY_CHAR_Q,
	KEY_ARROW_UP,
	KEY_ARROW_DOWN,
	KEY_ARROW_LEFT,
	KEY_ARROW_RIGHT,
	KEY_ENTER,
};

const static struct {
	const char* pattern;
	const enum key key;
} keymap[] = {
	{"\033[A"	, KEY_ARROW_UP},
	{"\033[B"	, KEY_ARROW_DOWN},
	{"\033[C"	, KEY_ARROW_RIGHT},
	{"\033[D"	, KEY_ARROW_LEFT},
	{"\n"		, KEY_ENTER},
	{"q"		, KEY_CHAR_Q},
};

/* returns (char)NULL if there is no input */
enum key input_get_key();
/* OLD: sets up terminal and key listener thread */
int input_init();
int input_raw();
void input_blocking();
/* reads a single char into the buffer */
int input_getchar();

#endif
