#ifndef INPUT_
#define INPUT_

/* returns (char)NULL if there is no input */
char input_get();
/* sets up terminal and key listener thread */
int input_init();
/* cleans up */
void input_finish();

#endif
