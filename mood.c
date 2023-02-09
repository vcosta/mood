#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
/* #include <sys/epoll.h> */


#define MAX_RNG_STATE 256

#define MAX_BUFFER 128

/* #define MAX_EPOLL_EVENTS 10 */

#define MAX(a, b)	((a) > (b) ? (a) : (b))


const char *moods[] = {
	"your mood is: fucked\n",
	"your mood is: screwed\n",
	"your mood is: okayish\n"
};

const char *infos[] = {
	"you are fleeing kinda blue\n",
	"you are down in the dumps\n",
	"it is raining outside\n"
};


static int max_conns;
static int *conns;

void doit(int conn_sock) {
	char buf[MAX_BUFFER];
	size_t len;

	long mood;

	mood = (random() % 3);
	len = snprintf(buf, MAX_BUFFER, "%s", moods[mood]);
	send(conn_sock, buf, len, 0);

	mood = (random() % 3);
	len = snprintf(buf, MAX_BUFFER, "%s", infos[mood]);
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

	max_conns = 0;
	conns = NULL;

	for (;;) {
		fd_set in_set, out_set, exc_set;
		int max_desc;

		FD_ZERO(&in_set);
		FD_ZERO(&out_set);
		FD_ZERO(&exc_set);

		FD_SET(sd, &in_set);
		FD_SET(sd, &exc_set);

		max_desc = sd;
		for (int i = 0; i < max_conns; i++) {
			const int conn_sock = conns[i];
			if (conn_sock != -1) {
				FD_SET(conn_sock, &out_set);
				FD_SET(conn_sock, &exc_set);
				max_desc = MAX(max_desc, conn_sock);
			}
		}

		select(max_desc+1, &in_set, &out_set, &exc_set, NULL);

		if (FD_ISSET(sd, &in_set)) {
			struct sockaddr_in6 conn_addr;
			socklen_t conn_addrlen;
			int conn_sock;

			conn_sock = accept(sd, (struct sockaddr *) &conn_addr, &conn_addrlen);
			if (conn_sock == -1) {
				perror("accept error: ");
				exit(1);
			}
			conns = realloc(conns, (max_conns+1) * sizeof(*conns));
			conns[max_conns] = conn_sock;
			max_conns++;
		}

		for (int i = 0; i < max_conns; i++) {
			const int conn_sock = conns[i];
			if (FD_ISSET(conn_sock, &exc_set)) {
				close(conn_sock);
				max_conns--;
				conns[i] = -1;
			}

			if (FD_ISSET(conn_sock, &out_set)) {
				doit(conn_sock);
				close(conn_sock);
				max_conns--;
				conns[i] = -1;
			}
		}

		/* DEFRAG conns */
	}

	return 0;
}

