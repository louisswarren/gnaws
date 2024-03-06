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

int
just_(const char *funcname, int r)
{
	if (r < 0) {
		perror(funcname);
		exit(1);
	}
	return r;
}
#define just(FUNC, ...) just_(#FUNC, FUNC(__VA_ARGS__));

extern const char _binary_response_http_start[];
extern const char _binary_response_http_end[];

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

	sockfd = just(socket, AF_INET, SOCK_STREAM, 0);

	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(8080);

	just(bind, sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));

	while (1) {
		just(listen, sockfd, SOMAXCONN);
		printf("Waiting ...\n");
		connfd = just(accept, sockfd, (struct sockaddr *)&cli, &len);
		handle_proc = just(fork, );

		if (!handle_proc) {
			handle(connfd);
			_Exit(0);
		} else {
			close(connfd);
		}
	}
	close(sockfd);
}
