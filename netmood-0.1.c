#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#include <sys/epoll.h>


#define MAX_RNG_STATE 256

#define MAX_BUFFER 128

#define MAX_EPOLL_EVENTS 10


void doit(int conn_sock) {
	char buf[MAX_BUFFER];

	long mood = (random() % 3);

	size_t len = snprintf(buf, MAX_BUFFER, "you mood is: %s\n", ((mood == 0) ? "fucked" : ((mood == 1) ? "screwed" : "okayish")));
	send(conn_sock, buf, len, 0);
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
	opt.l_onoff = 1;
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

	for (;;) {
		struct sockaddr_in6 conn_addr;
		socklen_t conn_addrlen;
		int conn_sock;

		conn_sock = accept(sd, (struct sockaddr *) &conn_addr, &conn_addrlen);
		if (conn_sock == -1) {
			perror("accept error: ");
			exit(1);
		}

		doit(conn_sock);
		close(conn_sock);
	}

	return 0;
}

