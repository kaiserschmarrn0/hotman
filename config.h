#include "hotman.h"

#define MOD XCB_MOD_MASK_4
#define SHIFT XCB_MOD_MASK_SHIFT

static const hot_key keys[] = {
	{  MOD, XK_e, "st -e ksh -l &" },
	{  MOD, XK_w, "chrome &" },
	{  MOD, XK_Up, "mixerctl outputs.master=+5 &" },
	{  MOD, XK_Down, "mixerctl outputs.master=+5 &" },
	{  MOD, XK_m, "mixerctl outputs.master.mute=toggle &" },
	{  MOD | SHIFT, XK_Up, "xbacklight -inc 10 &" },
	{  MOD | SHIFT, XK_Down, "xbacklight -dec 5 &" },
};
