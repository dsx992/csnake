#ifndef TERM
#define TERM

struct term_ctx {
	int w, h;
};

int term_init();
int term_finish();
int term_isblocking();
void term_setblocking();
void term_setnotblocking();

#endif
