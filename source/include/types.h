#ifndef TYPES_H
#define TYPES_H

#include <stdbool.h>
#include <stdint.h>

#include <swc.h>
#include <wayland-server.h>

enum {
	MOD1 = SWC_MOD_ALT,
	MOD4 = SWC_MOD_LOGO,
	SHFT = SWC_MOD_SHIFT,
	CTRL = SWC_MOD_CTRL,
};

union arg {
	int            i;
	uint32_t       ui;
	float          f;
	const void*    v;
};

struct bind {
	uint32_t       type;
	uint32_t       mods;
	uint32_t       ksym;
	union arg      arg;
	void           (*fn)(void* data, uint32_t time, uint32_t value, uint32_t state);
};

struct client {
	struct wl_list link;
	struct swc_window* win;
	struct screen* scr;
	bool           mapped;
	bool           floating;
	bool           fullscreen;
	int32_t        x;
	int32_t        y;
	uint32_t       w;
	uint32_t       h;
	uint32_t       ws;
};

struct screen {
	struct wl_list link;
	struct swc_screen* scr;
	int32_t        x;
	int32_t        y;
	uint32_t       w;
	uint32_t       h;
};

struct wm {
	struct wl_display* dpy;
	struct wl_event_loop* ev_loop;

	struct wl_list screens;
	struct wl_list clients;

	struct screen* sel_screen;
	struct client* sel_client;
};

#endif /* TYPES_H */

