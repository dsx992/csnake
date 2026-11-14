#define _POSIX_C_SOURCE 199309L
#include "leaderboard.h"
#include "game.h"
#include <stdio.h>
#include <time.h>

/* leaderboard file */
#define LEADERBOARD "leaderboard"

struct game_ctx;
extern struct game_ctx game_ctx;

static size_t getline(char *buf, size_t n, FILE *s);

/* leaderboard format:
 * 	name	50	unix epoch
 * 	name	30	unix epoch
 * 	name	10	unix epoch
 * 	is sorted by score
 */
int leaderboard_commit(const char *name)
{
	static char buf[128];
	static char new[128];
	int m, bn, nn, s, ok;
	time_t t;
	FILE *leader, *tmp;

	ok = 0;
	tmp = fopen("." LEADERBOARD, "w");
	leader = fopen(LEADERBOARD, "r");

	t = time(NULL);
	nn = (name[0] == 0)
		? snprintf(new, sizeof(new),
				"%s\t%i\t%lu\n", "<empty>", game_ctx.score, t)
		: snprintf(new, sizeof(new),
				"%s\t%i\t%lu\n", name, game_ctx.score, t);

	if (!tmp)
		goto done;

	/* copies and inserts into tmp (sorted) */
	if (leader) {
		while ((bn = getline(buf, sizeof(buf), leader))) {
			m = sscanf(buf, "%*s\t%i", &s);

			/* assert only one match */
			if (m != 1) {
				ok = 0;
				goto done;
			}

			if (s < game_ctx.score)
				break;
			
			fwrite(buf, bn, sizeof(char), tmp);
		}

		fwrite(new, nn, sizeof(char), tmp);

		/* may be a missing score from previous break. */
		if (bn)
			fwrite(buf, bn, sizeof(char), tmp);

		while ((bn = getline(buf, sizeof(buf), leader))) {
			m = sscanf(buf, "%*s\t%i", &s);

			/* assert only one match */
			if (m != 1) {
				ok = 0;
				goto done;
			}

			fwrite(buf, bn, sizeof(char), tmp);
		}
		fclose(leader);
	} else {
		/* if leaderboard does not exist, just write the current
		 * score. */
		fwrite(new, nn, sizeof(char), tmp);
	}

	fclose(tmp);
	
	tmp = fopen("." LEADERBOARD, "r");
	leader = fopen(LEADERBOARD, "w");

	if (tmp && leader) {
		while ((nn = getline(buf, sizeof(buf), tmp)))
			fwrite(buf, nn, sizeof(char), leader);
		ok = 1;
	}
done:
	if (tmp)
		fclose(tmp);
	if (leader)
		fclose(leader);

	remove("." LEADERBOARD);

	return ok;
}

static size_t getline(char *buf, size_t n, FILE *s)
{
	size_t i;
	for(i = 0; i < n; i++) {
		if (!fread(buf + i, sizeof(char), 1, s)) {
			break;
		}
		if (buf[i] == '\n') {
			i++;
			break;
		}
	}
	buf[i] = 0;
	return i;
}
