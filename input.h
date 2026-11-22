#ifndef INPUT_
#define INPUT_

#include <stddef.h>

/*
 * the `input` set of functions read characters from stdin into an internal
 * buffer. User can request the next key press in terms of the `enum key`.
 *
 * This file exposes:
 *	`input_get_key()`: returns the next key parsed from the internal buffer.
 * 	`input_init()`: sets up a listener thread that continuously runs
 * 	`getchar()` and places the result in the internal buffer.
 *	`input_finish()`: kills the listener thread.
 */

/*
 * The input set of functions provide a way to parse characters continously
 * from stdin.
 *
 * `input_setreal()` starts a thread that parses input. This is nessecary to
 * handle control sequences where the only delimiter is the rate of the
 * character stream.
 *
 * `input_get()` gets a key when `input_setreal()` is set.
 *
 * `input_getkey()` gets a character when `input_setnoreal()` is set. It is
 * then required to provide a keymap. If canonical is set or the stream is not
 * empty at invocation then parsing may fail (see `input_setreal()`).
 */

/*
 * The underlying parser should take any keymap being a char * to enum map.
 * Escape sequences should automatically result in a debounce.
 */
struct keymap {
	const char *pattern;
	int bind;
};

int input_get();
int input_getkey(const struct keymap *keymap, size_t size);
int input_setreal(const struct keymap *keymap, size_t size);
int input_setnoreal();
int input_init();

#endif
