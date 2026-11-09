CC := gcc -std=c99
flags := -Wall -Wpedantic

.PHONY: clean

snake: snake.c input.o
	$(CC) $(flags) -o snake snake.c input.o

input.o: input.c
	$(CC) $(flags) -o input.o -c input.c

clean:
	rm -rf input.o snake
