/*
 * gpio.c
 *
 * Linux GPIO userspace utility (character interface)
 *
 * Copyright (C) 2016 QWERTY Embedded Design
 * Michael Welling <mwelling@ieee.org>
 *
 * Based on gpio-hammer.c
 * Copyright (C) 2016 Linus Walleij
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <getopt.h>
#include <sys/ioctl.h>
#include <linux/gpio.h>

void print_usage()
{
	printf("USAGE:\tgpio -n NAME -o OFF [-d DIR] [-v VAL]\n");
	printf("\t-n, --name      NAME\tGPIO device name.\n");
	printf("\t-o, --offset    OFF\tGPIO device offset.\n");
	printf("\t-d, --direction DIR\tSet GPIO direction. { in, out }\n");
	printf("\t-v, --value     VAL\tSet GPIO values in binary.\n");
	printf("\nEXAMPLE: gpio -n gpiochip0 -o0 -o1 -o2 -d out -v 001\n");
}

int main(int argc, char **argv)
{
	int fd;
	char *gpio_name = NULL;
	unsigned int lines[GPIOHANDLES_MAX];
	char *value = NULL;
	char *direction = NULL;
	int nlines = 0;
	char *chrdev_name;
	struct gpiohandle_request req;
	struct gpiohandle_data data;
	int ret;
	int opt_i = 0;
	int c;
	int i;

	static struct option long_opts[]=
	{
		{ "direction", required_argument, 0, 'd' },
		{ "name", required_argument, 0, 'n' },
		{ "offset", required_argument, 0, 'o' },
		{ "value", required_argument, 0, 'v' },
		{ 0, 0, 0, 0 },
	};

	while((c = getopt_long(argc, argv, "d:n:o:v:",
			       long_opts, &opt_i)) != -1)
	{
		switch(c)
		{
		case 'd':
			direction = optarg;
			break;
		case 'n':
			gpio_name = optarg;
			break;
		case 'o':
			lines[nlines] = strtoul(optarg, NULL, 10);
			nlines++;
			break;
		case 'v':
			value = optarg;
			break;
		case '?':
			print_usage();
			return 0;
			break;
		}
	}

	if (!gpio_name || !nlines) {
		print_usage();
		return -EINVAL;
	}

	ret = asprintf(&chrdev_name, "/dev/%s", gpio_name);
	if (ret < 0) {
		fprintf(stderr, "Out of memory\n", chrdev_name);
		return -ENOMEM;
	}

	fd = open(chrdev_name, 0);
	if (fd < 0) {
		ret = -errno;
		fprintf(stderr, "Failed to open %s\n", chrdev_name);
		goto exit_free_error;
	}

	for (i = 0; i < nlines; i++)
		req.lineoffsets[i] = lines[i];

	if (direction) {
		if (strcmp(direction, "out") == 0)
			req.flags = GPIOHANDLE_REQUEST_OUTPUT;
		else if (strcmp(direction, "in") == 0)
			req.flags = GPIOHANDLE_REQUEST_INPUT;
		else {
			fprintf(stderr, "Invalid direction %s\n", direction);
			ret = -EINVAL;
			goto exit_close_error;
		}
	}
	else {
		req.flags = GPIOHANDLE_REQUEST_INPUT;
	}

	strcpy(req.consumer_label, "gpio-util");
	req.lines = nlines;

	ret = ioctl(fd, GPIO_GET_LINEHANDLE_IOCTL, &req);
	if (ret == -1) {
		ret = -errno;
		fprintf(stderr, "Failed to issue GET LINEHANDLE "
			"IOCTL (%d)\n",
			ret);
		goto exit_close_error;
	}

	if (value && (req.flags == GPIOHANDLE_REQUEST_OUTPUT)) {
		for(i = 0; i < nlines; i++)
			data.values[i] = value[i % strlen(value)] - '0';

		ret = ioctl(req.fd, GPIOHANDLE_SET_LINE_VALUES_IOCTL, &data);
		if (ret == -1) {
			ret = -errno;
			fprintf(stderr, "Failed to issue GPIOHANDLE SET LINE "
				"VALUES IOCTL (%d)\n",
				ret);
			goto exit_close_error;
		}
	}

	ret = ioctl(req.fd, GPIOHANDLE_GET_LINE_VALUES_IOCTL, &data);
	if (ret == -1) {
		ret = -errno;
		fprintf(stderr, "Failed to issue GPIOHANDLE GET LINE "
			"VALUES IOCTL (%d)\n",
			ret);
		goto exit_close_error;
	}

	for (i = 0; i < nlines; i++) {
		printf("%s[%d] = %d\n", gpio_name, lines[i], data.values[i]);
	}

exit_close_error:
	if (close(fd) == -1)
		perror("Failed to close GPIO character device file");
exit_free_error:
	free(chrdev_name);

	return ret;
}
