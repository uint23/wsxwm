#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <swc.h>
#include <wayland-server.h>
#include <wayland-util.h>
#include <xkbcommon/xkbcommon-keysyms.h>

#include "config.h"
#include "types.h"
#include "util.h"
#include "wsxwm.h"

static void focus(struct client* c, bool raise);
static void on_screen_destroy(void* data);
static void on_screen_usable_geometry_changed(void* data);
static void on_win_destroy(void* data);
static void on_win_entered(void* data);
static void setup(void);
static void setup_binds(void);
static void set_floating(struct client* c, bool floating, bool raise);
static void tile(struct screen* s);

/* master width in px */
static uint32_t master_width = 0;

struct wm wm;
const struct swc_manager manager = {
	.new_screen = new_screen, .new_window = new_window, .new_device = new_device,
};
struct swc_window_handler window_handler = {
	.destroy = on_win_destroy, .entered = on_win_entered,
};
struct swc_screen_handler screen_handler = {
	.destroy = on_screen_destroy,
	.usable_geometry_changed = on_screen_usable_geometry_changed,
};

static void focus(struct client* c, bool raise)
{
	if (wm.sel_client)
		swc_window_set_border(
			wm.sel_client->win,
			cfg.border_col_normal, cfg.border_width,
			0, 0
		);

	if (c)
		swc_window_set_border(
			c->win,
			cfg.border_col_active, cfg.border_width,
			0, 0
		);

	if (raise && c && c->floating)
		set_floating(c, true, true);

	swc_window_focus(c ? c->win : NULL);
	wm.sel_client = c;
}

static void on_screen_destroy(void* data)
{
	struct screen* s = data;

	if (!s)
		return;

	wl_list_remove(&s->link);

	if (wm.sel_screen == s) {
		if (wl_list_empty(&wm.screens))
			wm.sel_screen = NULL;
		else
			wm.sel_screen = wl_container_of(wm.screens.next, wm.sel_screen, link);
	}

	free(s);
}

static void on_screen_usable_geometry_changed(void* data)
{
	struct screen* s = data;

	tile(s);
}

static void on_win_destroy(void* data)
{
	struct client* c = data;
	struct client* next;

	if (!c)
		return;

	if (wm.grab.active && wm.grab.c == c) {
		wm.grab.active = false;
		wm.grab.c = NULL;
	}

	if (c->floating)
		wl_list_remove(&c->float_link);
	else
		wl_list_remove(&c->tiled_link);

	if (wm.sel_client == c) {
		wm.sel_client = NULL;
		next = first_float(c->scr);
		if (!next)
			next = first_tiled(c->scr);
		focus(next, true);
	}

	tile(c->scr);
	free(c);
}

static void on_win_entered(void* data)
{
	if (wm.grab.active)
		return;

	struct client* c = data;
	focus(c, true);
}

static void setup(void)
{
	/* display */
	wm.dpy = wl_display_create();
	if (!wm.dpy)
		die(EXIT_FAILURE, "wl_display_create failed");

	/* variables */
	wl_list_init(&wm.screens);
	wl_list_init(&wm.tiled);
	wl_list_init(&wm.floating);
	wm.sel_client = NULL;
	wm.sel_screen = NULL;
	wm.grab.active = false;
	wm.grab.resize = false;
	wm.grab.c = NULL;

	/* event loop */
	wm.ev_loop = wl_display_get_event_loop(wm.dpy);
	if (!swc_initialize(wm.dpy, wm.ev_loop, &manager))
		die(EXIT_FAILURE, "swc_initialize failed\n");

	/* TODO: temp wallpaper and quit binding */
	swc_wallpaper_color_set(0xff1e1f21);
	setup_binds();

	/* display socket */
	const char* sock;
	sock = wl_display_add_socket_auto(wm.dpy);
	if (!sock)
		die(EXIT_FAILURE, "wl_display_add_socket_auto failed\n");
	setenv("WAYLAND_DISPLAY", sock, 1);
	_log(stderr, "WAYLAND_DISPLAY=%s\n", sock);

	/* signals */
	signal(SIGINT,  sig_handler);
	signal(SIGTERM, sig_handler);
	signal(SIGQUIT, sig_handler);
}

static void setup_binds(void)
{
	for (size_t i = 0; i < LENGTH(binds); i++) {
		const struct bind* b = &binds[i];
		swc_add_binding(b->type, b->mods, b->ksym, b->fn, (void*)&b->arg);
	}
}

static void set_floating(struct client* c, bool floating, bool raise)
{
	if (!c)
		return;

	/* client must be in exactly one list */
	if (floating) {
		if (!c->floating) {
			c->floating = true;
			wl_list_remove(&c->tiled_link);
			wl_list_insert(&wm.floating, &c->float_link);
		}
		else if (raise) {
			wl_list_remove(&c->float_link);
			wl_list_insert(&wm.floating, &c->float_link);
		}

		swc_window_set_stacked(c->win);
	}
	else {
		if (c->floating) {
			c->floating = false;
			wl_list_remove(&c->float_link);
			wl_list_insert(&wm.tiled, &c->tiled_link);
		}

		swc_window_set_tiled(c->win);
	}
}

static void tile(struct screen* s)
{
	struct client* c;
	struct swc_rectangle geom;
	struct swc_rectangle* scr_geom;

	int32_t out_gaps = cfg.gaps + cfg.border_width;
	int32_t in_gaps = cfg.gaps + (cfg.border_width * 2);

	int32_t x;
	int32_t y;
	uint32_t w;
	uint32_t h;

	if (!s)
		return;

	scr_geom = &s->scr->usable_geometry;

	size_t n = 0;
	wl_list_for_each(c, &wm.tiled, tiled_link) {
		if (is_tiled(c, s))
			n++;
	}
	if (n == 0)
		return;

	x = scr_geom->x + out_gaps;
	y = scr_geom->y + out_gaps;
	w = scr_geom->width - (out_gaps * 2);
	h = scr_geom->height - (out_gaps * 2);

	if (n == 1) {
		wl_list_for_each(c, &wm.tiled, tiled_link) {
			if (!is_tiled(c, s))
				continue;

			geom.x = x;
			geom.y = y;
			geom.width  = w;
			geom.height = h;

			swc_window_set_geometry(c->win, &geom);
			return;
		}
	}

	size_t i = 0;
	wl_list_for_each(c, &wm.tiled, tiled_link) {
		if (!is_tiled(c, s))
			continue;

		if (master_width == 0) /* uninitialised */
			master_width = ((w - in_gaps) * cfg.master_width) / 100;

		if (i == 0) {
			/* master */
			geom.x = x;
			geom.y = y;
			geom.width = master_width;
			geom.height = h;
		}
		else {
			/* stack */
			uint32_t stackn = n - 1;
			uint32_t idx = i - 1;

			uint32_t stack_height = (h - in_gaps * (stackn - 1)) / stackn;

			geom.x = x + master_width + in_gaps;
			geom.y = y + (idx * (stack_height + in_gaps));
			geom.width  = w - master_width - in_gaps;
			geom.height = stack_height;
		}

		swc_window_set_geometry(c->win, &geom);
		i++;
	}
}

void focus_next(void* data, uint32_t time, uint32_t value, uint32_t state)
{
	(void)data;
	(void)time;
	(void)value;

	struct client* c;
	struct screen* s;

	if (state != WL_KEYBOARD_KEY_STATE_PRESSED)
		return;

	if (!wm.sel_screen)
		return;

	s = wm.sel_screen;

	if (!wm.sel_client) {
		c = first_float(s);
		if (!c)
			c = first_tiled(s);
		focus(c, false);
		return;
	}

	/* do not raise/reorder floats while cycling */
	if (wm.sel_client->floating) {
		for (c = wl_container_of(wm.sel_client->float_link.next, c, float_link);
			&c->float_link != &wm.floating;
			c = wl_container_of(c->float_link.next, c, float_link)) {
			if (is_float(c, s)) {
				focus(c, false);
				return;
			}
		}

		c = first_tiled(s);
		if (!c)
			c = first_float(s);
		if (c)
			focus(c, false);
		return;
	}

	for (c = wl_container_of(wm.sel_client->tiled_link.next, c, tiled_link);
		&c->tiled_link != &wm.tiled;
		c = wl_container_of(c->tiled_link.next, c, tiled_link)) {
		if (is_tiled(c, s)) {
			focus(c, false);
			return;
		}
	}

	c = first_float(s);
	if (!c)
		c = first_tiled(s);
	focus(c, false);
}

void focus_prev(void* data, uint32_t time, uint32_t value, uint32_t state)
{
	(void)data;
	(void)time;
	(void)value;

	struct client* c;
	struct screen* s;

	if (state != WL_KEYBOARD_KEY_STATE_PRESSED)
		return;

	if (!wm.sel_screen)
		return;

	s = wm.sel_screen;

	if (!wm.sel_client) {
		c = first_float(s);
		if (!c)
			c = first_tiled(s);
		focus(c, false);
		return;
	}

	if (wm.sel_client->floating) {
		for (c = wl_container_of(wm.sel_client->float_link.prev, c, float_link);
			&c->float_link != &wm.floating;
			c = wl_container_of(c->float_link.prev, c, float_link)) {
			if (is_float(c, s)) {
				focus(c, false);
				return;
			}
		}

		c = last_tiled(s);
		if (!c)
			c = last_float(s);
		if (c)
			focus(c, false);
		return;
	}

	for (c = wl_container_of(wm.sel_client->tiled_link.prev, c, tiled_link);
		&c->tiled_link != &wm.tiled;
		c = wl_container_of(c->tiled_link.prev, c, tiled_link)) {
		if (is_tiled(c, s)) {
			focus(c, false);
			return;
		}
	}

	c = last_float(s);
	if (!c)
		c = last_tiled(s);
	focus(c, false);
}

void master_next(void* data, uint32_t time, uint32_t value, uint32_t state)
{
	(void)data;
	(void)time;
	(void)value;

	struct client* first;
	struct client* last;
	struct screen* s;

	if (state != WL_KEYBOARD_KEY_STATE_PRESSED)
		return;

	if (!wm.sel_screen)
		return;

	s = wm.sel_screen;
	first = first_tiled(s);
	last = last_tiled(s);

	if (!first || !last || first == last)
		return;

	wl_list_remove(&last->tiled_link);
	wl_list_insert(first->tiled_link.prev, &last->tiled_link);
	focus(first_tiled(s), true);
	tile(s);
}

void master_prev(void* data, uint32_t time, uint32_t value, uint32_t state)
{
	(void)data;
	(void)time;
	(void)value;

	struct client* first;
	struct client* last;
	struct screen* s;

	if (state != WL_KEYBOARD_KEY_STATE_PRESSED)
		return;

	if (!wm.sel_screen)
		return;

	s = wm.sel_screen;
	first = first_tiled(s);
	last = last_tiled(s);

	if (!first || !last || first == last)
		return;

	wl_list_remove(&first->tiled_link);
	wl_list_insert(&last->tiled_link, &first->tiled_link);
	focus(first_tiled(s), true);
	tile(s);
}

void kill_sel(void* data, uint32_t time, uint32_t value, uint32_t state)
{
	(void)data;
	(void)time;
	(void)value;

	if (state != WL_KEYBOARD_KEY_STATE_PRESSED)
		return;

	if (!wm.sel_client)
		return;

	swc_window_close(wm.sel_client->win);
}

void master_resize(void* data, uint32_t time, uint32_t value, uint32_t state)
{
	(void)time;
	(void)value;

	union arg* a = data;

	if (state != WL_KEYBOARD_KEY_STATE_PRESSED)
		return;

	if (!wm.sel_screen)
		return;

	{ /* bounds */
		struct swc_rectangle* g = &wm.sel_screen->scr->usable_geometry;

		uint32_t out_gaps = cfg.gaps + cfg.border_width;
		uint32_t in_gaps = cfg.gaps + (cfg.border_width * 2);

		int32_t total = (int32_t)g->width - (int32_t)(out_gaps * 2);

		int32_t min_master = 20;
		int32_t max_master = total - (int32_t)in_gaps - 20;

		if (max_master < min_master)
			max_master = min_master;

		int32_t mw = (int32_t)master_width + a->i;

		if (mw < min_master)
			mw = max_master;
		else if (mw > max_master)
			mw = min_master;

		master_width = (uint32_t)mw;
	}

	tile(wm.sel_screen);
}

void mouse_move(void* data, uint32_t time, uint32_t value, uint32_t state)
{
	(void)data;
	(void)time;
	(void)value;

	if (state == WL_POINTER_BUTTON_STATE_PRESSED) {
		if (!wm.sel_client)
			return;

		if (!wm.sel_client->floating) {
			set_floating(wm.sel_client, true, false);
			tile(wm.sel_client->scr);
		}

		wm.grab.active = true;
		wm.grab.resize = false;
		wm.grab.c = wm.sel_client;

		swc_window_begin_move(wm.grab.c->win);
	}
	else {
		if (!wm.grab.active || wm.grab.resize || !wm.grab.c)
			return;

		swc_window_end_move(wm.grab.c->win);

		wm.grab.active = false;
		wm.grab.c = NULL;
	}
}

void mouse_resize(void* data, uint32_t time, uint32_t value, uint32_t state)
{
	(void)data;
	(void)time;
	(void)value;

	if (state == WL_POINTER_BUTTON_STATE_PRESSED) {
		if (!wm.sel_client)
			return;

		if (!wm.sel_client->floating) {
			set_floating(wm.sel_client, true, false);
			tile(wm.sel_client->scr);
		}

		wm.grab.active = true;
		wm.grab.resize = true;
		wm.grab.c = wm.sel_client;

		swc_window_begin_resize(
			wm.grab.c->win,
			SWC_WINDOW_EDGE_RIGHT | SWC_WINDOW_EDGE_BOTTOM
		);
	}
	else {
		if (!wm.grab.active || !wm.grab.resize || !wm.grab.c)
			return;

		swc_window_end_resize(wm.grab.c->win);

		wm.grab.active = false;
		wm.grab.c = NULL;
	}
}

void new_screen(struct swc_screen* scr)
{
	struct screen* s;

	s = malloc(sizeof(*s));
	if (!s)
		die(EXIT_FAILURE, "new screen calloc failed");

	s->scr = scr;

	s->x = 0;
	s->y = 0;
	s->w = 0;
	s->h = 0;

	wl_list_insert(&wm.screens, &s->link);

	if (!wm.sel_screen)
		wm.sel_screen = s;

	swc_screen_set_handler(scr, &screen_handler, s);

	_log(stderr, "new_screen=%p\n", (void*)scr);
}

void new_window(struct swc_window* win)
{
	struct client* c;

	c = malloc(sizeof(*c));
	if (!c)
		die(EXIT_FAILURE, "malloc client failed");

	win->motion_throttle_ms = 1000 / cfg.motion_throttle_hz;
	win->min_width = 20;
	win->min_height = 20;
	win->max_width = 0;
	win->max_height = 0;

	c->win = win;
	c->scr = wm.sel_screen;
	c->mapped = false;
	c->floating = wm.global_floating;
	c->fullscreen = false;
	c->ws = false;

	if (c->floating) {
		wl_list_insert(&wm.floating, &c->float_link);
		wl_list_init(&c->tiled_link);
		swc_window_set_handler(win, &window_handler, c);
		swc_window_set_stacked(win);
	}
	else {
		wl_list_insert(&wm.tiled, &c->tiled_link);
		wl_list_init(&c->float_link);
		swc_window_set_handler(win, &window_handler, c);
		swc_window_set_tiled(win);
	}
	swc_window_show(win);
	focus(c, true);
	tile(wm.sel_screen);

	_log(stderr, "new_window=%p\n", (void*)win);
}

void new_device(struct libinput_device* dev)
{
	(void)dev;
}

void quit(void* data, uint32_t time, uint32_t value, uint32_t state)
{
	(void)data;
	(void)time;
	(void)value;
	(void)state;

	wl_display_terminate(wm.dpy);
}

void spawn(void* data, uint32_t time, uint32_t value, uint32_t state)
{
	union arg* a = data;
	char* const* cmd = (char* const*)a->v;

	(void)time;
	(void)value;

	if (state != WL_KEYBOARD_KEY_STATE_PRESSED)
		return;

	if (fork() == 0) {
		execvp(cmd[0], cmd);
		_exit(127);
	}
}

void toggle_float(void* data, uint32_t time, uint32_t value, uint32_t state)
{
	(void)data;
	(void)time;
	(void)value;

	if (state != WL_KEYBOARD_KEY_STATE_PRESSED)
		return;

	if (!wm.sel_client)
		return;

	set_floating(wm.sel_client, !wm.sel_client->floating, true);

	tile(wm.sel_client->scr);
}

void toggle_float_global(void* data, uint32_t time, uint32_t value, uint32_t state)
{
	(void)data;
	(void)time;
	(void)value;

	if (state != WL_KEYBOARD_KEY_STATE_PRESSED)
		return;

	wm.global_floating = !wm.global_floating;
}

int main(void)
{
	setup();
	wl_display_run(wm.dpy);
	swc_finalize();
	wl_display_destroy(wm.dpy);
	return EXIT_SUCCESS;
}
