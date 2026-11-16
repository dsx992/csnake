#ifndef ARENA_
#define ARENA_

#include <stddef.h>

struct arena;

int arena_init();
void arena_finish();
void *arena_malloc(size_t size);

#endif
