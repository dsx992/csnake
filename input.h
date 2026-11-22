#ifndef INPUT_
#define INPUT_

#include <stddef.h>

struct keymap {
	const char *string;
	const int bind;
};

void input_setblocking(int b);
int input_getkey();
void input_setkeymap(const struct keymap *km, size_t ks);
void input_flush();
void input_init();
void input_finish();

#endif
