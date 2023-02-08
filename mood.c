#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <netinet/in.h>

#define MAX_RNG_STATE 256

#define MAX_BUFFER 128


void loop(void) {
	char buf[MAX_BUFFER];

	long mood = (random() % 3);

	size_t len = snprintf(buf, MAX_BUFFER, "you mood is: %s\n", ((mood == 0) ? "fucked" : ((mood == 1) ? "screwed" : "okayish")));
	fputs(buf, stdout);
}

int main(void) {
/*	unsigned int seed = 66666; */
	unsigned int seed = 12345;
	char state[MAX_RNG_STATE];

	initstate(seed, state, MAX_RNG_STATE);
	setstate(state);

	int sd;
	sd = socket(AF_INET6, SOCK_STREAM, 0);	/* PF_INET? */
	if (sd == -1) {
		perror("socket error: ");
		exit(1);
	}

	struct linger opt;
	opt.l_onoff = 0;
	opt.l_linger = 0;
	setsockopt(sd, SOL_SOCKET, SO_LINGER, &opt, sizeof(opt));

	struct sockaddr_in6 addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin6_family = AF_INET6;
	addr.sin6_port = htons(4004);
	addr.sin6_addr = in6addr_any;

	if (bind(sd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
		perror("bind error: ");
		exit(1);
	}

	listen(sd, 10);

	/* epoll(); - Linux only */
	
	for (;;) {
		loop();
	}

	return 0;
}

