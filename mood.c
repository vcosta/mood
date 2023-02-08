#include <stdio.h>
#include <stdlib.h>

#define MAX_STATE 256


void loop(void) {
	long mood = (random() % 3);

	printf("you mood is: %s\n", ((mood == 0) ? "fucked" : ((mood == 1) ? "screwed" : "okayish")));
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

