// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2010-2011 Calxeda, Inc.
 * Copyright (c) 2019, NVIDIA CORPORATION. All rights reserved.
 */

#അന്തര്ഭാവ <ansi.h>
#അന്തര്ഭാവ <cli.h>
#അന്തര്ഭാവ <malloc.h>
#അന്തര്ഭാവ <errno.h>
#അന്തര്ഭാവ <linux/ctype.h>
#അന്തര്ഭാവ <linux/delay.h>
#അന്തര്ഭാവ <linux/list.h>
#അന്തര്ഭാവ <watchdog.h>

#അന്തര്ഭാവ "menu.h"

#define ansi 1

/*
 * Internally, each item in a menu is represented by a struct menu_item.
 *
 * These items will be alloc'd and initialized by menu_item_add and destroyed
 * by menu_item_destroy, and the consumer of the interface never sees that
 * this struct is used at all.
 */
ഘടന menu_item {
	അക്ഷരം *താക്കോൽ;
	ശൂന്യം *data;
	ഘടന list_head list;
};

/*
 * The menu is composed of a list of items along with settings and callbacks
 * provided by the user. An incomplete definition of this struct is available
 * in menu.h, but the full definition is here to prevent consumers from
 * relying on its contents.
 */
ഘടന menu {
	ഘടന menu_item *default_item;
	പൂർണ്ണസംഖ്യ timeout;
	അക്ഷരം *title;
	പൂർണ്ണസംഖ്യ prompt;
	ശൂന്യം (*display_statusline)(ഘടന menu *);
	ശൂന്യം (*item_data_print)(ശൂന്യം *);
	അക്ഷരം *(*item_choice)(ശൂന്യം *);
	bool (*need_reprint)(ശൂന്യം *);
	ശൂന്യം *item_choice_data;
	ഘടന list_head ഇനങ്ങൾ;
	പൂർണ്ണസംഖ്യ item_cnt;
};

/*
 * An iterator function for menu items. callback will be called for each item
 * in m, with m, a pointer to the item, and extra being passed to callback. If
 * callback returns a value other than NULL, iteration stops and the value
 * return by callback is returned from menu_items_iter.  This allows it to be
 * used for search type operations. It is also safe for callback to remove the
 * item from the list of items.
 */
സ്ഥിതം വരിയിൽ ശൂന്യം *menu_ഇനങ്ങൾ_iter(ഘടന menu *m,
		ശൂന്യം *(*callback)(ഘടന menu *, ഘടന menu_item *, ശൂന്യം *),
		ശൂന്യം *extra)
{
	ഘടന list_head *pos, *n;
	ഘടന menu_item *item;
	ശൂന്യം *ret;

	list_for_each_safe(pos, n, &m->ഇനങ്ങൾ) {
		item = list_entry(pos, ഘടന menu_item, list);

		ret = callback(m, item, extra);

		എങ്കിൽ (ret)
			തിരികെ ret;
	}

	തിരികെ ശൂന്യ;
}

/*
 * Print a menu_item. If the consumer provided an item_data_print function
 * when creating the menu, call it with a pointer to the item's private data.
 * Otherwise, print the key of the item.
 */
സ്ഥിതം വരിയിൽ ശൂന്യം *menu_item_print(ഘടന menu *m,
				ഘടന menu_item *item,
				ശൂന്യം *extra)
{
	എങ്കിൽ (!m->item_data_print) {
		puts(item->താക്കോൽ);
		putc('\n');
	} അല്ലെങ്കിൽ {
		m->item_data_print(item->data);
	}

	തിരികെ ശൂന്യ;
}

/*
 * Free the memory used by a menu item. This includes the memory used by its
 * key.
 */
സ്ഥിതം വരിയിൽ ശൂന്യം *menu_item_destroy(ഘടന menu *m,
				ഘടന menu_item *item,
				ശൂന്യം *extra)
{
	എങ്കിൽ (item->താക്കോൽ)
		സ്വതന്ത്രം(item->താക്കോൽ);

	സ്വതന്ത്രം(item);

	തിരികെ ശൂന്യ;
}

/*
 * Display a menu so the user can make a choice of an item. First display its
 * title, if any, and then each item in the menu.
 */
സ്ഥിതം വരിയിൽ ശൂന്യം menu_display(ഘടന menu *m)
{
	എങ്കിൽ (m->need_reprint) {
		എങ്കിൽ (!m->need_reprint(m->item_choice_data))
			തിരികെ;
	}

	എങ്കിൽ (m->title) {
		puts(m->title);
		putc('\n');
	}
	എങ്കിൽ (m->display_statusline)
		m->display_statusline(m);

	menu_ഇനങ്ങൾ_iter(m, menu_item_print, ശൂന്യ);
}

/*
 * Check if an item's key matches a provided string, pointed to by extra. If
 * extra is NULL, an item with a NULL key will match. Otherwise, the item's
 * key has to match according to strcmp.
 *
 * This is called via menu_items_iter, so it returns a pointer to the item if
 * the key matches, and returns NULL otherwise.
 */
സ്ഥിതം വരിയിൽ ശൂന്യം *menu_item_താക്കോൽ_match(ഘടന menu *m,
			ഘടന menu_item *item, ശൂന്യം *extra)
{
	അക്ഷരം *item_താക്കോൽ = extra;

	എങ്കിൽ (!item_താക്കോൽ || !item->താക്കോൽ) {
		എങ്കിൽ (item_താക്കോൽ == item->താക്കോൽ)
			തിരികെ item;

		തിരികെ ശൂന്യ;
	}

	എങ്കിൽ (strcmp(item->താക്കോൽ, item_താക്കോൽ) == 0)
		തിരികെ item;

	തിരികെ ശൂന്യ;
}

/*
 * Find the first item with a key matching item_key, if any exists.
 */
സ്ഥിതം വരിയിൽ ഘടന menu_item *menu_item_by_താക്കോൽ(ഘടന menu *m,
							അക്ഷരം *item_താക്കോൽ)
{
	തിരികെ menu_ഇനങ്ങൾ_iter(m, menu_item_താക്കോൽ_match, item_താക്കോൽ);
}

/*
 * Set *choice to point to the default item's data, if any default item was
 * set, and returns 1. If no default item was set, returns -ENOENT.
 */
പൂർണ്ണസംഖ്യ menu_default_choice(ഘടന menu *m, ശൂന്യം **choice)
{
	എങ്കിൽ (m->default_item) {
		*choice = m->default_item->data;
		തിരികെ 1;
	}

	തിരികെ -ENOENT;
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
സ്ഥിതം വരിയിൽ പൂർണ്ണസംഖ്യ menu_interactive_choice(ഘടന menu *m, ശൂന്യം **choice)
{
	അക്ഷരം cbuf[CONFIG_SYS_CBSIZE];
	ഘടന menu_item *choice_item = ശൂന്യ;
	പൂർണ്ണസംഖ്യ readret;

	ആകുമ്പോൾ (!choice_item) {
		cbuf[0] = '\0';

		menu_display(m);

		എങ്കിൽ (!m->item_choice) {
			readret = cli_readline_into_buffer("Enter choice: ",
							   cbuf, m->timeout);

			എങ്കിൽ (readret >= 0) {
				choice_item = menu_item_by_താക്കോൽ(m, cbuf);
				എങ്കിൽ (!choice_item)
					printf("%s not found\n", cbuf);
			} അല്ലെങ്കിൽ എങ്കിൽ (readret == -1)  {
				printf("<INTERRUPT>\n");
				തിരികെ -EINTR;
			} അല്ലെങ്കിൽ {
				തിരികെ menu_default_choice(m, choice);
			}
		} അല്ലെങ്കിൽ {
			അക്ഷരം *താക്കോൽ = m->item_choice(m->item_choice_data);

			എങ്കിൽ (താക്കോൽ)
				choice_item = menu_item_by_താക്കോൽ(m, താക്കോൽ);
		}

		എങ്കിൽ (!choice_item)
			m->timeout = 0;
	}

	*choice = choice_item->data;

	തിരികെ 1;
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
പൂർണ്ണസംഖ്യ menu_default_set(ഘടന menu *m, അക്ഷരം *item_താക്കോൽ)
{
	ഘടന menu_item *item;

	എങ്കിൽ (!m)
		തിരികെ -EINVAL;

	item = menu_item_by_താക്കോൽ(m, item_താക്കോൽ);

	എങ്കിൽ (!item)
		തിരികെ -ENOENT;

	m->default_item = item;

	തിരികെ 1;
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
പൂർണ്ണസംഖ്യ menu_get_choice(ഘടന menu *m, ശൂന്യം **choice)
{
	എങ്കിൽ (!m || !choice)
		തിരികെ -EINVAL;

	എങ്കിൽ (!m->item_cnt)
		തിരികെ -ENOENT;

	എങ്കിൽ (!m->prompt)
		തിരികെ menu_default_choice(m, choice);

	തിരികെ menu_interactive_choice(m, choice);
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
പൂർണ്ണസംഖ്യ menu_item_add(ഘടന menu *m, അക്ഷരം *item_താക്കോൽ, ശൂന്യം *item_data)
{
	ഘടന menu_item *item;

	എങ്കിൽ (!m)
		തിരികെ -EINVAL;

	item = menu_item_by_താക്കോൽ(m, item_താക്കോൽ);

	എങ്കിൽ (item) {
		item->data = item_data;
		തിരികെ 1;
	}

	item = malloc(വലുപ്പം *item);
	എങ്കിൽ (!item)
		തിരികെ -ENOMEM;

	item->താക്കോൽ = strdup(item_താക്കോൽ);

	എങ്കിൽ (!item->താക്കോൽ) {
		സ്വതന്ത്രം(item);
		തിരികെ -ENOMEM;
	}

	item->data = item_data;

	list_add_tail(&item->list, &m->ഇനങ്ങൾ);
	m->item_cnt++;

	തിരികെ 1;
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
ഘടന menu *menu_create(അക്ഷരം *title, പൂർണ്ണസംഖ്യ timeout, പൂർണ്ണസംഖ്യ prompt,
				ശൂന്യം (*display_statusline)(ഘടന menu *),
				ശൂന്യം (*item_data_print)(ശൂന്യം *),
				അക്ഷരം *(*item_choice)(ശൂന്യം *),
				bool (*need_reprint)(ശൂന്യം *),
				ശൂന്യം *item_choice_data)
{
	ഘടന menu *m;

	m = malloc(വലുപ്പം *m);

	എങ്കിൽ (!m)
		തിരികെ ശൂന്യ;

	m->default_item = ശൂന്യ;
	m->prompt = prompt;
	m->timeout = timeout;
	m->display_statusline = display_statusline;
	m->item_data_print = item_data_print;
	m->item_choice = item_choice;
	m->need_reprint = need_reprint;
	m->item_choice_data = item_choice_data;
	m->item_cnt = 0;

	എങ്കിൽ (title) {
		m->title = strdup(title);
		എങ്കിൽ (!m->title) {
			സ്വതന്ത്രം(m);
			തിരികെ ശൂന്യ;
		}
	} അല്ലെങ്കിൽ
		m->title = ശൂന്യ;

	INIT_LIST_HEAD(&m->ഇനങ്ങൾ);

	തിരികെ m;
}

/*
 * menu_destroy() - frees the memory used by a menu and its items.
 *
 * m - Points to a menu created by menu_create().
 *
 * Returns 1 if successful, or -EINVAL if m is NULL.
 */
പൂർണ്ണസംഖ്യ menu_destroy(ഘടന menu *m)
{
	എങ്കിൽ (!m)
		തിരികെ -EINVAL;

	menu_ഇനങ്ങൾ_iter(m, menu_item_destroy, ശൂന്യ);

	എങ്കിൽ (m->title)
		സ്വതന്ത്രം(m->title);

	സ്വതന്ത്രം(m);

	തിരികെ 1;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ bootmenu_conv_shortcut_താക്കോൽ(ഘടന bootmenu_data *menu, പൂർണ്ണസംഖ്യ ichar)
{
	പൂർണ്ണസംഖ്യ shortcut_താക്കോൽ;

	ichar = tolower(ichar);
	തിരഞ്ഞെടുപ്പ് (ichar) {
	/* a-z for bootmenu entry > 9 */
	അവസ്ഥ 'a' ... 'z':
		shortcut_താക്കോൽ = ichar - 'a' + 9;
		നിർത്തുക;
	/* 1-9 for bootmenu entry <= 9 */
	അവസ്ഥ '1' ... '9':
		shortcut_താക്കോൽ = ichar - '1';
		നിർത്തുക;
	/* Reserve 0 for last option (aka Exit) */
	അവസ്ഥ '0':
	സ്ഥിരസ്ഥിതി:
		തിരികെ -1;
	}

	തിരികെ shortcut_താക്കോൽ;
}

എണ്ണൽ bootmenu_താക്കോൽ bootmenu_autoboot_loop(ഘടന bootmenu_data *menu,
					 ഘടന cli_ch_state *cch)
{
	എണ്ണൽ bootmenu_താക്കോൽ താക്കോൽ = BKEY_NONE;
	പൂർണ്ണസംഖ്യ i, c;

	ആകുമ്പോൾ (menu->delay > 0) {
		പൂർണ്ണസംഖ്യ ichar;

		എങ്കിൽ (ansi)
			printf(ANSI_CURSOR_POSITION, menu->എണ്ണം + 5, 3);
		printf("Hit any key to stop autoboot: %d ", menu->delay);
		വേണ്ടി (i = 0; i < 100; ++i) {
			എങ്കിൽ (!tstc()) {
				schedule();
				mdelay(10);
				തുടരുക;
			}

			menu->delay = -1;
			c = getchar();

			ichar = cli_ch_process(cch, c);

			തിരഞ്ഞെടുപ്പ് (ichar) {
			അവസ്ഥ '\0':
				താക്കോൽ = BKEY_NONE;
				നിർത്തുക;
			അവസ്ഥ '\n':
				താക്കോൽ = BKEY_SELECT;
				നിർത്തുക;
			അവസ്ഥ 0x3: /* ^C */
				താക്കോൽ = BKEY_QUIT;
				നിർത്തുക;
			അവസ്ഥ 'A' ... 'Z':
			അവസ്ഥ 'a' ... 'z':
			അവസ്ഥ '0' ... '9':
				താക്കോൽ = BKEY_SHORTCUT;
				നിർത്തുക;
			സ്ഥിരസ്ഥിതി:
				താക്കോൽ = BKEY_NONE;
				നിർത്തുക;
			}
			നിർത്തുക;
		}

		എങ്കിൽ (താക്കോൽ == BKEY_SHORTCUT)
			cch->shortcut_താക്കോൽ = bootmenu_conv_shortcut_താക്കോൽ(menu, ichar);

		എങ്കിൽ (menu->delay < 0)
			നിർത്തുക;

		--menu->delay;
	}

	എങ്കിൽ (ansi)
		printf(ANSI_CURSOR_POSITION ANSI_CLEAR_വരി, menu->എണ്ണം + 5, 1);

	എങ്കിൽ (menu->delay == 0)
		താക്കോൽ = BKEY_SELECT;

	തിരികെ താക്കോൽ;
}

എണ്ണൽ bootmenu_താക്കോൽ bootmenu_conv_താക്കോൽ(പൂർണ്ണസംഖ്യ ichar)
{
	എണ്ണൽ bootmenu_താക്കോൽ താക്കോൽ;

	തിരഞ്ഞെടുപ്പ് (ichar) {
	അവസ്ഥ '\n':
		/* enter key was pressed */
		താക്കോൽ = BKEY_SELECT;
		നിർത്തുക;
	അവസ്ഥ CTL_CH('c'):
	അവസ്ഥ '\e':
		/* ^C was pressed */
		താക്കോൽ = BKEY_QUIT;
		നിർത്തുക;
	അവസ്ഥ CTL_CH('p'):
		താക്കോൽ = BKEY_UP;
		നിർത്തുക;
	അവസ്ഥ CTL_CH('n'):
		താക്കോൽ = BKEY_DOWN;
		നിർത്തുക;
	അവസ്ഥ CTL_CH('s'):
		താക്കോൽ = BKEY_SAVE;
		നിർത്തുക;
	അവസ്ഥ '+':
		താക്കോൽ = BKEY_PLUS;
		നിർത്തുക;
	അവസ്ഥ '-':
		താക്കോൽ = BKEY_MINUS;
		നിർത്തുക;
	അവസ്ഥ ' ':
		താക്കോൽ = BKEY_SPACE;
		നിർത്തുക;
	സ്ഥിരസ്ഥിതി:
		താക്കോൽ = BKEY_NONE;
		നിർത്തുക;
	}

	തിരികെ താക്കോൽ;
}

എണ്ണൽ bootmenu_താക്കോൽ bootmenu_loop(ഘടന bootmenu_data *menu,
				ഘടന cli_ch_state *cch)
{
	എണ്ണൽ bootmenu_താക്കോൽ താക്കോൽ;
	പൂർണ്ണസംഖ്യ c, errchar = 0;

	c = cli_ch_process(cch, 0);
	എങ്കിൽ (!c) {
		ആകുമ്പോൾ (!c && !tstc()) {
			schedule();
			mdelay(10);
			c = cli_ch_process(cch, errchar);
			errchar = -ETIMEDOUT;
		}
		എങ്കിൽ (!c) {
			c = getchar();
			c = cli_ch_process(cch, c);
		}
	}

	താക്കോൽ = bootmenu_conv_താക്കോൽ(c);

	എങ്കിൽ (താക്കോൽ == BKEY_NONE && isalnum(c)) {
		താക്കോൽ = BKEY_SHORTCUT;
		cch->shortcut_താക്കോൽ = bootmenu_conv_shortcut_താക്കോൽ(menu, c);
	}

	തിരികെ താക്കോൽ;
}
