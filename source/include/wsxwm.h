#ifndef WSXWM_H
#define WSXWM_H

#include <stdint.h>

#include "types.h"

void quit(void* data, uint32_t time, uint32_t value, uint32_t state);

extern struct wm wm;

#endif /* WSXWM_H */
