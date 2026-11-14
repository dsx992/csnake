CC := gcc -std=c99
flags := -Wall -Wpedantic -g

.PHONY: clean

default: snake

snake: snake.c rings.h point.h input.o term.o
	$(CC) $(flags) -o snake snake.c input.o term.o

debug: snake.c rings.h input.o
	$(CC) $(flags) -o snake_debug snake.c input.o

input.o: input.c input.h
	$(CC) $(flags) -o input.o -c input.c

term.o: term.c term.h point.h
	$(CC) $(flags) -o term.o -c term.c

clean:
	rm -rf input.o term.o snake snake_debug
