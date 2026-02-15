#ifndef WSXWM_H
#define WSXWM_H

#include <stdint.h>

#include "types.h"

extern void focus_next(void* data, uint32_t time, uint32_t value, uint32_t state);
extern void focus_prev(void* data, uint32_t time, uint32_t value, uint32_t state);
extern void new_screen(struct swc_screen* scr);
extern void new_window(struct swc_window* win);
extern void new_device(struct libinput_device* dev);
extern void quit(void* data, uint32_t time, uint32_t value, uint32_t state);
extern void spawn(void* data, uint32_t time, uint32_t value, uint32_t state);

extern struct wm wm;

#endif /* WSXWM_H */
