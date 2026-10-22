// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2010-2011 Calxeda, Inc.
 * Copyright (c) 2019, NVIDIA CORPORATION. All rights reserved.
 */

#ضمن <ansi.h>
#ضمن <cli.h>
#ضمن <malloc.h>
#ضمن <errno.h>
#ضمن <linux/ctype.h>
#ضمن <linux/delay.h>
#ضمن <linux/list.h>
#ضمن <watchdog.h>

#ضمن "menu.h"

#define ansi 1

/*
 * Internally, each item in a menu is represented by a struct menu_item.
 *
 * These items will be alloc'd and initialized by menu_item_add and destroyed
 * by menu_item_destroy, and the consumer of the interface never sees that
 * this struct is used at all.
 */
بنية menu_item {
	حرف *key;
	فراغ *data;
	بنية list_head list;
};

/*
 * The menu is composed of a list of items along with settings and callbacks
 * provided by the user. An incomplete definition of this struct is available
 * in menu.h, but the full definition is here to prevent consumers from
 * relying on its contents.
 */
بنية menu {
	بنية menu_item *default_item;
	صحيح timeout;
	حرف *title;
	صحيح prompt;
	فراغ (*display_statusline)(بنية menu *);
	فراغ (*item_data_print)(فراغ *);
	حرف *(*item_choice)(فراغ *);
	bool (*need_reprint)(فراغ *);
	فراغ *item_choice_data;
	بنية list_head items;
	صحيح item_cnt;
};

/*
 * An iterator function for menu items. callback will be called for each item
 * in m, with m, a pointer to the item, and extra being passed to callback. If
 * callback returns a value other than NULL, iteration stops and the value
 * return by callback is returned from menu_items_iter.  This allows it to be
 * used for search type operations. It is also safe for callback to remove the
 * item from the list of items.
 */
ساكن مضمن فراغ *menu_items_iter(بنية menu *m,
		فراغ *(*callback)(بنية menu *, بنية menu_item *, فراغ *),
		فراغ *extra)
{
	بنية list_head *pos, *n;
	بنية menu_item *item;
	فراغ *ret;

	list_for_each_safe(pos, n, &m->items) {
		item = list_entry(pos, بنية menu_item, list);

		ret = callback(m, item, extra);

		𐡄𐡍 (ret)
			أرجع ret;
	}

	أرجع عدم;
}

/*
 * Print a menu_item. If the consumer provided an item_data_print function
 * when creating the menu, call it with a pointer to the item's private data.
 * Otherwise, print the key of the item.
 */
ساكن مضمن فراغ *menu_item_print(بنية menu *m,
				بنية menu_item *item,
				فراغ *extra)
{
	𐡄𐡍 (!m->item_data_print) {
		puts(item->key);
		putc('\n');
	} وإلا {
		m->item_data_print(item->data);
	}

	أرجع عدم;
}

/*
 * Free the memory used by a menu item. This includes the memory used by its
 * key.
 */
ساكن مضمن فراغ *menu_item_destroy(بنية menu *m,
				بنية menu_item *item,
				فراغ *extra)
{
	𐡄𐡍 (item->key)
		حرر(item->key);

	حرر(item);

	أرجع عدم;
}

/*
 * Display a menu so the user can make a choice of an item. First display its
 * title, if any, and then each item in the menu.
 */
ساكن مضمن فراغ menu_display(بنية menu *m)
{
	𐡄𐡍 (m->need_reprint) {
		𐡄𐡍 (!m->need_reprint(m->item_choice_data))
			أرجع;
	}

	𐡄𐡍 (m->title) {
		puts(m->title);
		putc('\n');
	}
	𐡄𐡍 (m->display_statusline)
		m->display_statusline(m);

	menu_items_iter(m, menu_item_print, عدم);
}

/*
 * Check if an item's key matches a provided string, pointed to by extra. If
 * extra is NULL, an item with a NULL key will match. Otherwise, the item's
 * key has to match according to strcmp.
 *
 * This is called via menu_items_iter, so it returns a pointer to the item if
 * the key matches, and returns NULL otherwise.
 */
ساكن مضمن فراغ *menu_item_key_match(بنية menu *m,
			بنية menu_item *item, فراغ *extra)
{
	حرف *item_key = extra;

	𐡄𐡍 (!item_key || !item->key) {
		𐡄𐡍 (item_key == item->key)
			أرجع item;

		أرجع عدم;
	}

	𐡄𐡍 (strcmp(item->key, item_key) == 0)
		أرجع item;

	أرجع عدم;
}

/*
 * Find the first item with a key matching item_key, if any exists.
 */
ساكن مضمن بنية menu_item *menu_item_by_key(بنية menu *m,
							حرف *item_key)
{
	أرجع menu_items_iter(m, menu_item_key_match, item_key);
}

/*
 * Set *choice to point to the default item's data, if any default item was
 * set, and returns 1. If no default item was set, returns -ENOENT.
 */
صحيح menu_default_choice(بنية menu *m, فراغ **choice)
{
	𐡄𐡍 (m->default_item) {
		*choice = m->default_item->data;
		أرجع 1;
	}

	أرجع -ENOENT;
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
ساكن مضمن صحيح menu_interactive_choice(بنية menu *m, فراغ **choice)
{
	حرف cbuf[CONFIG_SYS_CBSIZE];
	بنية menu_item *choice_item = عدم;
	صحيح readret;

	طالما (!choice_item) {
		cbuf[0] = '\0';

		menu_display(m);

		𐡄𐡍 (!m->item_choice) {
			readret = cli_readline_into_buffer("Enter choice: ",
							   cbuf, m->timeout);

			𐡄𐡍 (readret >= 0) {
				choice_item = menu_item_by_key(m, cbuf);
				𐡄𐡍 (!choice_item)
					printf("%s not found\n", cbuf);
			} وإلا 𐡄𐡍 (readret == -1)  {
				printf("<INTERRUPT>\n");
				أرجع -EINTR;
			} وإلا {
				أرجع menu_default_choice(m, choice);
			}
		} وإلا {
			حرف *key = m->item_choice(m->item_choice_data);

			𐡄𐡍 (key)
				choice_item = menu_item_by_key(m, key);
		}

		𐡄𐡍 (!choice_item)
			m->timeout = 0;
	}

	*choice = choice_item->data;

	أرجع 1;
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
صحيح menu_default_set(بنية menu *m, حرف *item_key)
{
	بنية menu_item *item;

	𐡄𐡍 (!m)
		أرجع -EINVAL;

	item = menu_item_by_key(m, item_key);

	𐡄𐡍 (!item)
		أرجع -ENOENT;

	m->default_item = item;

	أرجع 1;
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
صحيح menu_get_choice(بنية menu *m, فراغ **choice)
{
	𐡄𐡍 (!m || !choice)
		أرجع -EINVAL;

	𐡄𐡍 (!m->item_cnt)
		أرجع -ENOENT;

	𐡄𐡍 (!m->prompt)
		أرجع menu_default_choice(m, choice);

	أرجع menu_interactive_choice(m, choice);
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
صحيح menu_item_add(بنية menu *m, حرف *item_key, فراغ *item_data)
{
	بنية menu_item *item;

	𐡄𐡍 (!m)
		أرجع -EINVAL;

	item = menu_item_by_key(m, item_key);

	𐡄𐡍 (item) {
		item->data = item_data;
		أرجع 1;
	}

	item = malloc(حجم *item);
	𐡄𐡍 (!item)
		أرجع -ENOMEM;

	item->key = strdup(item_key);

	𐡄𐡍 (!item->key) {
		حرر(item);
		أرجع -ENOMEM;
	}

	item->data = item_data;

	list_add_tail(&item->list, &m->items);
	m->item_cnt++;

	أرجع 1;
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
بنية menu *menu_create(حرف *title, صحيح timeout, صحيح prompt,
				فراغ (*display_statusline)(بنية menu *),
				فراغ (*item_data_print)(فراغ *),
				حرف *(*item_choice)(فراغ *),
				bool (*need_reprint)(فراغ *),
				فراغ *item_choice_data)
{
	بنية menu *m;

	m = malloc(حجم *m);

	𐡄𐡍 (!m)
		أرجع عدم;

	m->default_item = عدم;
	m->prompt = prompt;
	m->timeout = timeout;
	m->display_statusline = display_statusline;
	m->item_data_print = item_data_print;
	m->item_choice = item_choice;
	m->need_reprint = need_reprint;
	m->item_choice_data = item_choice_data;
	m->item_cnt = 0;

	𐡄𐡍 (title) {
		m->title = strdup(title);
		𐡄𐡍 (!m->title) {
			حرر(m);
			أرجع عدم;
		}
	} وإلا
		m->title = عدم;

	INIT_LIST_HEAD(&m->items);

	أرجع m;
}

/*
 * menu_destroy() - frees the memory used by a menu and its items.
 *
 * m - Points to a menu created by menu_create().
 *
 * Returns 1 if successful, or -EINVAL if m is NULL.
 */
صحيح menu_destroy(بنية menu *m)
{
	𐡄𐡍 (!m)
		أرجع -EINVAL;

	menu_items_iter(m, menu_item_destroy, عدم);

	𐡄𐡍 (m->title)
		حرر(m->title);

	حرر(m);

	أرجع 1;
}

ساكن صحيح bootmenu_conv_shortcut_key(بنية bootmenu_data *menu, صحيح ichar)
{
	صحيح shortcut_key;

	ichar = tolower(ichar);
	بدل (ichar) {
	/* a-z for bootmenu entry > 9 */
	حالة 'a' ... 'z':
		shortcut_key = ichar - 'a' + 9;
		اكسر;
	/* 1-9 for bootmenu entry <= 9 */
	حالة '1' ... '9':
		shortcut_key = ichar - '1';
		اكسر;
	/* Reserve 0 for last option (aka Exit) */
	حالة '0':
	افتراضي:
		أرجع -1;
	}

	أرجع shortcut_key;
}

تعداد bootmenu_key bootmenu_autoboot_loop(بنية bootmenu_data *menu,
					 بنية cli_ch_state *cch)
{
	تعداد bootmenu_key key = BKEY_NONE;
	صحيح i, c;

	طالما (menu->delay > 0) {
		صحيح ichar;

		𐡄𐡍 (ansi)
			printf(ANSI_CURSOR_POSITION, menu->count + 5, 3);
		printf("Hit any key to stop autoboot: %d ", menu->delay);
		لكل (i = 0; i < 100; ++i) {
			𐡄𐡍 (!tstc()) {
				schedule();
				mdelay(10);
				استمر;
			}

			menu->delay = -1;
			c = getchar();

			ichar = cli_ch_process(cch, c);

			بدل (ichar) {
			حالة '\0':
				key = BKEY_NONE;
				اكسر;
			حالة '\n':
				key = BKEY_SELECT;
				اكسر;
			حالة 0x3: /* ^C */
				key = BKEY_QUIT;
				اكسر;
			حالة 'A' ... 'Z':
			حالة 'a' ... 'z':
			حالة '0' ... '9':
				key = BKEY_SHORTCUT;
				اكسر;
			افتراضي:
				key = BKEY_NONE;
				اكسر;
			}
			اكسر;
		}

		𐡄𐡍 (key == BKEY_SHORTCUT)
			cch->shortcut_key = bootmenu_conv_shortcut_key(menu, ichar);

		𐡄𐡍 (menu->delay < 0)
			اكسر;

		--menu->delay;
	}

	𐡄𐡍 (ansi)
		printf(ANSI_CURSOR_POSITION ANSI_CLEAR_LINE, menu->count + 5, 1);

	𐡄𐡍 (menu->delay == 0)
		key = BKEY_SELECT;

	أرجع key;
}

تعداد bootmenu_key bootmenu_conv_key(صحيح ichar)
{
	تعداد bootmenu_key key;

	بدل (ichar) {
	حالة '\n':
		/* enter key was pressed */
		key = BKEY_SELECT;
		اكسر;
	حالة CTL_CH('c'):
	حالة '\e':
		/* ^C was pressed */
		key = BKEY_QUIT;
		اكسر;
	حالة CTL_CH('p'):
		key = BKEY_UP;
		اكسر;
	حالة CTL_CH('n'):
		key = BKEY_DOWN;
		اكسر;
	حالة CTL_CH('s'):
		key = BKEY_SAVE;
		اكسر;
	حالة '+':
		key = BKEY_PLUS;
		اكسر;
	حالة '-':
		key = BKEY_MINUS;
		اكسر;
	حالة ' ':
		key = BKEY_SPACE;
		اكسر;
	افتراضي:
		key = BKEY_NONE;
		اكسر;
	}

	أرجع key;
}

تعداد bootmenu_key bootmenu_loop(بنية bootmenu_data *menu,
				بنية cli_ch_state *cch)
{
	تعداد bootmenu_key key;
	صحيح c, errchar = 0;

	c = cli_ch_process(cch, 0);
	𐡄𐡍 (!c) {
		طالما (!c && !tstc()) {
			schedule();
			mdelay(10);
			c = cli_ch_process(cch, errchar);
			errchar = -ETIMEDOUT;
		}
		𐡄𐡍 (!c) {
			c = getchar();
			c = cli_ch_process(cch, c);
		}
	}

	key = bootmenu_conv_key(c);

	𐡄𐡍 (key == BKEY_NONE && isalnum(c)) {
		key = BKEY_SHORTCUT;
		cch->shortcut_key = bootmenu_conv_shortcut_key(menu, c);
	}

	أرجع key;
}
