#include "types.h"

#define MOD XCB_MOD_MASK_4
#define SHIFT XCB_MOD_MASK_SHIFT

const key keys[] = {
	{ MOD, XK_w, "qutebrowser &" },
	{ MOD | SHIFT, XK_e, "tabbed -c -r 2 st -w '' &" },
	{ MOD, XK_e, "st &" },
	{ MOD, XK_Up, "amixer set Master 5db+" },
	{ MOD, XK_Down, "amixer set Master 5db-" },
};
