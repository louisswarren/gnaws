#include <stdio.h>
#include <netinet/in.h>
#include <stdlib.h>

#include <unistd.h>
#include <errno.h>
#include <signal.h>

/* Behaviour:
	* EINTR is always retried
	* Unhandled errors exit
	* Handlers with a `continue` are retried
*/
#define trycatch(expr, ...) do { \
		if ((long)(expr) < 0) { \
			switch (errno) { \
			case EINTR: \
				continue; \
			__VA_ARGS__ \
				break; \
			default: \
				perror(#expr); \
				_exit(1); \
				break; \
			} \
		} \
		break; \
	} while(1)

#define try(expr) do { \
		if ((long)(expr) < 0) { \
			switch (errno) { \
			case EINTR: \
				continue; \
			default: \
				perror(#expr); \
				_exit(1); \
				break; \
			} \
		} \
		break; \
	} while(1)

extern const char _binary_response_http_start[];
extern const char _binary_response_http_end[];

static
void
serve(int fd)
{
	const char *p = _binary_response_http_start;
	do {
		ssize_t n;
		try(n = write(fd, p, _binary_response_http_end - p));
		p += n;
	} while (p < _binary_response_http_end);

	shutdown(fd, SHUT_WR);
}

int
main(int argc, const char *argv[])
{
	uint16_t port = 8080;

	if (argc > 1) {
		char *end;
		long port_spec = strtol(argv[1], &end, 0);
		if (!*end && port_spec > 0 && port_spec < 32768) {
			port = (uint16_t)port_spec;
		} else {
			fprintf(stderr, "Invalid port specified\n");
			return 1;
		}
	}

	pid_t serve_proc;

	struct sockaddr_in servaddr = {0}, cli = {0};
	int sockfd, connfd;
	unsigned int len = sizeof(cli);

	//assert((long)SIG_ERR < 0);
	try(signal(SIGCHLD, SIG_IGN));

	try(sockfd = socket(AF_INET, SOCK_STREAM, 0));

	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(port);

	try(bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)));

	while (1) {
		try(listen(sockfd, SOMAXCONN));
		trycatch(connfd = accept(sockfd, (struct sockaddr *)&cli, &len),
			case EMFILE:
			case ENFILE:
			case ENOMEM:
			case EPROTO:
				perror("accept");
		);
		if (connfd < 0) continue;

		trycatch(serve_proc = fork(),
			case EAGAIN:
			case ENOMEM:
				perror("fork");
		);
		if (serve_proc == 0) {
			serve(connfd);
			close(connfd);
			_exit(0);
		}
		/* if serve_prc == -1 or serve_proc > 0 */
		close(connfd);
	}
	__builtin_unreachable();
	close(sockfd);
	return 0;
}
