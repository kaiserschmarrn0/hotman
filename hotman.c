//hotman, the hotkey manager for araiwm

#include <xcb/xcb.h>
#include <X11/keysym.h>
#include <stdlib.h>
#include "config.h"

static xcb_connection_t	*connection;
static xcb_screen_t	*screen;

static void
hot_cleanup(void)
{
	xcb_ungrab_key(connection,
			XCB_GRAB_ANY,
			screen->root,
			XCB_MOD_MASK_ANY);
	xcb_flush(connection);
	xcb_disconnect(connection);
}

static void
hot_init(void)
{
	uint32_t values[] = { XCB_EVENT_MASK_KEY_PRESS };
	connection = xcb_connect(NULL, NULL);
	screen = xcb_setup_roots_iterator(xcb_get_setup(connection)).data;
	xcb_change_window_attributes_checked(connection,
			screen->root,
			XCB_CW_EVENT_MASK,
			values);
	xcb_flush(connection);
}

static void
hot_keygrab(void)
{
	xcb_keycode_t keycode;
	xcb_key_symbols_t *keysyms = xcb_key_symbols_alloc(connection);
	for (int i = 0; i < 7; i++) {
		keycode = *xcb_key_symbols_get_keycode(keysyms, keys[i].key);
		xcb_grab_key(connection,
				0,
				screen->root,
				keys[i].mod,
				keycode,
				XCB_GRAB_MODE_ASYNC,
				XCB_GRAB_MODE_ASYNC);
	}
	xcb_key_symbols_free(keysyms);
}

static xcb_keysym_t
hot_get_keysym(xcb_keycode_t keycode)
{
	xcb_key_symbols_t *keysyms = xcb_key_symbols_alloc(connection);
	xcb_keysym_t keysym = xcb_key_symbols_get_keysym(keysyms, keycode, 0);
	xcb_key_symbols_free(keysyms);
	return keysym;
}

static void
hot_loop(void)
{
	xcb_generic_event_t *event;
	xcb_key_press_event_t *e;
	xcb_keysym_t keysym;
	for (;;) {
		if (xcb_connection_has_error(connection) != 0) {
			xcb_disconnect(connection);
			return;
		}
		event = xcb_wait_for_event(connection);
		if (event && (event->response_type & ~0x80) == XCB_KEY_PRESS) {
			e = (xcb_key_press_event_t *)event;
			keysym = hot_get_keysym(e->detail);
			for (int i = 0; i < sizeof(keys)/sizeof(keys[0]); i++)
				if (keysym == keys[i].key)
					system(keys[i].command);
		}
		xcb_flush(connection);
		free(event);
	}
}

int
main(void)
{
	hot_init();
	hot_keygrab();
	hot_loop();
	atexit(hot_cleanup);
	return 0;
}
