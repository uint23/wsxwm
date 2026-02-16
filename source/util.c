#include <stdlib.h>

#include <swc.h>

#include "util.h"
#include "wsxwm.h"

void die(int ret, const char* fmt, ...)
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

struct client* first_float(struct screen* s)
{
	struct client* c;

	wl_list_for_each(c, &wm.floating, float_link) {
		if (is_float(c, s))
			return c;
	}

	return NULL;
}

struct client* first_tiled(struct screen* s)
{
	struct client* c;

	wl_list_for_each(c, &wm.tiled, tiled_link) {
		if (is_tiled(c, s))
			return c;
	}

	return NULL;
}

bool is_float(const struct client* c, const struct screen* s)
{
	return c && c->scr == s && c->floating;
}

bool is_tiled(const struct client* c, const struct screen* s)
{
	return c && c->scr == s && !c->floating;
}

struct client* last_float(struct screen* s)
{
	struct client* c;

	wl_list_for_each_reverse(c, &wm.floating, float_link) {
		if (is_float(c, s))
			return c;
	}

	return NULL;
}

struct client* last_tiled(struct screen* s)
{
	struct client* c;

	wl_list_for_each_reverse(c, &wm.tiled, tiled_link) {
		if (is_tiled(c, s))
			return c;
	}

	return NULL;
}

void _log(FILE* fd, const char* fmt, ...)
{
	va_list ap;

	fprintf(fd, "wsxwm: ");

	va_start(ap, fmt);
	vfprintf(fd, fmt, ap);
	va_end(ap);

	fputc('\n', fd);
	fflush(fd);
}

void sig_handler(int s)
{
	(void)s;

	if (wm.dpy)
		wl_display_terminate(wm.dpy);
}

