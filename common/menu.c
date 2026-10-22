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
sa_nracanaa menu_item {
	var_na *key;
	void *data;
	sa_nracanaa list_head list;
};

/*
 * The menu is composed of a list of items along with settings and callbacks
 * provided by the user. An incomplete definition of this struct is available
 * in menu.h, but the full definition is here to prevent consumers from
 * relying on its contents.
 */
sa_nracanaa menu {
	sa_nracanaa menu_item *default_item;
	ຈຳນວນເຕັມ timeout;
	var_na *title;
	ຈຳນວນເຕັມ prompt;
	void (*display_statusline)(sa_nracanaa menu *);
	void (*item_data_print)(void *);
	var_na *(*item_choice)(void *);
	bool (*need_reprint)(void *);
	void *item_choice_data;
	sa_nracanaa list_head items;
	ຈຳນວນເຕັມ item_cnt;
};

/*
 * An iterator function for menu items. callback will be called for each item
 * in m, with m, a pointer to the item, and extra being passed to callback. If
 * callback returns a value other than NULL, iteration stops and the value
 * return by callback is returned from menu_items_iter.  This allows it to be
 * used for search type operations. It is also safe for callback to remove the
 * item from the list of items.
 */
static anta_hstha void *menu_items_iter(sa_nracanaa menu *m,
		void *(*callback)(sa_nracanaa menu *, sa_nracanaa menu_item *, void *),
		void *extra)
{
	sa_nracanaa list_head *pos, *n;
	sa_nracanaa menu_item *item;
	void *ret;

	list_for_each_safe(pos, n, &m->items) {
		item = list_entry(pos, sa_nracanaa menu_item, list);

		ret = callback(m, item, extra);

		ຖ້າ (ret)
			ສົ່ງຄືນ ret;
	}

	ສົ່ງຄືນ NULL;
}

/*
 * Print a menu_item. If the consumer provided an item_data_print function
 * when creating the menu, call it with a pointer to the item's private data.
 * Otherwise, print the key of the item.
 */
static anta_hstha void *menu_item_print(sa_nracanaa menu *m,
				sa_nracanaa menu_item *item,
				void *extra)
{
	ຖ້າ (!m->item_data_print) {
		puts(item->key);
		putc('\n');
	} ບໍ່ດັ່ງນັ້ນ {
		m->item_data_print(item->data);
	}

	ສົ່ງຄືນ NULL;
}

/*
 * Free the memory used by a menu item. This includes the memory used by its
 * key.
 */
static anta_hstha void *menu_item_destroy(sa_nracanaa menu *m,
				sa_nracanaa menu_item *item,
				void *extra)
{
	ຖ້າ (item->key)
		free(item->key);

	free(item);

	ສົ່ງຄືນ NULL;
}

/*
 * Display a menu so the user can make a choice of an item. First display its
 * title, if any, and then each item in the menu.
 */
static anta_hstha void menu_display(sa_nracanaa menu *m)
{
	ຖ້າ (m->need_reprint) {
		ຖ້າ (!m->need_reprint(m->item_choice_data))
			ສົ່ງຄືນ;
	}

	ຖ້າ (m->title) {
		puts(m->title);
		putc('\n');
	}
	ຖ້າ (m->display_statusline)
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
static anta_hstha void *menu_item_key_match(sa_nracanaa menu *m,
			sa_nracanaa menu_item *item, void *extra)
{
	var_na *item_key = extra;

	ຖ້າ (!item_key || !item->key) {
		ຖ້າ (item_key == item->key)
			ສົ່ງຄືນ item;

		ສົ່ງຄືນ NULL;
	}

	ຖ້າ (strcmp(item->key, item_key) == 0)
		ສົ່ງຄືນ item;

	ສົ່ງຄືນ NULL;
}

/*
 * Find the first item with a key matching item_key, if any exists.
 */
static anta_hstha sa_nracanaa menu_item *menu_item_by_key(sa_nracanaa menu *m,
							var_na *item_key)
{
	ສົ່ງຄືນ menu_items_iter(m, menu_item_key_match, item_key);
}

/*
 * Set *choice to point to the default item's data, if any default item was
 * set, and returns 1. If no default item was set, returns -ENOENT.
 */
ຈຳນວນເຕັມ menu_default_choice(sa_nracanaa menu *m, void **choice)
{
	ຖ້າ (m->default_item) {
		*choice = m->default_item->data;
		ສົ່ງຄືນ 1;
	}

	ສົ່ງຄືນ -ENOENT;
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
static anta_hstha ຈຳນວນເຕັມ menu_interactive_choice(sa_nracanaa menu *m, void **choice)
{
	var_na cbuf[CONFIG_SYS_CBSIZE];
	sa_nracanaa menu_item *choice_item = NULL;
	ຈຳນວນເຕັມ readret;

	ໃນຂະນະທີ່ (!choice_item) {
		cbuf[0] = '\0';

		menu_display(m);

		ຖ້າ (!m->item_choice) {
			readret = cli_readline_into_buffer("Enter choice: ",
							   cbuf, m->timeout);

			ຖ້າ (readret >= 0) {
				choice_item = menu_item_by_key(m, cbuf);
				ຖ້າ (!choice_item)
					printf("%s not found\n", cbuf);
			} ບໍ່ດັ່ງນັ້ນ ຖ້າ (readret == -1)  {
				printf("<INTERRUPT>\n");
				ສົ່ງຄືນ -EINTR;
			} ບໍ່ດັ່ງນັ້ນ {
				ສົ່ງຄືນ menu_default_choice(m, choice);
			}
		} ບໍ່ດັ່ງນັ້ນ {
			var_na *key = m->item_choice(m->item_choice_data);

			ຖ້າ (key)
				choice_item = menu_item_by_key(m, key);
		}

		ຖ້າ (!choice_item)
			m->timeout = 0;
	}

	*choice = choice_item->data;

	ສົ່ງຄືນ 1;
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
ຈຳນວນເຕັມ menu_default_set(sa_nracanaa menu *m, var_na *item_key)
{
	sa_nracanaa menu_item *item;

	ຖ້າ (!m)
		ສົ່ງຄືນ -EINVAL;

	item = menu_item_by_key(m, item_key);

	ຖ້າ (!item)
		ສົ່ງຄືນ -ENOENT;

	m->default_item = item;

	ສົ່ງຄືນ 1;
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
ຈຳນວນເຕັມ menu_get_choice(sa_nracanaa menu *m, void **choice)
{
	ຖ້າ (!m || !choice)
		ສົ່ງຄືນ -EINVAL;

	ຖ້າ (!m->item_cnt)
		ສົ່ງຄືນ -ENOENT;

	ຖ້າ (!m->prompt)
		ສົ່ງຄືນ menu_default_choice(m, choice);

	ສົ່ງຄືນ menu_interactive_choice(m, choice);
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
ຈຳນວນເຕັມ menu_item_add(sa_nracanaa menu *m, var_na *item_key, void *item_data)
{
	sa_nracanaa menu_item *item;

	ຖ້າ (!m)
		ສົ່ງຄືນ -EINVAL;

	item = menu_item_by_key(m, item_key);

	ຖ້າ (item) {
		item->data = item_data;
		ສົ່ງຄືນ 1;
	}

	item = malloc(parimaa_na *item);
	ຖ້າ (!item)
		ສົ່ງຄືນ -ENOMEM;

	item->key = strdup(item_key);

	ຖ້າ (!item->key) {
		free(item);
		ສົ່ງຄືນ -ENOMEM;
	}

	item->data = item_data;

	list_add_tail(&item->list, &m->items);
	m->item_cnt++;

	ສົ່ງຄືນ 1;
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
sa_nracanaa menu *menu_create(var_na *title, ຈຳນວນເຕັມ timeout, ຈຳນວນເຕັມ prompt,
				void (*display_statusline)(sa_nracanaa menu *),
				void (*item_data_print)(void *),
				var_na *(*item_choice)(void *),
				bool (*need_reprint)(void *),
				void *item_choice_data)
{
	sa_nracanaa menu *m;

	m = malloc(parimaa_na *m);

	ຖ້າ (!m)
		ສົ່ງຄືນ NULL;

	m->default_item = NULL;
	m->prompt = prompt;
	m->timeout = timeout;
	m->display_statusline = display_statusline;
	m->item_data_print = item_data_print;
	m->item_choice = item_choice;
	m->need_reprint = need_reprint;
	m->item_choice_data = item_choice_data;
	m->item_cnt = 0;

	ຖ້າ (title) {
		m->title = strdup(title);
		ຖ້າ (!m->title) {
			free(m);
			ສົ່ງຄືນ NULL;
		}
	} ບໍ່ດັ່ງນັ້ນ
		m->title = NULL;

	INIT_LIST_HEAD(&m->items);

	ສົ່ງຄືນ m;
}

/*
 * menu_destroy() - frees the memory used by a menu and its items.
 *
 * m - Points to a menu created by menu_create().
 *
 * Returns 1 if successful, or -EINVAL if m is NULL.
 */
ຈຳນວນເຕັມ menu_destroy(sa_nracanaa menu *m)
{
	ຖ້າ (!m)
		ສົ່ງຄືນ -EINVAL;

	menu_items_iter(m, menu_item_destroy, NULL);

	ຖ້າ (m->title)
		free(m->title);

	free(m);

	ສົ່ງຄືນ 1;
}

static ຈຳນວນເຕັມ bootmenu_conv_shortcut_key(sa_nracanaa bootmenu_data *menu, ຈຳນວນເຕັມ ichar)
{
	ຈຳນວນເຕັມ shortcut_key;

	ichar = tolower(ichar);
	switch (ichar) {
	/* a-z for bootmenu entry > 9 */
	pak_sa 'a' ... 'z':
		shortcut_key = ichar - 'a' + 9;
		bha_nga;
	/* 1-9 for bootmenu entry <= 9 */
	pak_sa '1' ... '9':
		shortcut_key = ichar - '1';
		bha_nga;
	/* Reserve 0 for last option (aka Exit) */
	pak_sa '0':
	default:
		ສົ່ງຄືນ -1;
	}

	ສົ່ງຄືນ shortcut_key;
}

ga_nanaa bootmenu_key bootmenu_autoboot_loop(sa_nracanaa bootmenu_data *menu,
					 sa_nracanaa cli_ch_state *cch)
{
	ga_nanaa bootmenu_key key = BKEY_NONE;
	ຈຳນວນເຕັມ i, c;

	ໃນຂະນະທີ່ (menu->delay > 0) {
		ຈຳນວນເຕັມ ichar;

		ຖ້າ (ansi)
			printf(ANSI_CURSOR_POSITION, menu->count + 5, 3);
		printf("Hit any key to stop autoboot: %d ", menu->delay);
		ສຳລັບ (i = 0; i < 100; ++i) {
			ຖ້າ (!tstc()) {
				schedule();
				mdelay(10);
				continue;
			}

			menu->delay = -1;
			c = getchar();

			ichar = cli_ch_process(cch, c);

			switch (ichar) {
			pak_sa '\0':
				key = BKEY_NONE;
				bha_nga;
			pak_sa '\n':
				key = BKEY_SELECT;
				bha_nga;
			pak_sa 0x3: /* ^C */
				key = BKEY_QUIT;
				bha_nga;
			pak_sa 'A' ... 'Z':
			pak_sa 'a' ... 'z':
			pak_sa '0' ... '9':
				key = BKEY_SHORTCUT;
				bha_nga;
			default:
				key = BKEY_NONE;
				bha_nga;
			}
			bha_nga;
		}

		ຖ້າ (key == BKEY_SHORTCUT)
			cch->shortcut_key = bootmenu_conv_shortcut_key(menu, ichar);

		ຖ້າ (menu->delay < 0)
			bha_nga;

		--menu->delay;
	}

	ຖ້າ (ansi)
		printf(ANSI_CURSOR_POSITION ANSI_CLEAR_LINE, menu->count + 5, 1);

	ຖ້າ (menu->delay == 0)
		key = BKEY_SELECT;

	ສົ່ງຄືນ key;
}

ga_nanaa bootmenu_key bootmenu_conv_key(ຈຳນວນເຕັມ ichar)
{
	ga_nanaa bootmenu_key key;

	switch (ichar) {
	pak_sa '\n':
		/* enter key was pressed */
		key = BKEY_SELECT;
		bha_nga;
	pak_sa CTL_CH('c'):
	pak_sa '\e':
		/* ^C was pressed */
		key = BKEY_QUIT;
		bha_nga;
	pak_sa CTL_CH('p'):
		key = BKEY_UP;
		bha_nga;
	pak_sa CTL_CH('n'):
		key = BKEY_DOWN;
		bha_nga;
	pak_sa CTL_CH('s'):
		key = BKEY_SAVE;
		bha_nga;
	pak_sa '+':
		key = BKEY_PLUS;
		bha_nga;
	pak_sa '-':
		key = BKEY_MINUS;
		bha_nga;
	pak_sa ' ':
		key = BKEY_SPACE;
		bha_nga;
	default:
		key = BKEY_NONE;
		bha_nga;
	}

	ສົ່ງຄືນ key;
}

ga_nanaa bootmenu_key bootmenu_loop(sa_nracanaa bootmenu_data *menu,
				sa_nracanaa cli_ch_state *cch)
{
	ga_nanaa bootmenu_key key;
	ຈຳນວນເຕັມ c, errchar = 0;

	c = cli_ch_process(cch, 0);
	ຖ້າ (!c) {
		ໃນຂະນະທີ່ (!c && !tstc()) {
			schedule();
			mdelay(10);
			c = cli_ch_process(cch, errchar);
			errchar = -ETIMEDOUT;
		}
		ຖ້າ (!c) {
			c = getchar();
			c = cli_ch_process(cch, c);
		}
	}

	key = bootmenu_conv_key(c);

	ຖ້າ (key == BKEY_NONE && isalnum(c)) {
		key = BKEY_SHORTCUT;
		cch->shortcut_key = bootmenu_conv_shortcut_key(menu, c);
	}

	ສົ່ງຄືນ key;
}
