#include <xcb/xcb_keysyms.h>

typedef struct {
	uint16_t mod;
	xcb_keysym_t key;
	char* command;
} key;
