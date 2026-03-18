#include "termios.h"

int tcsetattr(int __fd, int __optional_actions, const struct termios *__termios_p) {
    return -1;
}

int tcgetattr(int __fd, struct termios *__termios_p) {
    return -1;
}
