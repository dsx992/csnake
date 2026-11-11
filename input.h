#ifndef INPUT_
#define INPUT_

enum key {
	KEY_NULL = 0,
	KEY_UP,
	KEY_DOWN,
	KEY_LEFT,
	KEY_RIGHT,
	KEY_Q,
	KEY_ENTER,
};

/* returns (char)NULL if there is no input */
enum key input_get();
/* sets up terminal and key listener thread */
int input_init();
/* cleans up */
void input_finish();

#endif
