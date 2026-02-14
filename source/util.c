#include <stdlib.h>

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

