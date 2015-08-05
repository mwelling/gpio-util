/*
 * gpio.c
 *
 * Linux GPIO userspace utility
 *
 * Copyright (C) 2015 QWERTY Embedded Design
 * Michael Welling <mwelling@ieee.org>
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

int gpio_exported(char * gpio_number)
{
	char gpio_dir[256];
	struct stat s;

	sprintf(gpio_dir, "/sys/class/gpio/gpio%s", gpio_number);
	return (stat(gpio_dir, &s) == 0);
}

int export_gpio(char * gpio_number)
{
	int fd;
	int ret;	

	if (gpio_exported(gpio_number))
		return 0;

	fd = open("/sys/class/gpio/export", O_WRONLY);
	if (fd < 0)
		return fd;

	ret = write(fd, gpio_number, strlen(gpio_number));

	close(fd);
	return ret;
}

int set_gpio_direction(char * gpio_number, char * direction)
{
	int fd;
	int ret;
	char dir_file[256];

	sprintf(dir_file, "/sys/class/gpio/gpio%s/direction", gpio_number);

	fd = open(dir_file, O_RDWR);
	if (fd < 0)
		return fd;

	ret = write(fd, direction, strlen(direction));

	close(fd);
	return ret;
}

int get_gpio_direction(char * gpio_number)
{
	int fd;
	int ret;
	char dir_file[256];
	char direction[4];

	sprintf(dir_file, "/sys/class/gpio/gpio%s/direction", gpio_number);

	fd = open(dir_file, O_RDWR);
	if (fd < 0)
		return fd;

	ret = read(fd, direction, 4);
	if(ret > 0) {
		direction[ret-1] = '\0';
		printf("gpio direction = %s\n", direction);
	}

	close(fd);
	return ret;
}

int set_gpio_value(char * gpio_number, char * value)
{
	int fd;
	int ret;
	char val_file[256];

	sprintf(val_file, "/sys/class/gpio/gpio%s/value", gpio_number);

	fd = open(val_file, O_RDWR);
	if (fd < 0)
		return fd;

	ret = write(fd, value, strlen(value));

	close(fd);
	return ret;
}

int get_gpio_value(char * gpio_number)
{
	int fd;
	int ret;
	char val_file[256];
	char value[2];

	sprintf(val_file, "/sys/class/gpio/gpio%s/value", gpio_number);

	fd = open(val_file, O_RDWR);
	if (fd < 0)
		return fd;

	ret = read(fd, value, 2);
	if(ret > 0) {
		value[ret-1] = '\0';
		printf("gpio value = %s\n", value);
	}

	close(fd);
	return ret;
}

void print_usage()
{
	printf("USAGE:\tgpio -n NUM [-d DIR] [-v VAL]\n");
	printf("\t-n, --number    NUM\tGPIO number NUM.\n");
	printf("\t-d, --direction DIR\tSet GPIO direction DIR. { in, out }\n");
	printf("\t-v, --value     VAL\tSet GPIO value VAL. { 0, 1 }\n");
}

int main(int argc, char **argv)
{
	int fd;
	char *gpio_number = NULL;
	char *gpio_value = NULL;
	char *gpio_direction = NULL;
	int option_index = 0;
	int c;

	static struct option long_options[]=
	{
		{ "value", required_argument, 0, 'v' },
		{ "number", required_argument, 0, 'n' },
		{ "direction", required_argument, 0, 'd' },
		{ 0, 0, 0, 0 },
	};

	while((c = getopt_long (argc, argv, "v:n:d:", long_options, &option_index)) != -1)
	{
		switch(c)
		{
		case 'n':
			gpio_number = optarg;
			break;
		case 'v':
			gpio_value = optarg;
			break;
		case 'd':
			gpio_direction = optarg;
			break;
		case '?':
			print_usage();
			return 0;
			break;
		}
	}

	if (gpio_number == NULL) {
		print_usage();
		return 1;
	}

	if (export_gpio(gpio_number) < 0) {
		fprintf(stderr, "Cannot export gpio: %s\n", strerror(errno));
		return 1;
	}

	if (gpio_direction) {
		if (set_gpio_direction(gpio_number, gpio_direction) < 0) {
			fprintf(stderr, "Cannot set gpio direction: %s\n", strerror(errno));
			return 1;
		}
	}
	else {
		if (get_gpio_direction(gpio_number) < 0) {
			fprintf(stderr, "Cannot get gpio direction: %s\n", strerror(errno));
			return 1;
		}
	}

	if (gpio_value) {
		if (set_gpio_value(gpio_number, gpio_value) < 0) {
			fprintf(stderr, "Cannot set gpio value: %s\n", strerror(errno));
			return 1;
		}
	}
	else {
		if (get_gpio_value(gpio_number) < 0) {
			fprintf(stderr, "Cannot set gpio value: %s\n", strerror(errno));
			return 1;
		}
	}

	return 0;
}
