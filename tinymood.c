#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(void) {
//	srandom(66666);
//	srandom(12345);
	srandom(time(NULL));

	long mood = (random() % 3);

	printf("you mood is: %s\n", ((mood == 0) ? "fucked" : ((mood == 1) ? "screwed" : "okayish")));
	return 0;
}

