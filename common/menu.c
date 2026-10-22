// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2010-2011 Calxeda, Inc.
 * Copyright (c) 2019, NVIDIA CORPORATION. All rights reserved.
 */

#ਅਨ੍ਤਰ੍ਭਾਵ <ansi.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <cli.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <malloc.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <errno.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <linux/ctype.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <linux/delay.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <linux/list.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <watchdog.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "menu.h"

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ansi 1

/*
 * Internally, each item in a menu is represented by a struct menu_item.
 *
 * These items will be alloc'd and initialized by menu_item_add and destroyed
 * by menu_item_destroy, and the consumer of the interface never sees that
 * this struct is used at all.
 */
ਬਣਤਰ menu_ਵਸਤੂ {
	ਅੱਖਰ *ਕੁੰਜੀ;
	ਖਾਲੀ *ਅੰਕੜੇ;
	ਬਣਤਰ list_ਸਿਰ list;
};

/*
 * The menu is composed of a list of items along with settings and callbacks
 * provided by the user. An incomplete definition of this struct is available
 * in menu.h, but the full definition is here to prevent consumers from
 * relying on its contents.
 */
ਬਣਤਰ menu {
	ਬਣਤਰ menu_ਵਸਤੂ *default_ਵਸਤੂ;
	ਪੂਰਨ timeout;
	ਅੱਖਰ *title;
	ਪੂਰਨ prompt;
	ਖਾਲੀ (*display_statusline)(ਬਣਤਰ menu *);
	ਖਾਲੀ (*ਵਸਤੂ_ਅੰਕੜੇ_print)(ਖਾਲੀ *);
	ਅੱਖਰ *(*ਵਸਤੂ_choice)(ਖਾਲੀ *);
	bool (*need_reprint)(ਖਾਲੀ *);
	ਖਾਲੀ *ਵਸਤੂ_choice_ਅੰਕੜੇ;
	ਬਣਤਰ list_ਸਿਰ ਚੀਜ਼ਾਂ;
	ਪੂਰਨ ਵਸਤੂ_cnt;
};

/*
 * An iterator function for menu items. callback will be called for each item
 * in m, with m, a pointer to the item, and extra being passed to callback. If
 * callback returns a value other than NULL, iteration stops and the value
 * return by callback is returned from menu_items_iter.  This allows it to be
 * used for search type operations. It is also safe for callback to remove the
 * item from the list of items.
 */
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ *menu_ਚੀਜ਼ਾਂ_iter(ਬਣਤਰ menu *m,
		ਖਾਲੀ *(*callback)(ਬਣਤਰ menu *, ਬਣਤਰ menu_ਵਸਤੂ *, ਖਾਲੀ *),
		ਖਾਲੀ *extra)
{
	ਬਣਤਰ list_ਸਿਰ *pos, *n;
	ਬਣਤਰ menu_ਵਸਤੂ *ਵਸਤੂ;
	ਖਾਲੀ *ret;

	list_for_each_ਸੁਰੱਖਿਅਤ(pos, n, &m->ਚੀਜ਼ਾਂ) {
		ਵਸਤੂ = list_ਇੰਦਰਾਜ(pos, ਬਣਤਰ menu_ਵਸਤੂ, list);

		ret = callback(m, ਵਸਤੂ, extra);

		ਜੇ (ret)
			ਵਾਪਸ ret;
	}

	ਵਾਪਸ ਨਿਰਾ;
}

/*
 * Print a menu_item. If the consumer provided an item_data_print function
 * when creating the menu, call it with a pointer to the item's private data.
 * Otherwise, print the key of the item.
 */
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ *menu_ਵਸਤੂ_print(ਬਣਤਰ menu *m,
				ਬਣਤਰ menu_ਵਸਤੂ *ਵਸਤੂ,
				ਖਾਲੀ *extra)
{
	ਜੇ (!m->ਵਸਤੂ_ਅੰਕੜੇ_print) {
		puts(ਵਸਤੂ->ਕੁੰਜੀ);
		putc('\n');
	} ਨਹੀਂਤਾਂ {
		m->ਵਸਤੂ_ਅੰਕੜੇ_print(ਵਸਤੂ->ਅੰਕੜੇ);
	}

	ਵਾਪਸ ਨਿਰਾ;
}

/*
 * Free the memory used by a menu item. This includes the memory used by its
 * key.
 */
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ *menu_ਵਸਤੂ_destroy(ਬਣਤਰ menu *m,
				ਬਣਤਰ menu_ਵਸਤੂ *ਵਸਤੂ,
				ਖਾਲੀ *extra)
{
	ਜੇ (ਵਸਤੂ->ਕੁੰਜੀ)
		ਮੁਕਤ(ਵਸਤੂ->ਕੁੰਜੀ);

	ਮੁਕਤ(ਵਸਤੂ);

	ਵਾਪਸ ਨਿਰਾ;
}

/*
 * Display a menu so the user can make a choice of an item. First display its
 * title, if any, and then each item in the menu.
 */
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ menu_display(ਬਣਤਰ menu *m)
{
	ਜੇ (m->need_reprint) {
		ਜੇ (!m->need_reprint(m->ਵਸਤੂ_choice_ਅੰਕੜੇ))
			ਵਾਪਸ;
	}

	ਜੇ (m->title) {
		puts(m->title);
		putc('\n');
	}
	ਜੇ (m->display_statusline)
		m->display_statusline(m);

	menu_ਚੀਜ਼ਾਂ_iter(m, menu_ਵਸਤੂ_print, ਨਿਰਾ);
}

/*
 * Check if an item's key matches a provided string, pointed to by extra. If
 * extra is NULL, an item with a NULL key will match. Otherwise, the item's
 * key has to match according to strcmp.
 *
 * This is called via menu_items_iter, so it returns a pointer to the item if
 * the key matches, and returns NULL otherwise.
 */
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ *menu_ਵਸਤੂ_ਕੁੰਜੀ_ਮੇਲ(ਬਣਤਰ menu *m,
			ਬਣਤਰ menu_ਵਸਤੂ *ਵਸਤੂ, ਖਾਲੀ *extra)
{
	ਅੱਖਰ *ਵਸਤੂ_ਕੁੰਜੀ = extra;

	ਜੇ (!ਵਸਤੂ_ਕੁੰਜੀ || !ਵਸਤੂ->ਕੁੰਜੀ) {
		ਜੇ (ਵਸਤੂ_ਕੁੰਜੀ == ਵਸਤੂ->ਕੁੰਜੀ)
			ਵਾਪਸ ਵਸਤੂ;

		ਵਾਪਸ ਨਿਰਾ;
	}

	ਜੇ (strcmp(ਵਸਤੂ->ਕੁੰਜੀ, ਵਸਤੂ_ਕੁੰਜੀ) == 0)
		ਵਾਪਸ ਵਸਤੂ;

	ਵਾਪਸ ਨਿਰਾ;
}

/*
 * Find the first item with a key matching item_key, if any exists.
 */
ਸਥਿਰ ਸਤਰਅੰਦਰ ਬਣਤਰ menu_ਵਸਤੂ *menu_ਵਸਤੂ_by_ਕੁੰਜੀ(ਬਣਤਰ menu *m,
							ਅੱਖਰ *ਵਸਤੂ_ਕੁੰਜੀ)
{
	ਵਾਪਸ menu_ਚੀਜ਼ਾਂ_iter(m, menu_ਵਸਤੂ_ਕੁੰਜੀ_ਮੇਲ, ਵਸਤੂ_ਕੁੰਜੀ);
}

/*
 * Set *choice to point to the default item's data, if any default item was
 * set, and returns 1. If no default item was set, returns -ENOENT.
 */
ਪੂਰਨ menu_default_choice(ਬਣਤਰ menu *m, ਖਾਲੀ **choice)
{
	ਜੇ (m->default_ਵਸਤੂ) {
		*choice = m->default_ਵਸਤੂ->ਅੰਕੜੇ;
		ਵਾਪਸ 1;
	}

	ਵਾਪਸ -ENOENT;
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
ਸਥਿਰ ਸਤਰਅੰਦਰ ਪੂਰਨ menu_interactive_choice(ਬਣਤਰ menu *m, ਖਾਲੀ **choice)
{
	ਅੱਖਰ cbuf[CONFIG_SYS_CBSIZE];
	ਬਣਤਰ menu_ਵਸਤੂ *choice_ਵਸਤੂ = ਨਿਰਾ;
	ਪੂਰਨ readret;

	ਜਦੋਂ (!choice_ਵਸਤੂ) {
		cbuf[0] = '\0';

		menu_display(m);

		ਜੇ (!m->ਵਸਤੂ_choice) {
			readret = cli_readline_into_ਬਫਰ("Enter choice: ",
							   cbuf, m->timeout);

			ਜੇ (readret >= 0) {
				choice_ਵਸਤੂ = menu_ਵਸਤੂ_by_ਕੁੰਜੀ(m, cbuf);
				ਜੇ (!choice_ਵਸਤੂ)
					ਲਿਖੋ("%s not found\n", cbuf);
			} ਨਹੀਂਤਾਂ ਜੇ (readret == -1)  {
				ਲਿਖੋ("<INTERRUPT>\n");
				ਵਾਪਸ -EINTR;
			} ਨਹੀਂਤਾਂ {
				ਵਾਪਸ menu_default_choice(m, choice);
			}
		} ਨਹੀਂਤਾਂ {
			ਅੱਖਰ *ਕੁੰਜੀ = m->ਵਸਤੂ_choice(m->ਵਸਤੂ_choice_ਅੰਕੜੇ);

			ਜੇ (ਕੁੰਜੀ)
				choice_ਵਸਤੂ = menu_ਵਸਤੂ_by_ਕੁੰਜੀ(m, ਕੁੰਜੀ);
		}

		ਜੇ (!choice_ਵਸਤੂ)
			m->timeout = 0;
	}

	*choice = choice_ਵਸਤੂ->ਅੰਕੜੇ;

	ਵਾਪਸ 1;
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
ਪੂਰਨ menu_default_ਸੈੱਟ(ਬਣਤਰ menu *m, ਅੱਖਰ *ਵਸਤੂ_ਕੁੰਜੀ)
{
	ਬਣਤਰ menu_ਵਸਤੂ *ਵਸਤੂ;

	ਜੇ (!m)
		ਵਾਪਸ -EINVAL;

	ਵਸਤੂ = menu_ਵਸਤੂ_by_ਕੁੰਜੀ(m, ਵਸਤੂ_ਕੁੰਜੀ);

	ਜੇ (!ਵਸਤੂ)
		ਵਾਪਸ -ENOENT;

	m->default_ਵਸਤੂ = ਵਸਤੂ;

	ਵਾਪਸ 1;
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
ਪੂਰਨ menu_get_choice(ਬਣਤਰ menu *m, ਖਾਲੀ **choice)
{
	ਜੇ (!m || !choice)
		ਵਾਪਸ -EINVAL;

	ਜੇ (!m->ਵਸਤੂ_cnt)
		ਵਾਪਸ -ENOENT;

	ਜੇ (!m->prompt)
		ਵਾਪਸ menu_default_choice(m, choice);

	ਵਾਪਸ menu_interactive_choice(m, choice);
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
ਪੂਰਨ menu_ਵਸਤੂ_ਜੋੜੋ(ਬਣਤਰ menu *m, ਅੱਖਰ *ਵਸਤੂ_ਕੁੰਜੀ, ਖਾਲੀ *ਵਸਤੂ_ਅੰਕੜੇ)
{
	ਬਣਤਰ menu_ਵਸਤੂ *ਵਸਤੂ;

	ਜੇ (!m)
		ਵਾਪਸ -EINVAL;

	ਵਸਤੂ = menu_ਵਸਤੂ_by_ਕੁੰਜੀ(m, ਵਸਤੂ_ਕੁੰਜੀ);

	ਜੇ (ਵਸਤੂ) {
		ਵਸਤੂ->ਅੰਕੜੇ = ਵਸਤੂ_ਅੰਕੜੇ;
		ਵਾਪਸ 1;
	}

	ਵਸਤੂ = ਸਮ੍ਰਿਤੀ_ਦਿਓ(ਆਕਾਰ *ਵਸਤੂ);
	ਜੇ (!ਵਸਤੂ)
		ਵਾਪਸ -ENOMEM;

	ਵਸਤੂ->ਕੁੰਜੀ = strdup(ਵਸਤੂ_ਕੁੰਜੀ);

	ਜੇ (!ਵਸਤੂ->ਕੁੰਜੀ) {
		ਮੁਕਤ(ਵਸਤੂ);
		ਵਾਪਸ -ENOMEM;
	}

	ਵਸਤੂ->ਅੰਕੜੇ = ਵਸਤੂ_ਅੰਕੜੇ;

	list_ਜੋੜੋ_ਪੂਛ(&ਵਸਤੂ->list, &m->ਚੀਜ਼ਾਂ);
	m->ਵਸਤੂ_cnt++;

	ਵਾਪਸ 1;
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
ਬਣਤਰ menu *menu_create(ਅੱਖਰ *title, ਪੂਰਨ timeout, ਪੂਰਨ prompt,
				ਖਾਲੀ (*display_statusline)(ਬਣਤਰ menu *),
				ਖਾਲੀ (*ਵਸਤੂ_ਅੰਕੜੇ_print)(ਖਾਲੀ *),
				ਅੱਖਰ *(*ਵਸਤੂ_choice)(ਖਾਲੀ *),
				bool (*need_reprint)(ਖਾਲੀ *),
				ਖਾਲੀ *ਵਸਤੂ_choice_ਅੰਕੜੇ)
{
	ਬਣਤਰ menu *m;

	m = ਸਮ੍ਰਿਤੀ_ਦਿਓ(ਆਕਾਰ *m);

	ਜੇ (!m)
		ਵਾਪਸ ਨਿਰਾ;

	m->default_ਵਸਤੂ = ਨਿਰਾ;
	m->prompt = prompt;
	m->timeout = timeout;
	m->display_statusline = display_statusline;
	m->ਵਸਤੂ_ਅੰਕੜੇ_print = ਵਸਤੂ_ਅੰਕੜੇ_print;
	m->ਵਸਤੂ_choice = ਵਸਤੂ_choice;
	m->need_reprint = need_reprint;
	m->ਵਸਤੂ_choice_ਅੰਕੜੇ = ਵਸਤੂ_choice_ਅੰਕੜੇ;
	m->ਵਸਤੂ_cnt = 0;

	ਜੇ (title) {
		m->title = strdup(title);
		ਜੇ (!m->title) {
			ਮੁਕਤ(m);
			ਵਾਪਸ ਨਿਰਾ;
		}
	} ਨਹੀਂਤਾਂ
		m->title = ਨਿਰਾ;

	INIT_LIST_ਸਿਰ(&m->ਚੀਜ਼ਾਂ);

	ਵਾਪਸ m;
}

/*
 * menu_destroy() - frees the memory used by a menu and its items.
 *
 * m - Points to a menu created by menu_create().
 *
 * Returns 1 if successful, or -EINVAL if m is NULL.
 */
ਪੂਰਨ menu_destroy(ਬਣਤਰ menu *m)
{
	ਜੇ (!m)
		ਵਾਪਸ -EINVAL;

	menu_ਚੀਜ਼ਾਂ_iter(m, menu_ਵਸਤੂ_destroy, ਨਿਰਾ);

	ਜੇ (m->title)
		ਮੁਕਤ(m->title);

	ਮੁਕਤ(m);

	ਵਾਪਸ 1;
}

ਸਥਿਰ ਪੂਰਨ bootmenu_conv_shortcut_ਕੁੰਜੀ(ਬਣਤਰ bootmenu_ਅੰਕੜੇ *menu, ਪੂਰਨ ichar)
{
	ਪੂਰਨ shortcut_ਕੁੰਜੀ;

	ichar = tolower(ichar);
	ਚੋਣ (ichar) {
	/* a-z for bootmenu entry > 9 */
	ਹਾਲਤ 'a' ... 'z':
		shortcut_ਕੁੰਜੀ = ichar - 'a' + 9;
		ਰੋਕੋ;
	/* 1-9 for bootmenu entry <= 9 */
	ਹਾਲਤ '1' ... '9':
		shortcut_ਕੁੰਜੀ = ichar - '1';
		ਰੋਕੋ;
	/* Reserve 0 for last option (aka Exit) */
	ਹਾਲਤ '0':
	ਮੂਲ:
		ਵਾਪਸ -1;
	}

	ਵਾਪਸ shortcut_ਕੁੰਜੀ;
}

ਗਿਣਤੀ bootmenu_ਕੁੰਜੀ bootmenu_autoboot_loop(ਬਣਤਰ bootmenu_ਅੰਕੜੇ *menu,
					 ਬਣਤਰ cli_ch_state *cch)
{
	ਗਿਣਤੀ bootmenu_ਕੁੰਜੀ ਕੁੰਜੀ = BKEY_NONE;
	ਪੂਰਨ i, c;

	ਜਦੋਂ (menu->delay > 0) {
		ਪੂਰਨ ichar;

		ਜੇ (ansi)
			ਲਿਖੋ(ANSI_CURSOR_POSITION, menu->ਗਿਣਤੀ_2 + 5, 3);
		ਲਿਖੋ("Hit any key to stop autoboot: %d ", menu->delay);
		ਲਈ (i = 0; i < 100; ++i) {
			ਜੇ (!tstc()) {
				ਸਮਾਂ_ਸਾਰਣੀ();
				mdelay(10);
				ਜਾਰੀ;
			}

			menu->delay = -1;
			c = getchar();

			ichar = cli_ch_process(cch, c);

			ਚੋਣ (ichar) {
			ਹਾਲਤ '\0':
				ਕੁੰਜੀ = BKEY_NONE;
				ਰੋਕੋ;
			ਹਾਲਤ '\n':
				ਕੁੰਜੀ = BKEY_SELECT;
				ਰੋਕੋ;
			ਹਾਲਤ 0x3: /* ^C */
				ਕੁੰਜੀ = BKEY_QUIT;
				ਰੋਕੋ;
			ਹਾਲਤ 'A' ... 'Z':
			ਹਾਲਤ 'a' ... 'z':
			ਹਾਲਤ '0' ... '9':
				ਕੁੰਜੀ = BKEY_SHORTCUT;
				ਰੋਕੋ;
			ਮੂਲ:
				ਕੁੰਜੀ = BKEY_NONE;
				ਰੋਕੋ;
			}
			ਰੋਕੋ;
		}

		ਜੇ (ਕੁੰਜੀ == BKEY_SHORTCUT)
			cch->shortcut_ਕੁੰਜੀ = bootmenu_conv_shortcut_ਕੁੰਜੀ(menu, ichar);

		ਜੇ (menu->delay < 0)
			ਰੋਕੋ;

		--menu->delay;
	}

	ਜੇ (ansi)
		ਲਿਖੋ(ANSI_CURSOR_POSITION ANSI_CLEAR_ਸਤਰ, menu->ਗਿਣਤੀ_2 + 5, 1);

	ਜੇ (menu->delay == 0)
		ਕੁੰਜੀ = BKEY_SELECT;

	ਵਾਪਸ ਕੁੰਜੀ;
}

ਗਿਣਤੀ bootmenu_ਕੁੰਜੀ bootmenu_conv_ਕੁੰਜੀ(ਪੂਰਨ ichar)
{
	ਗਿਣਤੀ bootmenu_ਕੁੰਜੀ ਕੁੰਜੀ;

	ਚੋਣ (ichar) {
	ਹਾਲਤ '\n':
		/* enter key was pressed */
		ਕੁੰਜੀ = BKEY_SELECT;
		ਰੋਕੋ;
	ਹਾਲਤ CTL_CH('c'):
	ਹਾਲਤ '\e':
		/* ^C was pressed */
		ਕੁੰਜੀ = BKEY_QUIT;
		ਰੋਕੋ;
	ਹਾਲਤ CTL_CH('p'):
		ਕੁੰਜੀ = BKEY_ਉੱਪਰ;
		ਰੋਕੋ;
	ਹਾਲਤ CTL_CH('n'):
		ਕੁੰਜੀ = BKEY_ਥੱਲੇ;
		ਰੋਕੋ;
	ਹਾਲਤ CTL_CH('s'):
		ਕੁੰਜੀ = BKEY_SAVE;
		ਰੋਕੋ;
	ਹਾਲਤ '+':
		ਕੁੰਜੀ = BKEY_PLUS;
		ਰੋਕੋ;
	ਹਾਲਤ '-':
		ਕੁੰਜੀ = BKEY_MINUS;
		ਰੋਕੋ;
	ਹਾਲਤ ' ':
		ਕੁੰਜੀ = BKEY_SPACE;
		ਰੋਕੋ;
	ਮੂਲ:
		ਕੁੰਜੀ = BKEY_NONE;
		ਰੋਕੋ;
	}

	ਵਾਪਸ ਕੁੰਜੀ;
}

ਗਿਣਤੀ bootmenu_ਕੁੰਜੀ bootmenu_loop(ਬਣਤਰ bootmenu_ਅੰਕੜੇ *menu,
				ਬਣਤਰ cli_ch_state *cch)
{
	ਗਿਣਤੀ bootmenu_ਕੁੰਜੀ ਕੁੰਜੀ;
	ਪੂਰਨ c, errchar = 0;

	c = cli_ch_process(cch, 0);
	ਜੇ (!c) {
		ਜਦੋਂ (!c && !tstc()) {
			ਸਮਾਂ_ਸਾਰਣੀ();
			mdelay(10);
			c = cli_ch_process(cch, errchar);
			errchar = -ETIMEDOUT;
		}
		ਜੇ (!c) {
			c = getchar();
			c = cli_ch_process(cch, c);
		}
	}

	ਕੁੰਜੀ = bootmenu_conv_ਕੁੰਜੀ(c);

	ਜੇ (ਕੁੰਜੀ == BKEY_NONE && ਅੱਖਰ_ਜਾਂ_ਅੰਕ(c)) {
		ਕੁੰਜੀ = BKEY_SHORTCUT;
		cch->shortcut_ਕੁੰਜੀ = bootmenu_conv_shortcut_ਕੁੰਜੀ(menu, c);
	}

	ਵਾਪਸ ਕੁੰਜੀ;
}
