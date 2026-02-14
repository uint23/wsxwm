#ifndef UTIL_H
#define UTIL_H

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <wayland-server.h>

#include "extern.h"

/* TODO: dumped the boilerplate for now */
static void quit(void* data, uint32_t time, uint32_t value, uint32_t state)
{
	(void)data;
	(void)time;
	(void)value;
	(void)state;

	wl_display_terminate(wm.dpy);
}

static void new_screen(struct swc_screen* scr)
{
	(void)scr;
}

static void new_window(struct swc_window* win)
{
	(void)win;
}

static void new_device(struct libinput_device* dev)
{
	(void)dev;
}

static void sig_handler(int s)
{
	(void)s;

	if (wm.dpy)
		wl_display_terminate(wm.dpy);
}

/* logging */
static inline void die(int ret, const char* fmt, ...)
{
	va_list ap;

	fprintf(stderr, "wsxwm: ");

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);

	fputc('\n', stderr);
	fflush(stderr);

	exit(ret);
}

static inline void wl_log(FILE* fd, const char* fmt, ...)
{
	va_list ap;

	fprintf(fd, "wsxwm: ");

	va_start(ap, fmt);
	vfprintf(fd, fmt, ap);
	va_end(ap);

	fputc('\n', fd);
	fflush(fd);
}

#endif /* UTIL_H */

