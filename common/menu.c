// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2010-2011 Calxeda, Inc.
 * Copyright (c) 2019, NVIDIA CORPORATION. All rights reserved.
 */

#অন্তর্ভাব <ansi.h>
#অন্তর্ভাব <cli.h>
#অন্তর্ভাব <malloc.h>
#অন্তর্ভাব <errno.h>
#অন্তর্ভাব <linux/ctype.h>
#অন্তর্ভাব <linux/delay.h>
#অন্তর্ভাব <linux/list.h>
#অন্তর্ভাব <watchdog.h>

#অন্তর্ভাব "menu.h"

#define ansi 1

/*
 * Internally, each item in a menu is represented by a struct menu_item.
 *
 * These items will be alloc'd and initialized by menu_item_add and destroyed
 * by menu_item_destroy, and the consumer of the interface never sees that
 * this struct is used at all.
 */
সংরচনা menu_item {
	বর্ণ *চাবি;
	শূন্য *data;
	সংরচনা list_head list;
};

/*
 * The menu is composed of a list of items along with settings and callbacks
 * provided by the user. An incomplete definition of this struct is available
 * in menu.h, but the full definition is here to prevent consumers from
 * relying on its contents.
 */
সংরচনা menu {
	সংরচনা menu_item *default_item;
	পূর্ণাঙ্ক timeout;
	বর্ণ *title;
	পূর্ণাঙ্ক prompt;
	শূন্য (*display_statusline)(সংরচনা menu *);
	শূন্য (*item_data_print)(শূন্য *);
	বর্ণ *(*item_choice)(শূন্য *);
	bool (*need_reprint)(শূন্য *);
	শূন্য *item_choice_data;
	সংরচনা list_head বস্তুসমূহ;
	পূর্ণাঙ্ক item_cnt;
};

/*
 * An iterator function for menu items. callback will be called for each item
 * in m, with m, a pointer to the item, and extra being passed to callback. If
 * callback returns a value other than NULL, iteration stops and the value
 * return by callback is returned from menu_items_iter.  This allows it to be
 * used for search type operations. It is also safe for callback to remove the
 * item from the list of items.
 */
স্থির অন্তঃস্থ শূন্য *menu_বস্তুসমূহ_iter(সংরচনা menu *m,
		শূন্য *(*callback)(সংরচনা menu *, সংরচনা menu_item *, শূন্য *),
		শূন্য *extra)
{
	সংরচনা list_head *pos, *n;
	সংরচনা menu_item *item;
	শূন্য *ret;

	list_for_each_safe(pos, n, &m->বস্তুসমূহ) {
		item = list_entry(pos, সংরচনা menu_item, list);

		ret = callback(m, item, extra);

		যদি (ret)
			প্রত্যাগম ret;
	}

	প্রত্যাগম রিক্ত;
}

/*
 * Print a menu_item. If the consumer provided an item_data_print function
 * when creating the menu, call it with a pointer to the item's private data.
 * Otherwise, print the key of the item.
 */
স্থির অন্তঃস্থ শূন্য *menu_item_print(সংরচনা menu *m,
				সংরচনা menu_item *item,
				শূন্য *extra)
{
	যদি (!m->item_data_print) {
		puts(item->চাবি);
		putc('\n');
	} অন্যথা {
		m->item_data_print(item->data);
	}

	প্রত্যাগম রিক্ত;
}

/*
 * Free the memory used by a menu item. This includes the memory used by its
 * key.
 */
স্থির অন্তঃস্থ শূন্য *menu_item_destroy(সংরচনা menu *m,
				সংরচনা menu_item *item,
				শূন্য *extra)
{
	যদি (item->চাবি)
		মুক্ত(item->চাবি);

	মুক্ত(item);

	প্রত্যাগম রিক্ত;
}

/*
 * Display a menu so the user can make a choice of an item. First display its
 * title, if any, and then each item in the menu.
 */
স্থির অন্তঃস্থ শূন্য menu_display(সংরচনা menu *m)
{
	যদি (m->need_reprint) {
		যদি (!m->need_reprint(m->item_choice_data))
			প্রত্যাগম;
	}

	যদি (m->title) {
		puts(m->title);
		putc('\n');
	}
	যদি (m->display_statusline)
		m->display_statusline(m);

	menu_বস্তুসমূহ_iter(m, menu_item_print, রিক্ত);
}

/*
 * Check if an item's key matches a provided string, pointed to by extra. If
 * extra is NULL, an item with a NULL key will match. Otherwise, the item's
 * key has to match according to strcmp.
 *
 * This is called via menu_items_iter, so it returns a pointer to the item if
 * the key matches, and returns NULL otherwise.
 */
স্থির অন্তঃস্থ শূন্য *menu_item_চাবি_match(সংরচনা menu *m,
			সংরচনা menu_item *item, শূন্য *extra)
{
	বর্ণ *item_চাবি = extra;

	যদি (!item_চাবি || !item->চাবি) {
		যদি (item_চাবি == item->চাবি)
			প্রত্যাগম item;

		প্রত্যাগম রিক্ত;
	}

	যদি (strcmp(item->চাবি, item_চাবি) == 0)
		প্রত্যাগম item;

	প্রত্যাগম রিক্ত;
}

/*
 * Find the first item with a key matching item_key, if any exists.
 */
স্থির অন্তঃস্থ সংরচনা menu_item *menu_item_by_চাবি(সংরচনা menu *m,
							বর্ণ *item_চাবি)
{
	প্রত্যাগম menu_বস্তুসমূহ_iter(m, menu_item_চাবি_match, item_চাবি);
}

/*
 * Set *choice to point to the default item's data, if any default item was
 * set, and returns 1. If no default item was set, returns -ENOENT.
 */
পূর্ণাঙ্ক menu_default_choice(সংরচনা menu *m, শূন্য **choice)
{
	যদি (m->default_item) {
		*choice = m->default_item->data;
		প্রত্যাগম 1;
	}

	প্রত্যাগম -ENOENT;
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
স্থির অন্তঃস্থ পূর্ণাঙ্ক menu_interactive_choice(সংরচনা menu *m, শূন্য **choice)
{
	বর্ণ cbuf[CONFIG_SYS_CBSIZE];
	সংরচনা menu_item *choice_item = রিক্ত;
	পূর্ণাঙ্ক readret;

	যাবত্ (!choice_item) {
		cbuf[0] = '\0';

		menu_display(m);

		যদি (!m->item_choice) {
			readret = cli_readline_into_buffer("Enter choice: ",
							   cbuf, m->timeout);

			যদি (readret >= 0) {
				choice_item = menu_item_by_চাবি(m, cbuf);
				যদি (!choice_item)
					printf("%s not found\n", cbuf);
			} অন্যথা যদি (readret == -1)  {
				printf("<INTERRUPT>\n");
				প্রত্যাগম -EINTR;
			} অন্যথা {
				প্রত্যাগম menu_default_choice(m, choice);
			}
		} অন্যথা {
			বর্ণ *চাবি = m->item_choice(m->item_choice_data);

			যদি (চাবি)
				choice_item = menu_item_by_চাবি(m, চাবি);
		}

		যদি (!choice_item)
			m->timeout = 0;
	}

	*choice = choice_item->data;

	প্রত্যাগম 1;
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
পূর্ণাঙ্ক menu_default_set(সংরচনা menu *m, বর্ণ *item_চাবি)
{
	সংরচনা menu_item *item;

	যদি (!m)
		প্রত্যাগম -EINVAL;

	item = menu_item_by_চাবি(m, item_চাবি);

	যদি (!item)
		প্রত্যাগম -ENOENT;

	m->default_item = item;

	প্রত্যাগম 1;
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
পূর্ণাঙ্ক menu_get_choice(সংরচনা menu *m, শূন্য **choice)
{
	যদি (!m || !choice)
		প্রত্যাগম -EINVAL;

	যদি (!m->item_cnt)
		প্রত্যাগম -ENOENT;

	যদি (!m->prompt)
		প্রত্যাগম menu_default_choice(m, choice);

	প্রত্যাগম menu_interactive_choice(m, choice);
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
পূর্ণাঙ্ক menu_item_add(সংরচনা menu *m, বর্ণ *item_চাবি, শূন্য *item_data)
{
	সংরচনা menu_item *item;

	যদি (!m)
		প্রত্যাগম -EINVAL;

	item = menu_item_by_চাবি(m, item_চাবি);

	যদি (item) {
		item->data = item_data;
		প্রত্যাগম 1;
	}

	item = malloc(পরিমাণ *item);
	যদি (!item)
		প্রত্যাগম -ENOMEM;

	item->চাবি = strdup(item_চাবি);

	যদি (!item->চাবি) {
		মুক্ত(item);
		প্রত্যাগম -ENOMEM;
	}

	item->data = item_data;

	list_add_tail(&item->list, &m->বস্তুসমূহ);
	m->item_cnt++;

	প্রত্যাগম 1;
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
সংরচনা menu *menu_create(বর্ণ *title, পূর্ণাঙ্ক timeout, পূর্ণাঙ্ক prompt,
				শূন্য (*display_statusline)(সংরচনা menu *),
				শূন্য (*item_data_print)(শূন্য *),
				বর্ণ *(*item_choice)(শূন্য *),
				bool (*need_reprint)(শূন্য *),
				শূন্য *item_choice_data)
{
	সংরচনা menu *m;

	m = malloc(পরিমাণ *m);

	যদি (!m)
		প্রত্যাগম রিক্ত;

	m->default_item = রিক্ত;
	m->prompt = prompt;
	m->timeout = timeout;
	m->display_statusline = display_statusline;
	m->item_data_print = item_data_print;
	m->item_choice = item_choice;
	m->need_reprint = need_reprint;
	m->item_choice_data = item_choice_data;
	m->item_cnt = 0;

	যদি (title) {
		m->title = strdup(title);
		যদি (!m->title) {
			মুক্ত(m);
			প্রত্যাগম রিক্ত;
		}
	} অন্যথা
		m->title = রিক্ত;

	INIT_LIST_HEAD(&m->বস্তুসমূহ);

	প্রত্যাগম m;
}

/*
 * menu_destroy() - frees the memory used by a menu and its items.
 *
 * m - Points to a menu created by menu_create().
 *
 * Returns 1 if successful, or -EINVAL if m is NULL.
 */
পূর্ণাঙ্ক menu_destroy(সংরচনা menu *m)
{
	যদি (!m)
		প্রত্যাগম -EINVAL;

	menu_বস্তুসমূহ_iter(m, menu_item_destroy, রিক্ত);

	যদি (m->title)
		মুক্ত(m->title);

	মুক্ত(m);

	প্রত্যাগম 1;
}

স্থির পূর্ণাঙ্ক bootmenu_conv_shortcut_চাবি(সংরচনা bootmenu_data *menu, পূর্ণাঙ্ক ichar)
{
	পূর্ণাঙ্ক shortcut_চাবি;

	ichar = tolower(ichar);
	বিকল্প (ichar) {
	/* a-z for bootmenu entry > 9 */
	পক্ষ 'a' ... 'z':
		shortcut_চাবি = ichar - 'a' + 9;
		ভঙ্গ;
	/* 1-9 for bootmenu entry <= 9 */
	পক্ষ '1' ... '9':
		shortcut_চাবি = ichar - '1';
		ভঙ্গ;
	/* Reserve 0 for last option (aka Exit) */
	পক্ষ '0':
	স্বভাব:
		প্রত্যাগম -1;
	}

	প্রত্যাগম shortcut_চাবি;
}

গণনা bootmenu_চাবি bootmenu_autoboot_loop(সংরচনা bootmenu_data *menu,
					 সংরচনা cli_ch_state *cch)
{
	গণনা bootmenu_চাবি চাবি = BKEY_NONE;
	পূর্ণাঙ্ক i, c;

	যাবত্ (menu->delay > 0) {
		পূর্ণাঙ্ক ichar;

		যদি (ansi)
			printf(ANSI_CURSOR_POSITION, menu->গুনতি + 5, 3);
		printf("Hit any key to stop autoboot: %d ", menu->delay);
		প্রতি (i = 0; i < 100; ++i) {
			যদি (!tstc()) {
				schedule();
				mdelay(10);
				অনুবর্ত;
			}

			menu->delay = -1;
			c = getchar();

			ichar = cli_ch_process(cch, c);

			বিকল্প (ichar) {
			পক্ষ '\0':
				চাবি = BKEY_NONE;
				ভঙ্গ;
			পক্ষ '\n':
				চাবি = BKEY_SELECT;
				ভঙ্গ;
			পক্ষ 0x3: /* ^C */
				চাবি = BKEY_QUIT;
				ভঙ্গ;
			পক্ষ 'A' ... 'Z':
			পক্ষ 'a' ... 'z':
			পক্ষ '0' ... '9':
				চাবি = BKEY_SHORTCUT;
				ভঙ্গ;
			স্বভাব:
				চাবি = BKEY_NONE;
				ভঙ্গ;
			}
			ভঙ্গ;
		}

		যদি (চাবি == BKEY_SHORTCUT)
			cch->shortcut_চাবি = bootmenu_conv_shortcut_চাবি(menu, ichar);

		যদি (menu->delay < 0)
			ভঙ্গ;

		--menu->delay;
	}

	যদি (ansi)
		printf(ANSI_CURSOR_POSITION ANSI_CLEAR_সারি, menu->গুনতি + 5, 1);

	যদি (menu->delay == 0)
		চাবি = BKEY_SELECT;

	প্রত্যাগম চাবি;
}

গণনা bootmenu_চাবি bootmenu_conv_চাবি(পূর্ণাঙ্ক ichar)
{
	গণনা bootmenu_চাবি চাবি;

	বিকল্প (ichar) {
	পক্ষ '\n':
		/* enter key was pressed */
		চাবি = BKEY_SELECT;
		ভঙ্গ;
	পক্ষ CTL_CH('c'):
	পক্ষ '\e':
		/* ^C was pressed */
		চাবি = BKEY_QUIT;
		ভঙ্গ;
	পক্ষ CTL_CH('p'):
		চাবি = BKEY_UP;
		ভঙ্গ;
	পক্ষ CTL_CH('n'):
		চাবি = BKEY_DOWN;
		ভঙ্গ;
	পক্ষ CTL_CH('s'):
		চাবি = BKEY_SAVE;
		ভঙ্গ;
	পক্ষ '+':
		চাবি = BKEY_PLUS;
		ভঙ্গ;
	পক্ষ '-':
		চাবি = BKEY_MINUS;
		ভঙ্গ;
	পক্ষ ' ':
		চাবি = BKEY_SPACE;
		ভঙ্গ;
	স্বভাব:
		চাবি = BKEY_NONE;
		ভঙ্গ;
	}

	প্রত্যাগম চাবি;
}

গণনা bootmenu_চাবি bootmenu_loop(সংরচনা bootmenu_data *menu,
				সংরচনা cli_ch_state *cch)
{
	গণনা bootmenu_চাবি চাবি;
	পূর্ণাঙ্ক c, errchar = 0;

	c = cli_ch_process(cch, 0);
	যদি (!c) {
		যাবত্ (!c && !tstc()) {
			schedule();
			mdelay(10);
			c = cli_ch_process(cch, errchar);
			errchar = -ETIMEDOUT;
		}
		যদি (!c) {
			c = getchar();
			c = cli_ch_process(cch, c);
		}
	}

	চাবি = bootmenu_conv_চাবি(c);

	যদি (চাবি == BKEY_NONE && isalnum(c)) {
		চাবি = BKEY_SHORTCUT;
		cch->shortcut_চাবি = bootmenu_conv_shortcut_চাবি(menu, c);
	}

	প্রত্যাগম চাবি;
}
