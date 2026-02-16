#ifndef CONFIG_H
#define CONFIG_H

#include <xkbcommon/xkbcommon-keysyms.h>


#include "types.h"
#include "wsxwm.h"

static const struct config cfg = {
	.motion_throttle_hz = 85,
	.border_col_active = 0xffed953e,
	.border_col_normal = 0xff444444,
	.border_width = 1,
	.master_width = 60,  /* % of screen */
	.gaps = 0,
};

static const char* termcmd[] = { "st-wl", NULL };
static struct bind binds[] = {
	{ SWC_BINDING_KEY,    MOD4,        XKB_KEY_Return, { .v = termcmd }, spawn },
	{ SWC_BINDING_KEY,    MOD4|SHFT,   XKB_KEY_e,      { .v = NULL },    quit },
	{ SWC_BINDING_KEY,    MOD4,        XKB_KEY_k,      { .v = NULL },    focus_next },
	{ SWC_BINDING_KEY,    MOD4,        XKB_KEY_j,      { .v = NULL },    focus_prev },
	{ SWC_BINDING_KEY,    MOD4|SHFT,   XKB_KEY_q,      { .v = NULL },    kill_sel },
	{ SWC_BINDING_BUTTON, MOD4,        BTN_LEFT,       { .v = NULL },    mouse_move },
	{ SWC_BINDING_BUTTON, MOD4,        BTN_RIGHT,      { .v = NULL },    mouse_resize },
	{ SWC_BINDING_KEY,    MOD4,        XKB_KEY_space,  { .v = NULL },    toggle_float },
	{ SWC_BINDING_KEY,    MOD4,        XKB_KEY_l,      { .i = 50 },      master_resize },
	{ SWC_BINDING_KEY,    MOD4,        XKB_KEY_h,      { .i = -50 },     master_resize },
	{ SWC_BINDING_KEY,    MOD4|SHFT,   XKB_KEY_k,      { .v = NULL },    master_next },
	{ SWC_BINDING_KEY,    MOD4|SHFT,   XKB_KEY_j,      { .v = NULL },    master_prev },
};

#endif /* CONFIG_H */

