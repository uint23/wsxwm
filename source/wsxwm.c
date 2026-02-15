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

static void focus(struct client* c);
static void on_screen_destroy(void* data);
static void on_win_destroy(void* data);
static void on_win_entered(void* data);
static void setup(void);
static void setup_binds(void);

struct wm wm;
const struct swc_manager manager = {
	.new_screen = new_screen, .new_window = new_window, .new_device = new_device,
};
struct swc_window_handler window_handler = {
	.destroy = on_win_destroy,
	.entered = on_win_entered,
};
struct swc_screen_handler screen_handler = {
	.destroy = on_screen_destroy,
};

static void focus(struct client* c)
{
	if (wm.sel_client)
		swc_window_set_border(
			wm.sel_client->win,
			border_color_normal, border_width,
			border_color_normal, border_width
		);

	if (c)
		swc_window_set_border(
			c->win,
			border_color_active, border_width,
			border_color_active, border_width
		);

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

static void on_win_destroy(void* data)
{
	struct client* c = data;
	struct client* next = NULL;

	if (!c)
		return;

	if (wm.sel_client == c) {
		if (c->link.next != &wm.clients)
			next = wl_container_of(c->link.next, next, link);
		else if (c->link.prev != &wm.clients)
			next = wl_container_of(c->link.prev, next, link);
	}

	wl_list_remove(&c->link);
	focus(next);
	free(c);
}

static void on_win_entered(void* data)
{
	struct client* c = data;

	focus(c);
}

static void setup(void)
{
	/* display */
	wm.dpy = wl_display_create();
	if (!wm.dpy)
		die(EXIT_FAILURE, "wl_display_create failed");

	/* lists */
	wl_list_init(&wm.screens);
	wl_list_init(&wm.clients);
	wm.sel_client = NULL;
	wm.sel_screen = NULL;

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

void focus_next(void* data, uint32_t time, uint32_t value, uint32_t state)
{
	(void)data;
	(void)time;
	(void)value;

	struct client* c = NULL;

	if (state != WL_KEYBOARD_KEY_STATE_PRESSED)
		return;

	if (wl_list_empty(&wm.clients))
		return;

	if (!wm.sel_client)
		c = wl_container_of(wm.clients.next, c, link);
	else if (wm.sel_client->link.next != &wm.clients)
		c = wl_container_of(wm.sel_client->link.next, c, link);
	else
		c = wl_container_of(wm.clients.next, c, link);

	focus(c);
}

void focus_prev(void* data, uint32_t time, uint32_t value, uint32_t state)
{
	struct client* c = NULL;

	(void)data;
	(void)time;
	(void)value;

	if (state != WL_KEYBOARD_KEY_STATE_PRESSED)
		return;

	if (wl_list_empty(&wm.clients))
		return;

	if (!wm.sel_client)
		c = wl_container_of(wm.clients.prev, c, link);
	else if (wm.sel_client->link.prev != &wm.clients)
		c = wl_container_of(wm.sel_client->link.prev, c, link);
	else
		c = wl_container_of(wm.clients.prev, c, link);

	focus(c);
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

void new_screen(struct swc_screen* scr)
{
	struct screen* s;

	s = malloc(sizeof(*s));
	if (!s)
		die(EXIT_FAILURE, "new screen calloc failed");

	s->scr = scr;

	/* TODO: query screen geom */
	struct swc_rectangle r;

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

	c->win = win;
	c->scr = wm.sel_screen;
	c->mapped = 0;
	c->floating = 0;
	c->fullscreen = 0;
	c->ws = 0;

	wl_list_insert(&wm.clients, &c->link);
	swc_window_set_handler(win, &window_handler, c);
	swc_window_set_tiled(win);
	swc_window_show(win);
	focus(c);

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

int main(void)
{
	setup();
	wl_display_run(wm.dpy);
	swc_finalize();
	wl_display_destroy(wm.dpy);
	return EXIT_SUCCESS;
}

