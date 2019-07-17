#include <xcb/xcb_keysyms.h>
#include <X11/keysym.h>

#define STR_MAX 255

typedef struct {
        uint32_t mod;
        xcb_keysym_t key;
        char command[STR_MAX];
} hot_key;
