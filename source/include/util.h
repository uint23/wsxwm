#ifndef UTIL_H
#define UTIL_H

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>

#include <swc.h>
#include <wayland-server.h>

#include "types.h"

void die(int ret, const char* fmt, ...);
struct client* first_float(struct screen* s);
struct client* first_tiled(struct screen* s);
bool is_float(const struct client* c, const struct screen* s);
bool is_tiled(const struct client* c, const struct screen* s);
struct client* last_float(struct screen* s);
struct client* last_tiled(struct screen* s);
void _log(FILE* fd, const char* fmt, ...);
void sig_handler(int s);

#define LENGTH(x) (sizeof(x) / sizeof((x)[0]))

#endif /* UTIL_H */

