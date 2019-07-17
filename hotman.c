#include <signal.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>

#include <xcb/xcb.h>
#include <xcb/xcb_keysyms.h>

#include "config.h"

#define LEN(A) (sizeof(A)/sizeof(*A))

static int sigcode = 0;

static xcb_connection_t *connection;
static xcb_screen_t *screen;
static xcb_key_symbols_t *keysyms = NULL;

static void catch(int sig) {
	sigcode = sig;
}

static void key_press(xcb_generic_event_t *ev) {
	xcb_key_press_event_t *e = (xcb_key_press_event_t *)ev;
	
	xcb_keysym_t keysym = xcb_key_symbols_get_keysym(keysyms, e->detail, 0);
	
	for (int i = 0; i < LEN(keys); i++) {
		if (keysym == keys[i].key && keys[i].mod == e->state) {
			system(keys[i].command);
		}
	}
}

static void first_grab(void) {
	keysyms = xcb_key_symbols_alloc(connection);
	for (int i = 0; i < LEN(keys); i++) {
		xcb_keycode_t *keycode = xcb_key_symbols_get_keycode(keysyms, keys[i].key);
		xcb_grab_key(connection, 0, screen->root, keys[i].mod, *keycode, XCB_GRAB_MODE_ASYNC,
				XCB_GRAB_MODE_ASYNC);

		free(keycode);
	}
}

static void grab_keys(void) {
	xcb_key_symbols_free(keysyms);
	first_grab();
}

static void mapping_notify(xcb_generic_event_t *ev) {
	xcb_mapping_notify_event_t *e = (xcb_mapping_notify_event_t *)ev;
	if (e->request != XCB_MAPPING_MODIFIER && e->request != XCB_MAPPING_KEYBOARD) {
		return;
	}

	xcb_ungrab_key(connection, XCB_GRAB_ANY, screen->root, XCB_MOD_MASK_ANY);
	grab_keys();
}

int main(void) {
	signal(SIGINT, catch);
	signal(SIGTERM, catch);

	connection = xcb_connect(NULL, NULL);
	screen = xcb_setup_roots_iterator(xcb_get_setup(connection)).data;
	
	uint32_t values[] = { XCB_EVENT_MASK_KEY_PRESS };
	xcb_change_window_attributes_checked(connection, screen->root, XCB_CW_EVENT_MASK, values);

	first_grab();

	static void (*events[XCB_NO_OPERATION])(xcb_generic_event_t *event) = { NULL };
	events[XCB_KEY_PRESS] = key_press;
	events[XCB_MAPPING_NOTIFY] = mapping_notify;

	struct pollfd fd;
	fd.fd = xcb_get_file_descriptor(connection);
	fd.events = POLLIN;

	for (; sigcode == 0;) {
		xcb_flush(connection);

		if (poll(&fd, 1, -1) == -1) {
			break;
		}
		
		xcb_generic_event_t *event;
		while((event = xcb_poll_for_event(connection))) {	
			if (event->response_type == 0) {
				xcb_generic_error_t *error = (xcb_generic_error_t *)event;
				fprintf(stderr, "hotman: error:\n\terror_code: %d\n\tmajor_code: "
						"%d\n\tminor_code: %d\n", error->error_code,
						error->major_code, error->minor_code);
			}

			const unsigned int index = event->response_type & ~0x80;
			if (events[index]) {
				events[index](event);
			}
			free(event);
		}
	}

	xcb_key_symbols_free(keysyms);
	xcb_disconnect(connection);

	return 0;
}
