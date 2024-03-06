#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <signal.h>

extern const char _binary_response_http_start[], _binary_response_http_end[];

#define die(...) do { \
		fprintf(stderr, __VA_ARGS__); \
		fprintf(stderr, "\n"); \
		exit(1); \
	} while(0)
#define commit(MSG, EXPR) do { \
		if ((EXPR) < 0) { \
			perror(MSG); \
			return 1; \
		} \
	} while(0)

static
void
handle(int fd)
{
	printf("Serving\n");

	const char *p = _binary_response_http_start;
	do {
		ssize_t n = write(fd, p, _binary_response_http_end - p);
		if (n >= 0) {
			p += n;
		} else if (errno != EINTR) {
			break;
		}
	} while (p < _binary_response_http_end);

	close(fd);
}

int
main(void)
{
	pid_t handle_proc;

	struct sockaddr_in servaddr = {0}, cli = {0};
	int sockfd, connfd;
	unsigned int len = sizeof(cli);

	signal(SIGCHLD, SIG_IGN);

	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	if (sockfd < 0)
		die("Failed to create socket");

	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(8080);

	commit("bind", bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)));

	while (1) {
		commit("listen", listen(sockfd, SOMAXCONN));
		printf("Waiting ...\n");
		commit("accept", connfd = accept(sockfd, (struct sockaddr *)&cli, &len));
		commit("fork", handle_proc = fork());

		if (!handle_proc) {
			handle(connfd);
			_Exit(0);
		} else {
			close(connfd);
		}
	}
	close(sockfd);
}
