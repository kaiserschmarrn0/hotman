#include "types.h"

#define MOD XCB_MOD_MASK_4

const key keys[] = {
	{ MOD, XK_w, "qutebrowser &" },
	{ MOD, XK_t, "tabbed -c -r 2 st -w '' &" },
	{ MOD, XK_e, "st &" },
	{ MOD, XK_s, "tabbed -c surf -e &" },
	{ MOD, XK_Up, "amixer set Master 5db+" },
	{ MOD, XK_Down, "amixer set Master 5db-" },
	{ MOD, XK_x, "xfce4-terminal &" },
	{ MOD, XK_z, "firefox &" },
	{ MOD, XK_d, "gimp &" }
};
