#ifndef CONFIG_H
#define CONFIG_H

#include <xkbcommon/xkbcommon-keysyms.h>


#include "types.h"
#include "wsxwm.h"

/* TODO: temp border values */
static const uint32_t border_color_active = 0xff6699cc;
static const uint32_t border_color_normal = 0xff444444;
static const uint32_t border_width        = 1;

static const char* termcmd[] = { "st-wl", NULL };

static struct bind binds[] = {
	{ SWC_BINDING_KEY, MOD4,        XKB_KEY_Return, { .v = termcmd }, spawn },
	{ SWC_BINDING_KEY, MOD4|SHFT,   XKB_KEY_e,      { .v = NULL },    quit },
	{ SWC_BINDING_KEY, MOD4,        XKB_KEY_j,      { .v = NULL },    focus_next },
	{ SWC_BINDING_KEY, MOD4,        XKB_KEY_k,      { .v = NULL },    focus_prev },

};

#endif /* CONFIG_H */

