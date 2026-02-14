#include <stdio.h>
#include <stdlib.h>

#include <swc.h>
#include <wayland-server.h>
#include <xkbcommon/xkbcommon-keysyms.h>

#include "extern.h"
#include "types.h"
#include "util.h"

struct wm wm;
const struct swc_manager manager = {
	.new_screen = new_screen, .new_window = new_window, .new_device = new_device,
};

void setup(void);

void setup(void)
{
	/* init display */
	wm.dpy = wl_display_create();
	if (!wm.dpy)
		die (EXIT_FAILURE, "wl_display_create failed");

	/* init event loop */
	wm.ev_loop = wl_display_get_event_loop(wm.dpy);
	if (!swc_initialize(wm.dpy, wm.ev_loop, &manager))
		die(EXIT_FAILURE, "swc_initialize failed\n");

	/* TODO: temp wallpaper and quit binding */
	swc_wallpaper_color_set(0xff1e1f21);
	swc_add_binding(SWC_BINDING_KEY, SWC_MOD_LOGO | SWC_MOD_SHIFT, XKB_KEY_e, quit, NULL);

	/* create display socket */
	const char* sock;
	sock = wl_display_add_socket_auto(wm.dpy);
	if (!sock)
		die(EXIT_FAILURE, "wl_display_add_socket_auto failed\n");
	setenv("WAYLAND_DISPLAY", sock, 1);
	fprintf(stderr, "WAYLAND_DISPLAY=%s\n", sock);
}

int main(void)
{
	setup();
	wl_display_run(wm.dpy);
	swc_finalize();
	wl_display_destroy(wm.dpy);
	return EXIT_SUCCESS;
}

