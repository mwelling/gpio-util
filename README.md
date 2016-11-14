# gpio-util
Utility for accessing standard Linux GPIOs from userspace via new character device interface.

```
USAGE:	gpio -n NAME -o OFF [-d DIR] [-v VAL]
	-n, --name      NAME	GPIO device name.
	-o, --offset    OFF	GPIO device offset.
	-d, --direction DIR	Set GPIO direction. { in, out }
	-v, --value     VAL	Set GPIO values in binary.

EXAMPLE: gpio -n gpiochip0 -o0 -o1 -o2 -d out -v 001
```

Note: For legacy interface see sysfs branch of this repository.
