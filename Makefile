CC := gcc -std=c99
flags := -Wall -Wpedantic

.PHONY: clean

default: snake

snake: snake.c rings.h input.o
	$(CC) $(flags) -o snake snake.c input.o

debug: snake.c rings.h input.o
	$(CC) $(flags) -g -o snake_debug snake.c input.o

input.o: input.c
	$(CC) $(flags) -g -o input.o -c input.c

clean:
	rm -rf input.o snake
