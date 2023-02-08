#include <stdio.h>
#include <stdlib.h>

#define MAX_STATE 256

#define MAX_BUFFER 128


void loop(void) {
	char buf[MAX_BUFFER];

	long mood = (random() % 3);

	size_t len = snprintf(buf, MAX_BUFFER, "you mood is: %s\n", ((mood == 0) ? "fucked" : ((mood == 1) ? "screwed" : "okayish")));
	puts(buf);
}

int main(void) {
/*	unsigned int seed = 66666; */
	unsigned int seed = 12345;
	char state[MAX_STATE];

	initstate(seed, state, MAX_STATE);
	setstate(state);
	
	for (;;) {
		loop();
	}

	return 0;
}

