#include <xcb/xcb.h>
#include <xcb/xcb_keysyms.h>
#include <X11/keysym.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define STR_MAX 255

#define RELOAD_KEY "hot_reload"

#define LOG(A) printf("hotman: " A ".\n");

typedef struct {
 char id[STR_MAX];
 xcb_keysym_t key;
} hot_key_map_t;

typedef struct {
 char id[STR_MAX];
 uint32_t mod;
} hot_mod_map_t;

typedef struct {
 uint32_t mod;
 xcb_keysym_t key;
 char command[STR_MAX];
} hot_key_t;

static hot_mod_map_t hot_mod_map[] = {
 { "ctrl" , XCB_MOD_MASK_CONTROL },
 { "alt" , XCB_MOD_MASK_1 },
 { "super" , XCB_MOD_MASK_4 },
 { "shift" , XCB_MOD_MASK_SHIFT },
};

static hot_key_map_t hot_key_map[] = {
 { "a" , XK_a },
 { "b" , XK_b },
 { "backspace" , XK_BackSpace },
 { "c" , XK_c },
 { "d" , XK_d },
 { "delete" , XK_Delete },
 { "down" , XK_Down },
 { "e" , XK_e },
 { "end" , XK_End },
 { "escape" , XK_Escape },
 { "f" , XK_f },
 { "g" , XK_g },
 { "h" , XK_h },
 { "home" , XK_Home },
 { "i" , XK_i },
 { "j" , XK_j },
 { "k" , XK_k },
 { "l" , XK_l },
 { "left" , XK_Left },
 { "m" , XK_m },
 { "n" , XK_n },
 { "o" , XK_o },
 { "p" , XK_p },
 { "page_down" , XK_Page_Down },
 { "page_up" , XK_Page_Up },
 { "pause" , XK_Pause },
 { "q" , XK_q },
 { "r" , XK_r },
 { "return" , XK_Return },
 { "right" , XK_Right },
 { "s" , XK_s },
 { "scroll_lock" , XK_Scroll_Lock },
 { "t" , XK_t },
 { "tab" , XK_Tab },
 { "u" , XK_u },
 { "up" , XK_Up },
 { "v" , XK_v },
 { "w" , XK_w },
 { "x" , XK_x },
 { "y" , XK_y },
 { "z" , XK_z },
};

static xcb_connection_t *connection;
static xcb_screen_t *screen;
static hot_key_t *keys = NULL;
static int keys_len,
           keys_max = 8;

static void hot_add_key(hot_key_t subject) {
 if (keys_len + 1 == keys_max) {
  keys_max *= 2;
  hot_key_t *temp = malloc(keys_max * sizeof(hot_key_t));
  for (int i = 0; i < keys_len; i++) *(temp + i) = *(keys + i);
  free(keys);
  keys = temp;
 } else *(keys + keys_len) = subject;
 keys_len++;
}

static int hot_first(char key, int depth, int l, int r) {
 int piv = l + (r - l) / 2;
 if (l == r + 1) return piv;
 else if (hot_key_map[piv].id[depth] > key - 1) return hot_first(key, depth, l, piv - 1);
 else if(hot_key_map[piv].id[depth] < key) return hot_first(key, depth, piv + 1, r);
}

static int hot_search(char *key, int depth, int l, int r) {
 int a = hot_first(key[depth], depth, l, r);
 int b = hot_first(key[depth]+1, depth, l, r) - 1;
 if (a == b) return a;
 else return hot_search(key, depth+1, a, b);
}

static void hot_cleanup(void) {
 free(keys);
 xcb_ungrab_key(connection, XCB_GRAB_ANY, screen->root, XCB_MOD_MASK_ANY);
 xcb_flush(connection);
 xcb_disconnect(connection);
}

static xcb_keysym_t hot_get_keysym(xcb_keycode_t keycode) {
 xcb_key_symbols_t *keysyms = xcb_key_symbols_alloc(connection);
 xcb_keysym_t keysym = xcb_key_symbols_get_keysym(keysyms, keycode, 0);
 xcb_key_symbols_free(keysyms);
 return keysym;
}

int nscmp(char *one, char *two) {
 for (int i = 0; *one && *two; i++)
  if (*one == ' ') one++;
  else if (*one == ' ') two++;
  else if (*one++ != *two++) return 0;
 return 1;
}

char kgetc(FILE *subject) {
 char found = fgetc(subject);
 while (found == ' ') found = fgetc(subject);
 return found;
}

static int hot_read_config(char *path) {
 FILE *config;
 if (!(config = fopen(path, "r"))) {
  printf("hotman: can't open config.\n");
  return 0;
 }

 if (keys) free(keys);
 keys = malloc(keys_max * sizeof(hot_key_t));

 int i;
 hot_key_t new;
 char scan[STR_MAX] = { '\0' },
      c,
      p;

 c = kgetc(config);
 for (keys_len = 0; c != EOF;) {
  new.mod = 0;
  while (c != '=') {
   for (i = 0; c != '+' && c != '='; i++) {
    scan[i] = c; 
    c = kgetc(config);
   }
   scan[i] = '\0';
   if (c == '=') break; 
   else {
    for (int i = 0; i < sizeof(hot_mod_map)/sizeof(hot_mod_map_t); i++) {
     if (nscmp(scan, hot_mod_map[i].id)) new.mod |= hot_mod_map[i].mod;
    }
    c = kgetc(config);
   }
  }

  int found = hot_search(scan, 0, 0, sizeof(hot_key_map)/sizeof(hot_key_map_t) - 1);
  new.key = hot_key_map[found].key;
  //printf("\nDAMN: %s %s\n\n", scan, hot_key_map[found].id);
  printf("%d\n", keys_len);

  c = kgetc(config);
  for (i = 0; c != '\n' && c != EOF; i++) {
   new.command[i] = c;
   c = fgetc(config);
  }
  new.command[i] = '\0';
  c = kgetc(config);
  hot_add_key(new);
 }

 fclose(config);

 xcb_ungrab_key(connection, XCB_GRAB_ANY, screen->root, XCB_MOD_MASK_ANY);
 
 xcb_keycode_t *keycode;
 xcb_key_symbols_t *keysyms = xcb_key_symbols_alloc(connection);
 for (int i = 0; i < keys_len; i++) {
  keycode = xcb_key_symbols_get_keycode(keysyms, (keys+i)->key);
  xcb_grab_key(connection, 0, screen->root, (keys+i)->mod, *keycode, XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC);
  free(keycode);
 }
 xcb_key_symbols_free(keysyms);

 return 1;
}

int main(int argc, char **argv) {
 if (argc > 2) {
  printf("hotman: too many arguments.\n");
  return 0;
 } else if (argc < 2) {
  printf("hotman: no config specified.\n");
  return 0;
 }
 
 //setup connection
 uint32_t values[] = { XCB_EVENT_MASK_KEY_PRESS };
 connection = xcb_connect(NULL, NULL);
 screen = xcb_setup_roots_iterator(xcb_get_setup(connection)).data;
 xcb_change_window_attributes_checked(connection,
   screen->root,
   XCB_CW_EVENT_MASK,
   values);
 xcb_flush(connection);

 if (!hot_read_config(argv[1])) return 0;

 //register cleanup
 atexit(hot_cleanup);

 //event loop
 xcb_generic_event_t *event;
 xcb_key_press_event_t *e;
 xcb_keysym_t keysym;
 for (;;) {
  if (xcb_connection_has_error(connection) != 0) {
   xcb_disconnect(connection);
   break;
  }
  event = xcb_wait_for_event(connection);
  if (event && (event->response_type & ~0x80) == XCB_KEY_PRESS) {
   e = (xcb_key_press_event_t *)event;
   keysym = hot_get_keysym(e->detail);
   for (int i = 0; i < keys_len; i++)
    if (keysym == (keys + i)->key && (keys + i)->mod == e->state) {
     if (nscmp((keys + i)->command, RELOAD_KEY)) hot_read_config(argv[1]);
     else system((keys + i)->command);
    }
  }
  xcb_flush(connection);
  free(event);
 }

 return 0;
}
