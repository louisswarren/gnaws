#include <stdio.h>
#include <stdlib.h>

#include <sys/random.h>

#include <dlfcn.h>
#include <unistd.h>
#include <errno.h>

short
random(void)
{
	short r;
	if (getrandom(&r, sizeof(r), 0) != sizeof(r)) {
		exit(1);
	}
	return r;
}

ssize_t
write(int fd, const void *buf, size_t count)
{
	ssize_t (*f)(int, const void *, size_t);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
	f = dlsym(RTLD_NEXT, "write");
#pragma GCC diagnostic pop

	switch (random() % 6) {
	case 0:
		// Write interrupted before it began
		printf("write: Interrupting\n");
		fflush(stdout);
		errno = EINTR;
		return -1;
		break;
	/* Simulate write failing at various points */
	case 1:
		count = 1;
		/* FALLTHROUGH */
	case 2:
		count = 2;
		/* FALLTHROUGH */
	case 3:
		// Write was interrupted early on
		count = 4;
		/* FALLTHROUGH */
	case 4:
		// Write was interrupted early on
		count = 8;
		/* FALLTHROUGH */
	case 5:
		// Write was interrupted
		count = (random() % count) + 1;
		printf("write: cutting count to %d bytes\n", (int)count);
		fflush(stdout);
	default:
		break;
	}

	return f(fd, buf, count);
}
