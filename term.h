#ifndef TERM
#define TERM

struct term_ctx {
	int w, h;
};

int term_init();
int term_finish();

#endif
