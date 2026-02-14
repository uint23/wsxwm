#ifndef WSXWM_H
#define WSXWM_H

#include <stdint.h>

#include "types.h"

void new_screen(struct swc_screen* scr);
void new_window(struct swc_window* win);
void new_device(struct libinput_device* dev);
void quit(void* data, uint32_t time, uint32_t value, uint32_t state);

extern struct wm wm;

#endif /* WSXWM_H */
