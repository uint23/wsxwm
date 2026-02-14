#ifndef CONFIG_H
#define CONFIG_H

#include <xkbcommon/xkbcommon-keysyms.h>


#include "types.h"
#include "wsxwm.h"

static struct bind binds[] = {
	{ SWC_BINDING_KEY, MOD4 | SHFT, XKB_KEY_e, { .v = NULL }, quit },
};

#endif /* CONFIG_H */

