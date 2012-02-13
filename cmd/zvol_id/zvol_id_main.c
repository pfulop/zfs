/*
 * CDDL HEADER START
 *
 * The contents of this file are subject to the terms of the
 * Common Development and Distribution License (the "License").
 * You may not use this file except in compliance with the License.
 *
 * You can obtain a copy of the license at usr/src/OPENSOLARIS.LICENSE
 * or http://www.opensolaris.org/os/licensing.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 *
 * When distributing Covered Code, include this CDDL HEADER in each
 * file and include the License file at usr/src/OPENSOLARIS.LICENSE.
 * If applicable, add the following below this CDDL HEADER, with the
 * fields enclosed by brackets "[]" replaced with your own identifying
 * information: Portions Copyright [yyyy] [name of copyright owner]
 *
 * CDDL HEADER END
 */
/*
 * Copyright (c) 2011, Fajar A. Nugraha.  All rights reserved.
 * Use is subject to license terms.
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <linux/ioctl.h>
#include <libzfs.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/fs/zfs.h>

static int
zi_open_device(char *device, int *fd)
{
	*fd = open(device, O_RDONLY);
	if (*fd < 0)
		return (errno);

	return (0);
}

static void
zi_close_device(int fd)
{
	(void) close(fd);
}

static int
zi_get_name(int fd, char **name)
{
	char ioctl_name[MAXNAMELEN];
	struct stat64 statbuf;
	int partition;
	int error;

	error = ioctl(fd, BLKZNAME, ioctl_name);
	if (error)
		return (errno);

	error = fstat64(fd, &statbuf);
	if (error)
		return (errno);

	partition = minor(statbuf.st_rdev) % ZVOL_MINORS;
	if (partition)
		error = asprintf(name, "%s-part%d", ioctl_name, partition);
	else
		error = asprintf(name, "%s", ioctl_name);

	if (error == -1)
		return (ENOMEM);

	return (0);
}

static void
zi_put_name(char *name)
{
	free(name);
}

int
main(int argc, char **argv)
{
	char *name;
	int fd;
	int error;

	if (argc < 2) {
		printf("Usage: %s /dev/zdN\n", argv[0]);
		return (EINVAL);
	}

	error = zi_open_device(argv[1], &fd);
	if (error) {
		fprintf(stderr, "Unable to open device %s: %d\n",
		    argv[1], error);
		return (error);
	}

	error = zi_get_name(fd, &name);
	if (error)
		fprintf(stderr, "Unable to get zvol name for %s: %d\n",
		    argv[1], error);
	else
		fprintf(stdout, "%s\n", name);

	zi_put_name(name);
	zi_close_device(fd);

	return (error);
}
