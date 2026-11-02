#ifndef COMMON_H
#define COMMON_H

#include <stdbool.h>
#include <stdint.h>

#include <swc.h>
#include <wayland-server.h>

enum {
	BTN_LEFT   = 0x110,
	BTN_RIGHT  = 0x111,
	BTN_MIDDLE = 0x112,
};

enum {
	MOD1  = SWC_MOD_ALT,
	MOD4  = SWC_MOD_LOGO,
	SHIFT = SWC_MOD_SHIFT,
	CTRL  = SWC_MOD_CTRL,
};

union arg {
	int         i;
	uint32_t    u;
	float       f;
	const void* v;
};

struct bind {
	uint32_t  type;
	uint32_t  mods;
	uint32_t  ksym;
	union arg arg;
	void      (*fn)(void* data, uint32_t time, uint32_t value, uint32_t state);
};

struct client {
	struct wl_list tiled_link;
	struct wl_list float_link;
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

struct config {
	bool natural_scrolling;
	bool tap_to_click;
	uint32_t motion_throttle_hz;
	uint32_t master_width;
	uint32_t master_resize;
	uint32_t border_col_active;
	uint32_t border_col_normal;
	uint32_t border_width;
	uint32_t gaps;
};

struct grab {
	struct client* c;

	bool active, resize;
};

struct screen {
	struct wl_list     link;
	struct swc_screen* scr;

	int32_t  x, y;
	uint32_t w, h;
};

struct wm {
	struct wl_event_loop* ev_loop;

	struct wl_display* dpy;
	struct wl_list     screens, tiled, floating;
	struct screen*     sel_screen;
	struct client*     sel_client;
	struct grab        grab;
	bool               global_floating;
	uint8_t            ws;
};

void focus_next(void* data, uint32_t time, uint32_t value, uint32_t state);
void focus_prev(void* data, uint32_t time, uint32_t value, uint32_t state);
void kill_sel(void* data, uint32_t time, uint32_t value, uint32_t state);
void master_next(void* data, uint32_t time, uint32_t value, uint32_t state);
void master_prev(void* data, uint32_t time, uint32_t value, uint32_t state);
void master_resize(void* data, uint32_t time, uint32_t value, uint32_t state);
void mouse_move(void* data, uint32_t time, uint32_t value, uint32_t state);
void mouse_resize(void* data, uint32_t time, uint32_t value, uint32_t state);
void new_screen(struct swc_screen* scr);
void new_window(struct swc_window* win);
void new_device(struct libinput_device* dev);
void quit(void* data, uint32_t time, uint32_t value, uint32_t state);
void spawn(void* data, uint32_t time, uint32_t value, uint32_t state);
void toggle_float(void* data, uint32_t time, uint32_t value, uint32_t state);
void toggle_float_global(void* data, uint32_t time, uint32_t value, uint32_t state);
void workspace_goto(void* data, uint32_t time, uint32_t value, uint32_t state);
void workspace_moveto(void* data, uint32_t time, uint32_t value, uint32_t state);

extern struct wm wm;

#endif /* COMMON_H */
