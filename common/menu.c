// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2010-2011 Calxeda, Inc.
 * Copyright (c) 2019, NVIDIA CORPORATION. All rights reserved.
 */

#include <ansi.h>
#include <cli.h>
#include <malloc.h>
#include <errno.h>
#include <linux/ctype.h>
#include <linux/delay.h>
#include <linux/list.h>
#include <watchdog.h>

#include "menu.h"

#define ansi 1

/*
 * Internally, each item in a menu is represented by a struct menu_item.
 *
 * These items will be alloc'd and initialized by menu_item_add and destroyed
 * by menu_item_destroy, and the consumer of the interface never sees that
 * this struct is used at all.
 */
struct menu_item {
	char *key;
	void *data;
	struct list_head list;
};

/*
 * The menu is composed of a list of items along with settings and callbacks
 * provided by the user. An incomplete definition of this struct is available
 * in menu.h, but the full definition is here to prevent consumers from
 * relying on its contents.
 */
struct menu {
	struct menu_item *default_item;
	ціле timeout;
	char *title;
	ціле prompt;
	void (*display_statusline)(struct menu *);
	void (*item_data_print)(void *);
	char *(*item_choice)(void *);
	bool (*need_reprint)(void *);
	void *item_choice_data;
	struct list_head items;
	ціле item_cnt;
};

/*
 * An iterator function for menu items. callback will be called for each item
 * in m, with m, a pointer to the item, and extra being passed to callback. If
 * callback returns a value other than NULL, iteration stops and the value
 * return by callback is returned from menu_items_iter.  This allows it to be
 * used for search type operations. It is also safe for callback to remove the
 * item from the list of items.
 */
static inline void *menu_items_iter(struct menu *m,
		void *(*callback)(struct menu *, struct menu_item *, void *),
		void *extra)
{
	struct list_head *pos, *n;
	struct menu_item *item;
	void *ret;

	list_for_each_safe(pos, n, &m->items) {
		item = list_entry(pos, struct menu_item, list);

		ret = callback(m, item, extra);

		якщо (ret)
			повернути ret;
	}

	повернути NULL;
}

/*
 * Print a menu_item. If the consumer provided an item_data_print function
 * when creating the menu, call it with a pointer to the item's private data.
 * Otherwise, print the key of the item.
 */
static inline void *menu_item_print(struct menu *m,
				struct menu_item *item,
				void *extra)
{
	якщо (!m->item_data_print) {
		puts(item->key);
		putc('\n');
	} інакше {
		m->item_data_print(item->data);
	}

	повернути NULL;
}

/*
 * Free the memory used by a menu item. This includes the memory used by its
 * key.
 */
static inline void *menu_item_destroy(struct menu *m,
				struct menu_item *item,
				void *extra)
{
	якщо (item->key)
		free(item->key);

	free(item);

	повернути NULL;
}

/*
 * Display a menu so the user can make a choice of an item. First display its
 * title, if any, and then each item in the menu.
 */
static inline void menu_display(struct menu *m)
{
	якщо (m->need_reprint) {
		якщо (!m->need_reprint(m->item_choice_data))
			повернути;
	}

	якщо (m->title) {
		puts(m->title);
		putc('\n');
	}
	якщо (m->display_statusline)
		m->display_statusline(m);

	menu_items_iter(m, menu_item_print, NULL);
}

/*
 * Check if an item's key matches a provided string, pointed to by extra. If
 * extra is NULL, an item with a NULL key will match. Otherwise, the item's
 * key has to match according to strcmp.
 *
 * This is called via menu_items_iter, so it returns a pointer to the item if
 * the key matches, and returns NULL otherwise.
 */
static inline void *menu_item_key_match(struct menu *m,
			struct menu_item *item, void *extra)
{
	char *item_key = extra;

	якщо (!item_key || !item->key) {
		якщо (item_key == item->key)
			повернути item;

		повернути NULL;
	}

	якщо (strcmp(item->key, item_key) == 0)
		повернути item;

	повернути NULL;
}

/*
 * Find the first item with a key matching item_key, if any exists.
 */
static inline struct menu_item *menu_item_by_key(struct menu *m,
							char *item_key)
{
	повернути menu_items_iter(m, menu_item_key_match, item_key);
}

/*
 * Set *choice to point to the default item's data, if any default item was
 * set, and returns 1. If no default item was set, returns -ENOENT.
 */
ціле menu_default_choice(struct menu *m, void **choice)
{
	якщо (m->default_item) {
		*choice = m->default_item->data;
		повернути 1;
	}

	повернути -ENOENT;
}

/*
 * Displays the menu and asks the user to choose an item. *choice will point
 * to the private data of the item the user chooses. The user makes a choice
 * by inputting a string matching the key of an item. Invalid choices will
 * cause the user to be prompted again, repeatedly, until the user makes a
 * valid choice. The user can exit the menu without making a choice via ^c.
 *
 * Returns 1 if the user made a choice, or -EINTR if they bail via ^c.
 */
static inline ціле menu_interactive_choice(struct menu *m, void **choice)
{
	char cbuf[CONFIG_SYS_CBSIZE];
	struct menu_item *choice_item = NULL;
	ціле readret;

	поки (!choice_item) {
		cbuf[0] = '\0';

		menu_display(m);

		якщо (!m->item_choice) {
			readret = cli_readline_into_buffer("Enter choice: ",
							   cbuf, m->timeout);

			якщо (readret >= 0) {
				choice_item = menu_item_by_key(m, cbuf);
				якщо (!choice_item)
					printf("%s not found\n", cbuf);
			} інакше якщо (readret == -1)  {
				printf("<INTERRUPT>\n");
				повернути -EINTR;
			} інакше {
				повернути menu_default_choice(m, choice);
			}
		} інакше {
			char *key = m->item_choice(m->item_choice_data);

			якщо (key)
				choice_item = menu_item_by_key(m, key);
		}

		якщо (!choice_item)
			m->timeout = 0;
	}

	*choice = choice_item->data;

	повернути 1;
}

/*
 * menu_default_set() - Sets the default choice for the menu. This is safe to
 * call more than once on a menu.
 *
 * m - Points to a menu created by menu_create().
 *
 * item_key - Points to a string that, when compared using strcmp, matches the
 * key for an existing item in the menu.
 *
 * Returns 1 if successful, -EINVAL if m is NULL, or -ENOENT if no item with a
 * key matching item_key is found.
 */
ціле menu_default_set(struct menu *m, char *item_key)
{
	struct menu_item *item;

	якщо (!m)
		повернути -EINVAL;

	item = menu_item_by_key(m, item_key);

	якщо (!item)
		повернути -ENOENT;

	m->default_item = item;

	повернути 1;
}

/*
 * menu_get_choice() - Returns the user's selected menu entry, or the default
 * if the menu is set to not prompt or the timeout expires. This is safe to
 * call more than once.
 *
 * m - Points to a menu created by menu_create().
 *
 * choice - Points to a location that will store a pointer to the selected
 * menu item. If no item is selected or there is an error, no value will be
 * written at the location it points to.
 *
 * Returns 1 if successful, -EINVAL if m or choice is NULL, -ENOENT if no
 * default has been set and the menu is set to not prompt or the timeout
 * expires, or -EINTR if the user exits the menu via ^c.
 */
ціле menu_get_choice(struct menu *m, void **choice)
{
	якщо (!m || !choice)
		повернути -EINVAL;

	якщо (!m->item_cnt)
		повернути -ENOENT;

	якщо (!m->prompt)
		повернути menu_default_choice(m, choice);

	повернути menu_interactive_choice(m, choice);
}

/*
 * menu_item_add() - Adds or replaces a menu item. Note that this replaces the
 * data of an item if it already exists, but doesn't change the order of the
 * item.
 *
 * m - Points to a menu created by menu_create().
 *
 * item_key - Points to a string that will uniquely identify the item.  The
 * string will be copied to internal storage, and is safe to discard after
 * passing to menu_item_add.
 *
 * item_data - An opaque pointer associated with an item. It is never
 * dereferenced internally, but will be passed to the item_data_print, and
 * will be returned from menu_get_choice if the menu item is selected.
 *
 * Returns 1 if successful, -EINVAL if m is NULL, or -ENOMEM if there is
 * insufficient memory to add the menu item.
 */
ціле menu_item_add(struct menu *m, char *item_key, void *item_data)
{
	struct menu_item *item;

	якщо (!m)
		повернути -EINVAL;

	item = menu_item_by_key(m, item_key);

	якщо (item) {
		item->data = item_data;
		повернути 1;
	}

	item = malloc(sizeof *item);
	якщо (!item)
		повернути -ENOMEM;

	item->key = strdup(item_key);

	якщо (!item->key) {
		free(item);
		повернути -ENOMEM;
	}

	item->data = item_data;

	list_add_tail(&item->list, &m->items);
	m->item_cnt++;

	повернути 1;
}

/*
 * menu_create() - Creates a menu handle with default settings
 *
 * title - If not NULL, points to a string that will be displayed before the
 * list of menu items. It will be copied to internal storage, and is safe to
 * discard after passing to menu_create().
 *
 * timeout - A delay in seconds to wait for user input. If 0, timeout is
 * disabled, and the default choice will be returned unless prompt is 1.
 *
 * prompt - If 0, don't ask for user input unless there is an interrupted
 * timeout. If 1, the user will be prompted for input regardless of the value
 * of timeout.
 *
 * display_statusline - If not NULL, will be called to show a statusline when
 * the menu is displayed.
 *
 * item_data_print - If not NULL, will be called for each item when the menu
 * is displayed, with the pointer to the item's data passed as the argument.
 * If NULL, each item's key will be printed instead.  Since an item's key is
 * what must be entered to select an item, the item_data_print function should
 * make it obvious what the key for each entry is.
 *
 * item_choice - If not NULL, will be called when asking the user to choose an
 * item. Returns a key string corresponding to the chosen item or NULL if
 * no item has been selected.
 *
 * need_reprint - If not NULL, will be called before printing the menu.
 * Returning FALSE means the menu does not need reprint.
 *
 * item_choice_data - Will be passed as the argument to the item_choice function
 *
 * Returns a pointer to the menu if successful, or NULL if there is
 * insufficient memory available to create the menu.
 */
struct menu *menu_create(char *title, ціле timeout, ціле prompt,
				void (*display_statusline)(struct menu *),
				void (*item_data_print)(void *),
				char *(*item_choice)(void *),
				bool (*need_reprint)(void *),
				void *item_choice_data)
{
	struct menu *m;

	m = malloc(sizeof *m);

	якщо (!m)
		повернути NULL;

	m->default_item = NULL;
	m->prompt = prompt;
	m->timeout = timeout;
	m->display_statusline = display_statusline;
	m->item_data_print = item_data_print;
	m->item_choice = item_choice;
	m->need_reprint = need_reprint;
	m->item_choice_data = item_choice_data;
	m->item_cnt = 0;

	якщо (title) {
		m->title = strdup(title);
		якщо (!m->title) {
			free(m);
			повернути NULL;
		}
	} інакше
		m->title = NULL;

	INIT_LIST_HEAD(&m->items);

	повернути m;
}

/*
 * menu_destroy() - frees the memory used by a menu and its items.
 *
 * m - Points to a menu created by menu_create().
 *
 * Returns 1 if successful, or -EINVAL if m is NULL.
 */
ціле menu_destroy(struct menu *m)
{
	якщо (!m)
		повернути -EINVAL;

	menu_items_iter(m, menu_item_destroy, NULL);

	якщо (m->title)
		free(m->title);

	free(m);

	повернути 1;
}

static ціле bootmenu_conv_shortcut_key(struct bootmenu_data *menu, ціле ichar)
{
	ціле shortcut_key;

	ichar = tolower(ichar);
	switch (ichar) {
	/* a-z for bootmenu entry > 9 */
	case 'a' ... 'z':
		shortcut_key = ichar - 'a' + 9;
		break;
	/* 1-9 for bootmenu entry <= 9 */
	case '1' ... '9':
		shortcut_key = ichar - '1';
		break;
	/* Reserve 0 for last option (aka Exit) */
	case '0':
	default:
		повернути -1;
	}

	повернути shortcut_key;
}

enum bootmenu_key bootmenu_autoboot_loop(struct bootmenu_data *menu,
					 struct cli_ch_state *cch)
{
	enum bootmenu_key key = BKEY_NONE;
	ціле i, c;

	поки (menu->delay > 0) {
		ціле ichar;

		якщо (ansi)
			printf(ANSI_CURSOR_POSITION, menu->count + 5, 3);
		printf("Hit any key to stop autoboot: %d ", menu->delay);
		для (i = 0; i < 100; ++i) {
			якщо (!tstc()) {
				schedule();
				mdelay(10);
				continue;
			}

			menu->delay = -1;
			c = getchar();

			ichar = cli_ch_process(cch, c);

			switch (ichar) {
			case '\0':
				key = BKEY_NONE;
				break;
			case '\n':
				key = BKEY_SELECT;
				break;
			case 0x3: /* ^C */
				key = BKEY_QUIT;
				break;
			case 'A' ... 'Z':
			case 'a' ... 'z':
			case '0' ... '9':
				key = BKEY_SHORTCUT;
				break;
			default:
				key = BKEY_NONE;
				break;
			}
			break;
		}

		якщо (key == BKEY_SHORTCUT)
			cch->shortcut_key = bootmenu_conv_shortcut_key(menu, ichar);

		якщо (menu->delay < 0)
			break;

		--menu->delay;
	}

	якщо (ansi)
		printf(ANSI_CURSOR_POSITION ANSI_CLEAR_LINE, menu->count + 5, 1);

	якщо (menu->delay == 0)
		key = BKEY_SELECT;

	повернути key;
}

enum bootmenu_key bootmenu_conv_key(ціле ichar)
{
	enum bootmenu_key key;

	switch (ichar) {
	case '\n':
		/* enter key was pressed */
		key = BKEY_SELECT;
		break;
	case CTL_CH('c'):
	case '\e':
		/* ^C was pressed */
		key = BKEY_QUIT;
		break;
	case CTL_CH('p'):
		key = BKEY_UP;
		break;
	case CTL_CH('n'):
		key = BKEY_DOWN;
		break;
	case CTL_CH('s'):
		key = BKEY_SAVE;
		break;
	case '+':
		key = BKEY_PLUS;
		break;
	case '-':
		key = BKEY_MINUS;
		break;
	case ' ':
		key = BKEY_SPACE;
		break;
	default:
		key = BKEY_NONE;
		break;
	}

	повернути key;
}

enum bootmenu_key bootmenu_loop(struct bootmenu_data *menu,
				struct cli_ch_state *cch)
{
	enum bootmenu_key key;
	ціле c, errchar = 0;

	c = cli_ch_process(cch, 0);
	якщо (!c) {
		поки (!c && !tstc()) {
			schedule();
			mdelay(10);
			c = cli_ch_process(cch, errchar);
			errchar = -ETIMEDOUT;
		}
		якщо (!c) {
			c = getchar();
			c = cli_ch_process(cch, c);
		}
	}

	key = bootmenu_conv_key(c);

	якщо (key == BKEY_NONE && isalnum(c)) {
		key = BKEY_SHORTCUT;
		cch->shortcut_key = bootmenu_conv_shortcut_key(menu, c);
	}

	повернути key;
}
