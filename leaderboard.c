#define _POSIX_C_SOURCE 199309L
#include "leaderboard.h"
#include "game.h"
#include <stdio.h>
#include <time.h>
#include <string.h>

/* leaderboard file */
#define LEADERBOARD "leaderboard"

struct game_ctx;
extern struct game_ctx game_ctx;

FILE *leaderboard_open()
{
	return fopen(LEADERBOARD, "r");
}

static size_t getline(char *buf, size_t n, FILE *s);
static int write_entry(FILE *, const struct leaderboard_entry *);

int leaderboard_entry(FILE *s, struct leaderboard_entry *entry)
{
	char buf[128];
	int m;

	getline(buf, sizeof(buf), s);
	m = sscanf(buf, "%s\t%i\t%lu", entry->name, &entry->score, &entry->unix);
	
	return m == 3;
}

static const struct leaderboard_entry default_leaderboard[] = {
	{.name = "alice", .score = 50},
	{.name = "bob", .score = 30},
	{.name = "charlie", .score = 5},
};

int leaderboard_default()
{
	FILE *leader = fopen(LEADERBOARD, "w");

	if (!leader)
		return 0;

	for (int i = 0; i < LENGTH(default_leaderboard); i++)
		write_entry(leader, &default_leaderboard[i]);
	fclose(leader);

	return 1;
}

/* leaderboard format:
 * 	name	50	unix epoch
 * 	name	30	unix epoch
 * 	name	10	unix epoch
 * 	is sorted by score
 */
int leaderboard_commit(const char *name)
{
	int ok, inserted;
	time_t t;
	FILE *leader, *tmp;
	struct leaderboard_entry le, ge;

	ok = 0;
	inserted = 0;
	tmp = fopen("." LEADERBOARD, "w");
	leader = fopen(LEADERBOARD, "r");
	t = time(NULL);
	
	if (!name)
		goto done;

	if (!tmp)
		goto done;

	if (!leader) {
		leaderboard_default();
	}

	strcpy(ge.name, name);
	ge.score = game_ctx.score;
	ge.unix = t;

	/* insert all entries into tmp */
	while (leaderboard_entry(leader, &le)) {
		if (!inserted && le.score < game_ctx.score)
			inserted = write_entry(tmp, &ge);
		
		write_entry(tmp, &le);
	}
	/* if score is lowest */
	if (!inserted)
		inserted = write_entry(tmp, &ge);


	fclose(leader);
	fclose(tmp);
	
	tmp = fopen("." LEADERBOARD, "r");
	leader = fopen(LEADERBOARD, "w");

	if (!(tmp && leader))
		goto done;

	/* unneeded parsing but easier to read? */
	while (leaderboard_entry(tmp, &le))
		write_entry(leader, &le);
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
	char c;
	size_t i;


	for(i = 0; i < n - 1; i++) {
		c = fgetc(s);
		if (c == EOF || c == '\n')
			break;
		buf[i] = c;
	}

	buf[i] = 0;
	
	return i;
}

static int write_entry(FILE *fd, const struct leaderboard_entry *entry) {
	if (!fd || !entry)
		return 0;

	fprintf(fd, "%s\t%i\t%lu\n",
			entry->name[0] ? entry->name : "<empty>",
			entry->score,
			entry->unix);
	return 1;
}
