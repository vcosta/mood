#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
/* #include <sys/epoll.h> */
#include <signal.h>


#define MAX_RNG_STATE 256

#define MAX_BUFFER 128

/* #define MAX_EPOLL_EVENTS 10 */

#define MAX(a, b)	((a) > (b) ? (a) : (b))


const char *moods[] = {
	"your mood is: fucked\r\n",
	"your mood is: screwed\r\n",
	"your mood is: okayish\r\n"
};

const char *infos[] = {
	"you are fleeing kinda blue\r\n",
	"you are down in the dumps\r\n",
	"it is raining outside\r\n"
};


static int max_conns, used_conns;
static int *conns;


void defrag(void) {
	int j = -1;
	for (int i = 0; i < max_conns; i++) {
		if (conns[i] == -1)
			j = i;
		else if (j != -1) {
			conns[j] = conns[i];
			j = -1;
		}
	}
	conns = realloc(conns, used_conns * sizeof(*conns));
	max_conns = used_conns;
/*
	fprintf(stderr, "defrag: %d left out of %d\n", used_conns, max_conns);*/
}

bool doit(int conn_sock) {
	char buf[MAX_BUFFER];
	size_t len;
	ssize_t ret;

	long mood;

	mood = (random() % 3);
	len = snprintf(buf, MAX_BUFFER, "%s", moods[mood]);
	ret = send(conn_sock, buf, len, 0);
	if (ret <= 0) return false;

	mood = (random() % 3);
	len = snprintf(buf, MAX_BUFFER, "%s", infos[mood]);
	ret = send(conn_sock, buf, len, 0);
	if (ret <= 0) return false;

	sleep(2);

	return true;
}

int main(void) {
/*	unsigned int seed = 66666; */
	unsigned int seed = 12345;
	char state[MAX_RNG_STATE];

	initstate(seed, state, MAX_RNG_STATE);
	setstate(state);

	signal(SIGPIPE, SIG_IGN);

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

	max_conns = used_conns = 0;
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
		for (int i = 0; i < used_conns; i++) {
			const int conn_sock = conns[i];
			if (conn_sock != -1) {
				FD_SET(conn_sock, &in_set);
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
			conns = realloc(conns, (used_conns+1) * sizeof(*conns));
			conns[used_conns] = conn_sock;
			used_conns++;
			max_conns++;
		}

		for (int i = 0; i < max_conns; i++) {
			const int conn_sock = conns[i];
			if (FD_ISSET(conn_sock, &exc_set)) {
				close(conn_sock);
				conns[i] = -1;
				used_conns--;
				continue;
			}

			if (FD_ISSET(conn_sock, &in_set)) {
				char ibuffer[82];
				recv(conn_sock, ibuffer, sizeof(ibuffer), 0);
				printf("Received: '%s'", ibuffer);
			}

			if (FD_ISSET(conn_sock, &out_set)) {
				if (!doit(conn_sock)) {
					close(conn_sock);
					conns[i] = -1;
					used_conns--;
				}
			}
		}

		defrag();
	}

	return 0;
}

