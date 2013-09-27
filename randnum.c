/*
 * randnum.c -- generate (good) randum numbers.
 *
 * Copyright (C) 2001,2002 by Theodore Ts'o
 * 
 * This file may be distributed under the terms of the GNU Public
 * License.
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#include "pwgen.h"

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>

static int get_random_fd(void);

/* Platform-specific random devices can be ifdef'd here */
#define RANDOM_DEVICE "/dev/urandom"

/* Borrowed/adapted from e2fsprogs's UUID generation code */
static int get_random_fd()
{
	static int	fd = -2;

	if (fd == -2) {
		fd = open(RANDOM_DEVICE, O_RDONLY);
		if (fd == -1) {
			fprintf(stderr, "Unable to open %s: %s\n", RANDOM_DEVICE, strerror(errno));
			abort(); /* Something is seriously wrong with this system!! */
		}
	}
	return fd;
}

/*
 * Generate a random number n, where 0 <= n < max_num, using
 * /dev/urandom if possible.
 */
int pw_random_number(max_num)
	int max_num;
{
	int i, fd = get_random_fd();
	int nbytes=4;
	unsigned int rand_num;
	char *cp = (char *) &rand_num;

	int bits = 0, b = max_num;
	while(b > 0) {
		bits += 1;
		b >>= 1;
	}

	assert(fd >= 0);

	while (nbytes > 0) {
		i = read(fd, cp, nbytes);
		if ((i < 0) && ((errno == EINTR) || (errno == EAGAIN)))
			continue;
		if (i <= 0) {
			fprintf(stderr, "Random device is broken: %s\n", strerror(errno));
			abort();
		}
		nbytes -= i;
		cp += i;
	}

	assert(nbytes == 0);

	rand_num &= ((1<<bits)-1);
	if(rand_num >= max_num)
		return pw_random_number(max_num);
	return rand_num;
}
