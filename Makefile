CC := c99
flags := -Wall -Wpedantic -g

.PHONY: clean cleaner

default: snake


snake: snake.c game.o menu.o input.o term.o draw.o leaderboard.o arena.o
	$(CC) $(flags) -o snake snake.c game.o menu.o input.o term.o draw.o leaderboard.o arena.o

menu.o: menu.c menu.h draw.o
	$(CC) $(flags) -o menu.o -c menu.c

game.o: game.c game.h
	$(CC) $(flags) -o game.o -c game.c

draw.o: draw.c draw.h term.o
	$(CC) $(flags) -o draw.o -c draw.c

term.o: term.c term.h point.h
	$(CC) $(flags) -o term.o -c term.c

input.o: input.c input.h rings.h
	$(CC) $(flags) -o input.o -c input.c

leaderboard.o: leaderboard.c leaderboard.h
	$(CC) $(flags) -o leaderboard.o -c leaderboard.c

arena.o: arena.c arena.h
	$(CC) $(flags) -o arena.o -c arena.c

clean:
	rm -rf snake *.o

cleaner: clean
	rm -rf leaderboard tags
