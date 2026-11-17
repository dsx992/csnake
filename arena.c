#include "arena.h"

#include <stdlib.h>

/*
 * In the spirit of arena allocators, there exists only one.
 *
 * note:
 * 	The current intended use is for the ring type so that they can be of
 * 	dynamic type without needing to free them.
 */

struct arena {
	void *addr;
	void *end; /* points to the last _unallocated_ address */
	void *head;
};

volatile struct arena arena;

int arena_init(size_t size)
{
	void *ptr;

	ptr = malloc(size);

	if (ptr == NULL)
		return 0;
	
	arena = (struct arena){
		.addr = ptr,
		.end = (char *)ptr + size,
		.head = ptr
	};

	return 1;
}

void arena_finish()
{
	free(arena.addr);
}

void *arena_malloc(size_t size)
{
	void *old, *new;

	old = arena.head;
	new = (char *)old + size;

	if (new > arena.end)
		return NULL;

	arena.head = new;
	
	return old;
}
