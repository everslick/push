#include <termios.h>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>

#include "linux.h"

static struct termios initial_settings;

uint8_t linux_init(void) {
	struct termios new_settings;
                                                                                
	if (tcgetattr(0, &initial_settings) < 0) return (0);

	new_settings = initial_settings;
	new_settings.c_lflag &= ~(ICANON | ECHO | ISIG); // Ctrl-C is disabled
	new_settings.c_cc[VMIN] = 1;
	new_settings.c_cc[VTIME] = 1;

	tcsetattr(0, TCSANOW, &new_settings);

  return (1);
}

void linux_fini(void) {
	tcsetattr(0, TCSANOW, &initial_settings);
}
