#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

#include <swc.h>
#include <wayland-server.h>
#include <xkbcommon/xkbcommon-keysyms.h>

#include "config.h"
#include "extern.h"
#include "types.h"
#include "util.h"

static void setup(void);
static void setup_binds(void);

struct wm wm;
const struct swc_manager manager = {
	.new_screen = new_screen, .new_window = new_window, .new_device = new_device,
};

static void setup(void)
{
	/* display */
	wm.dpy = wl_display_create();
	if (!wm.dpy)
		die (EXIT_FAILURE, "wl_display_create failed");

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
	wl_log(stderr, "WAYLAND_DISPLAY=%s\n", sock);

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

int main(void)
{
	setup();
	wl_display_run(wm.dpy);
	swc_finalize();
	wl_display_destroy(wm.dpy);
	return EXIT_SUCCESS;
}

