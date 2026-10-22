// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2010-2011 Calxeda, Inc.
 * Copyright (c) 2019, NVIDIA CORPORATION. All rights reserved.
 */

#ಅನ್ತರ್ಭಾವ <ansi.h>
#ಅನ್ತರ್ಭಾವ <cli.h>
#ಅನ್ತರ್ಭಾವ <malloc.h>
#ಅನ್ತರ್ಭಾವ <errno.h>
#ಅನ್ತರ್ಭಾವ <linux/ctype.h>
#ಅನ್ತರ್ಭಾವ <linux/delay.h>
#ಅನ್ತರ್ಭಾವ <linux/list.h>
#ಅನ್ತರ್ಭಾವ <watchdog.h>

#ಅನ್ತರ್ಭಾವ "menu.h"

#define ansi 1

/*
 * Internally, each item in a menu is represented by a struct menu_item.
 *
 * These items will be alloc'd and initialized by menu_item_add and destroyed
 * by menu_item_destroy, and the consumer of the interface never sees that
 * this struct is used at all.
 */
ಸಂರಚನಾ menu_item {
	ವರ್ಣ *ಕೀಲಿ;
	ಶೂನ್ಯ *data;
	ಸಂರಚನಾ list_head list;
};

/*
 * The menu is composed of a list of items along with settings and callbacks
 * provided by the user. An incomplete definition of this struct is available
 * in menu.h, but the full definition is here to prevent consumers from
 * relying on its contents.
 */
ಸಂರಚನಾ menu {
	ಸಂರಚನಾ menu_item *default_item;
	ಪೂರ್ಣಾಙ್ಕ timeout;
	ವರ್ಣ *title;
	ಪೂರ್ಣಾಙ್ಕ prompt;
	ಶೂನ್ಯ (*display_statusline)(ಸಂರಚನಾ menu *);
	ಶೂನ್ಯ (*item_data_print)(ಶೂನ್ಯ *);
	ವರ್ಣ *(*item_choice)(ಶೂನ್ಯ *);
	bool (*need_reprint)(ಶೂನ್ಯ *);
	ಶೂನ್ಯ *item_choice_data;
	ಸಂರಚನಾ list_head ವಸ್ತುಗಳು;
	ಪೂರ್ಣಾಙ್ಕ item_cnt;
};

/*
 * An iterator function for menu items. callback will be called for each item
 * in m, with m, a pointer to the item, and extra being passed to callback. If
 * callback returns a value other than NULL, iteration stops and the value
 * return by callback is returned from menu_items_iter.  This allows it to be
 * used for search type operations. It is also safe for callback to remove the
 * item from the list of items.
 */
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ *menu_ವಸ್ತುಗಳು_iter(ಸಂರಚನಾ menu *m,
		ಶೂನ್ಯ *(*callback)(ಸಂರಚನಾ menu *, ಸಂರಚನಾ menu_item *, ಶೂನ್ಯ *),
		ಶೂನ್ಯ *extra)
{
	ಸಂರಚನಾ list_head *pos, *n;
	ಸಂರಚನಾ menu_item *item;
	ಶೂನ್ಯ *ret;

	list_for_each_safe(pos, n, &m->ವಸ್ತುಗಳು) {
		item = list_entry(pos, ಸಂರಚನಾ menu_item, list);

		ret = callback(m, item, extra);

		ಯದಿ (ret)
			ಪ್ರತ್ಯಾಗಮ ret;
	}

	ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
}

/*
 * Print a menu_item. If the consumer provided an item_data_print function
 * when creating the menu, call it with a pointer to the item's private data.
 * Otherwise, print the key of the item.
 */
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ *menu_item_print(ಸಂರಚನಾ menu *m,
				ಸಂರಚನಾ menu_item *item,
				ಶೂನ್ಯ *extra)
{
	ಯದಿ (!m->item_data_print) {
		puts(item->ಕೀಲಿ);
		putc('\n');
	} ಅನ್ಯಥಾ {
		m->item_data_print(item->data);
	}

	ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
}

/*
 * Free the memory used by a menu item. This includes the memory used by its
 * key.
 */
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ *menu_item_destroy(ಸಂರಚನಾ menu *m,
				ಸಂರಚನಾ menu_item *item,
				ಶೂನ್ಯ *extra)
{
	ಯದಿ (item->ಕೀಲಿ)
		ಮುಕ್ತ(item->ಕೀಲಿ);

	ಮುಕ್ತ(item);

	ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
}

/*
 * Display a menu so the user can make a choice of an item. First display its
 * title, if any, and then each item in the menu.
 */
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ menu_display(ಸಂರಚನಾ menu *m)
{
	ಯದಿ (m->need_reprint) {
		ಯದಿ (!m->need_reprint(m->item_choice_data))
			ಪ್ರತ್ಯಾಗಮ;
	}

	ಯದಿ (m->title) {
		puts(m->title);
		putc('\n');
	}
	ಯದಿ (m->display_statusline)
		m->display_statusline(m);

	menu_ವಸ್ತುಗಳು_iter(m, menu_item_print, ರಿಕ್ತ);
}

/*
 * Check if an item's key matches a provided string, pointed to by extra. If
 * extra is NULL, an item with a NULL key will match. Otherwise, the item's
 * key has to match according to strcmp.
 *
 * This is called via menu_items_iter, so it returns a pointer to the item if
 * the key matches, and returns NULL otherwise.
 */
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ *menu_item_ಕೀಲಿ_match(ಸಂರಚನಾ menu *m,
			ಸಂರಚನಾ menu_item *item, ಶೂನ್ಯ *extra)
{
	ವರ್ಣ *item_ಕೀಲಿ = extra;

	ಯದಿ (!item_ಕೀಲಿ || !item->ಕೀಲಿ) {
		ಯದಿ (item_ಕೀಲಿ == item->ಕೀಲಿ)
			ಪ್ರತ್ಯಾಗಮ item;

		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}

	ಯದಿ (strcmp(item->ಕೀಲಿ, item_ಕೀಲಿ) == 0)
		ಪ್ರತ್ಯಾಗಮ item;

	ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
}

/*
 * Find the first item with a key matching item_key, if any exists.
 */
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಸಂರಚನಾ menu_item *menu_item_by_ಕೀಲಿ(ಸಂರಚನಾ menu *m,
							ವರ್ಣ *item_ಕೀಲಿ)
{
	ಪ್ರತ್ಯಾಗಮ menu_ವಸ್ತುಗಳು_iter(m, menu_item_ಕೀಲಿ_match, item_ಕೀಲಿ);
}

/*
 * Set *choice to point to the default item's data, if any default item was
 * set, and returns 1. If no default item was set, returns -ENOENT.
 */
ಪೂರ್ಣಾಙ್ಕ menu_default_choice(ಸಂರಚನಾ menu *m, ಶೂನ್ಯ **choice)
{
	ಯದಿ (m->default_item) {
		*choice = m->default_item->data;
		ಪ್ರತ್ಯಾಗಮ 1;
	}

	ಪ್ರತ್ಯಾಗಮ -ENOENT;
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
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಪೂರ್ಣಾಙ್ಕ menu_interactive_choice(ಸಂರಚನಾ menu *m, ಶೂನ್ಯ **choice)
{
	ವರ್ಣ cbuf[CONFIG_SYS_CBSIZE];
	ಸಂರಚನಾ menu_item *choice_item = ರಿಕ್ತ;
	ಪೂರ್ಣಾಙ್ಕ readret;

	ಯಾವತ್ (!choice_item) {
		cbuf[0] = '\0';

		menu_display(m);

		ಯದಿ (!m->item_choice) {
			readret = cli_readline_into_buffer("Enter choice: ",
							   cbuf, m->timeout);

			ಯದಿ (readret >= 0) {
				choice_item = menu_item_by_ಕೀಲಿ(m, cbuf);
				ಯದಿ (!choice_item)
					printf("%s not found\n", cbuf);
			} ಅನ್ಯಥಾ ಯದಿ (readret == -1)  {
				printf("<INTERRUPT>\n");
				ಪ್ರತ್ಯಾಗಮ -EINTR;
			} ಅನ್ಯಥಾ {
				ಪ್ರತ್ಯಾಗಮ menu_default_choice(m, choice);
			}
		} ಅನ್ಯಥಾ {
			ವರ್ಣ *ಕೀಲಿ = m->item_choice(m->item_choice_data);

			ಯದಿ (ಕೀಲಿ)
				choice_item = menu_item_by_ಕೀಲಿ(m, ಕೀಲಿ);
		}

		ಯದಿ (!choice_item)
			m->timeout = 0;
	}

	*choice = choice_item->data;

	ಪ್ರತ್ಯಾಗಮ 1;
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
ಪೂರ್ಣಾಙ್ಕ menu_default_set(ಸಂರಚನಾ menu *m, ವರ್ಣ *item_ಕೀಲಿ)
{
	ಸಂರಚನಾ menu_item *item;

	ಯದಿ (!m)
		ಪ್ರತ್ಯಾಗಮ -EINVAL;

	item = menu_item_by_ಕೀಲಿ(m, item_ಕೀಲಿ);

	ಯದಿ (!item)
		ಪ್ರತ್ಯಾಗಮ -ENOENT;

	m->default_item = item;

	ಪ್ರತ್ಯಾಗಮ 1;
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
ಪೂರ್ಣಾಙ್ಕ menu_get_choice(ಸಂರಚನಾ menu *m, ಶೂನ್ಯ **choice)
{
	ಯದಿ (!m || !choice)
		ಪ್ರತ್ಯಾಗಮ -EINVAL;

	ಯದಿ (!m->item_cnt)
		ಪ್ರತ್ಯಾಗಮ -ENOENT;

	ಯದಿ (!m->prompt)
		ಪ್ರತ್ಯಾಗಮ menu_default_choice(m, choice);

	ಪ್ರತ್ಯಾಗಮ menu_interactive_choice(m, choice);
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
ಪೂರ್ಣಾಙ್ಕ menu_item_add(ಸಂರಚನಾ menu *m, ವರ್ಣ *item_ಕೀಲಿ, ಶೂನ್ಯ *item_data)
{
	ಸಂರಚನಾ menu_item *item;

	ಯದಿ (!m)
		ಪ್ರತ್ಯಾಗಮ -EINVAL;

	item = menu_item_by_ಕೀಲಿ(m, item_ಕೀಲಿ);

	ಯದಿ (item) {
		item->data = item_data;
		ಪ್ರತ್ಯಾಗಮ 1;
	}

	item = malloc(ಪರಿಮಾಣ *item);
	ಯದಿ (!item)
		ಪ್ರತ್ಯಾಗಮ -ENOMEM;

	item->ಕೀಲಿ = strdup(item_ಕೀಲಿ);

	ಯದಿ (!item->ಕೀಲಿ) {
		ಮುಕ್ತ(item);
		ಪ್ರತ್ಯಾಗಮ -ENOMEM;
	}

	item->data = item_data;

	list_add_tail(&item->list, &m->ವಸ್ತುಗಳು);
	m->item_cnt++;

	ಪ್ರತ್ಯಾಗಮ 1;
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
ಸಂರಚನಾ menu *menu_create(ವರ್ಣ *title, ಪೂರ್ಣಾಙ್ಕ timeout, ಪೂರ್ಣಾಙ್ಕ prompt,
				ಶೂನ್ಯ (*display_statusline)(ಸಂರಚನಾ menu *),
				ಶೂನ್ಯ (*item_data_print)(ಶೂನ್ಯ *),
				ವರ್ಣ *(*item_choice)(ಶೂನ್ಯ *),
				bool (*need_reprint)(ಶೂನ್ಯ *),
				ಶೂನ್ಯ *item_choice_data)
{
	ಸಂರಚನಾ menu *m;

	m = malloc(ಪರಿಮಾಣ *m);

	ಯದಿ (!m)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

	m->default_item = ರಿಕ್ತ;
	m->prompt = prompt;
	m->timeout = timeout;
	m->display_statusline = display_statusline;
	m->item_data_print = item_data_print;
	m->item_choice = item_choice;
	m->need_reprint = need_reprint;
	m->item_choice_data = item_choice_data;
	m->item_cnt = 0;

	ಯದಿ (title) {
		m->title = strdup(title);
		ಯದಿ (!m->title) {
			ಮುಕ್ತ(m);
			ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
		}
	} ಅನ್ಯಥಾ
		m->title = ರಿಕ್ತ;

	INIT_LIST_HEAD(&m->ವಸ್ತುಗಳು);

	ಪ್ರತ್ಯಾಗಮ m;
}

/*
 * menu_destroy() - frees the memory used by a menu and its items.
 *
 * m - Points to a menu created by menu_create().
 *
 * Returns 1 if successful, or -EINVAL if m is NULL.
 */
ಪೂರ್ಣಾಙ್ಕ menu_destroy(ಸಂರಚನಾ menu *m)
{
	ಯದಿ (!m)
		ಪ್ರತ್ಯಾಗಮ -EINVAL;

	menu_ವಸ್ತುಗಳು_iter(m, menu_item_destroy, ರಿಕ್ತ);

	ಯದಿ (m->title)
		ಮುಕ್ತ(m->title);

	ಮುಕ್ತ(m);

	ಪ್ರತ್ಯಾಗಮ 1;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ bootmenu_conv_shortcut_ಕೀಲಿ(ಸಂರಚನಾ bootmenu_data *menu, ಪೂರ್ಣಾಙ್ಕ ichar)
{
	ಪೂರ್ಣಾಙ್ಕ shortcut_ಕೀಲಿ;

	ichar = tolower(ichar);
	ವಿಕಲ್ಪ (ichar) {
	/* a-z for bootmenu entry > 9 */
	ಪಕ್ಷ 'a' ... 'z':
		shortcut_ಕೀಲಿ = ichar - 'a' + 9;
		ಭಙ್ಗ;
	/* 1-9 for bootmenu entry <= 9 */
	ಪಕ್ಷ '1' ... '9':
		shortcut_ಕೀಲಿ = ichar - '1';
		ಭಙ್ಗ;
	/* Reserve 0 for last option (aka Exit) */
	ಪಕ್ಷ '0':
	ಸ್ವಭಾವ:
		ಪ್ರತ್ಯಾಗಮ -1;
	}

	ಪ್ರತ್ಯಾಗಮ shortcut_ಕೀಲಿ;
}

ಗಣನಾ bootmenu_ಕೀಲಿ bootmenu_autoboot_loop(ಸಂರಚನಾ bootmenu_data *menu,
					 ಸಂರಚನಾ cli_ch_state *cch)
{
	ಗಣನಾ bootmenu_ಕೀಲಿ ಕೀಲಿ = BKEY_NONE;
	ಪೂರ್ಣಾಙ್ಕ i, c;

	ಯಾವತ್ (menu->delay > 0) {
		ಪೂರ್ಣಾಙ್ಕ ichar;

		ಯದಿ (ansi)
			printf(ANSI_CURSOR_POSITION, menu->ಎಣಿಕೆ + 5, 3);
		printf("Hit any key to stop autoboot: %d ", menu->delay);
		ಪ್ರತಿ (i = 0; i < 100; ++i) {
			ಯದಿ (!tstc()) {
				schedule();
				mdelay(10);
				ಅನುವರ್ತ;
			}

			menu->delay = -1;
			c = getchar();

			ichar = cli_ch_process(cch, c);

			ವಿಕಲ್ಪ (ichar) {
			ಪಕ್ಷ '\0':
				ಕೀಲಿ = BKEY_NONE;
				ಭಙ್ಗ;
			ಪಕ್ಷ '\n':
				ಕೀಲಿ = BKEY_SELECT;
				ಭಙ್ಗ;
			ಪಕ್ಷ 0x3: /* ^C */
				ಕೀಲಿ = BKEY_QUIT;
				ಭಙ್ಗ;
			ಪಕ್ಷ 'A' ... 'Z':
			ಪಕ್ಷ 'a' ... 'z':
			ಪಕ್ಷ '0' ... '9':
				ಕೀಲಿ = BKEY_SHORTCUT;
				ಭಙ್ಗ;
			ಸ್ವಭಾವ:
				ಕೀಲಿ = BKEY_NONE;
				ಭಙ್ಗ;
			}
			ಭಙ್ಗ;
		}

		ಯದಿ (ಕೀಲಿ == BKEY_SHORTCUT)
			cch->shortcut_ಕೀಲಿ = bootmenu_conv_shortcut_ಕೀಲಿ(menu, ichar);

		ಯದಿ (menu->delay < 0)
			ಭಙ್ಗ;

		--menu->delay;
	}

	ಯದಿ (ansi)
		printf(ANSI_CURSOR_POSITION ANSI_CLEAR_ಸಾಲು, menu->ಎಣಿಕೆ + 5, 1);

	ಯದಿ (menu->delay == 0)
		ಕೀಲಿ = BKEY_SELECT;

	ಪ್ರತ್ಯಾಗಮ ಕೀಲಿ;
}

ಗಣನಾ bootmenu_ಕೀಲಿ bootmenu_conv_ಕೀಲಿ(ಪೂರ್ಣಾಙ್ಕ ichar)
{
	ಗಣನಾ bootmenu_ಕೀಲಿ ಕೀಲಿ;

	ವಿಕಲ್ಪ (ichar) {
	ಪಕ್ಷ '\n':
		/* enter key was pressed */
		ಕೀಲಿ = BKEY_SELECT;
		ಭಙ್ಗ;
	ಪಕ್ಷ CTL_CH('c'):
	ಪಕ್ಷ '\e':
		/* ^C was pressed */
		ಕೀಲಿ = BKEY_QUIT;
		ಭಙ್ಗ;
	ಪಕ್ಷ CTL_CH('p'):
		ಕೀಲಿ = BKEY_UP;
		ಭಙ್ಗ;
	ಪಕ್ಷ CTL_CH('n'):
		ಕೀಲಿ = BKEY_DOWN;
		ಭಙ್ಗ;
	ಪಕ್ಷ CTL_CH('s'):
		ಕೀಲಿ = BKEY_SAVE;
		ಭಙ್ಗ;
	ಪಕ್ಷ '+':
		ಕೀಲಿ = BKEY_PLUS;
		ಭಙ್ಗ;
	ಪಕ್ಷ '-':
		ಕೀಲಿ = BKEY_MINUS;
		ಭಙ್ಗ;
	ಪಕ್ಷ ' ':
		ಕೀಲಿ = BKEY_SPACE;
		ಭಙ್ಗ;
	ಸ್ವಭಾವ:
		ಕೀಲಿ = BKEY_NONE;
		ಭಙ್ಗ;
	}

	ಪ್ರತ್ಯಾಗಮ ಕೀಲಿ;
}

ಗಣನಾ bootmenu_ಕೀಲಿ bootmenu_loop(ಸಂರಚನಾ bootmenu_data *menu,
				ಸಂರಚನಾ cli_ch_state *cch)
{
	ಗಣನಾ bootmenu_ಕೀಲಿ ಕೀಲಿ;
	ಪೂರ್ಣಾಙ್ಕ c, errchar = 0;

	c = cli_ch_process(cch, 0);
	ಯದಿ (!c) {
		ಯಾವತ್ (!c && !tstc()) {
			schedule();
			mdelay(10);
			c = cli_ch_process(cch, errchar);
			errchar = -ETIMEDOUT;
		}
		ಯದಿ (!c) {
			c = getchar();
			c = cli_ch_process(cch, c);
		}
	}

	ಕೀಲಿ = bootmenu_conv_ಕೀಲಿ(c);

	ಯದಿ (ಕೀಲಿ == BKEY_NONE && isalnum(c)) {
		ಕೀಲಿ = BKEY_SHORTCUT;
		cch->shortcut_ಕೀಲಿ = bootmenu_conv_shortcut_ಕೀಲಿ(menu, c);
	}

	ಪ್ರತ್ಯಾಗಮ ಕೀಲಿ;
}
