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

#ವ್ಯಾಖ್ಯಾನಿಸು ansi 1

/*
 * Internally, each item in a menu is represented by a struct menu_item.
 *
 * These items will be alloc'd and initialized by menu_item_add and destroyed
 * by menu_item_destroy, and the consumer of the interface never sees that
 * this struct is used at all.
 */
ರಚನೆ menu_ಅಂಶ {
	ಅಕ್ಷರ *ಕೀಲಿ;
	ಶೂನ್ಯ *ದತ್ತಾಂಶ;
	ರಚನೆ list_ತಲೆ list;
};

/*
 * The menu is composed of a list of items along with settings and callbacks
 * provided by the user. An incomplete definition of this struct is available
 * in menu.h, but the full definition is here to prevent consumers from
 * relying on its contents.
 */
ರಚನೆ menu {
	ರಚನೆ menu_ಅಂಶ *default_ಅಂಶ;
	ಪೂರ್ಣಾಂಕ timeout;
	ಅಕ್ಷರ *title;
	ಪೂರ್ಣಾಂಕ prompt;
	ಶೂನ್ಯ (*display_statusline)(ರಚನೆ menu *);
	ಶೂನ್ಯ (*ಅಂಶ_ದತ್ತಾಂಶ_print)(ಶೂನ್ಯ *);
	ಅಕ್ಷರ *(*ಅಂಶ_choice)(ಶೂನ್ಯ *);
	bool (*need_reprint)(ಶೂನ್ಯ *);
	ಶೂನ್ಯ *ಅಂಶ_choice_ದತ್ತಾಂಶ;
	ರಚನೆ list_ತಲೆ ವಸ್ತುಗಳು;
	ಪೂರ್ಣಾಂಕ ಅಂಶ_cnt;
};

/*
 * An iterator function for menu items. callback will be called for each item
 * in m, with m, a pointer to the item, and extra being passed to callback. If
 * callback returns a value other than NULL, iteration stops and the value
 * return by callback is returned from menu_items_iter.  This allows it to be
 * used for search type operations. It is also safe for callback to remove the
 * item from the list of items.
 */
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ *menu_ವಸ್ತುಗಳು_iter(ರಚನೆ menu *m,
		ಶೂನ್ಯ *(*callback)(ರಚನೆ menu *, ರಚನೆ menu_ಅಂಶ *, ಶೂನ್ಯ *),
		ಶೂನ್ಯ *extra)
{
	ರಚನೆ list_ತಲೆ *pos, *n;
	ರಚನೆ menu_ಅಂಶ *ಅಂಶ;
	ಶೂನ್ಯ *ret;

	list_for_each_ಸುರಕ್ಷಿತ(pos, n, &m->ವಸ್ತುಗಳು) {
		ಅಂಶ = list_ನಮೂದು(pos, ರಚನೆ menu_ಅಂಶ, list);

		ret = callback(m, ಅಂಶ, extra);

		ಆದರೆ (ret)
			ಹಿಂತಿರುಗಿ ret;
	}

	ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}

/*
 * Print a menu_item. If the consumer provided an item_data_print function
 * when creating the menu, call it with a pointer to the item's private data.
 * Otherwise, print the key of the item.
 */
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ *menu_ಅಂಶ_print(ರಚನೆ menu *m,
				ರಚನೆ menu_ಅಂಶ *ಅಂಶ,
				ಶೂನ್ಯ *extra)
{
	ಆದರೆ (!m->ಅಂಶ_ದತ್ತಾಂಶ_print) {
		puts(ಅಂಶ->ಕೀಲಿ);
		putc('\n');
	} ಇಲ್ಲದಿದ್ದರೆ {
		m->ಅಂಶ_ದತ್ತಾಂಶ_print(ಅಂಶ->ದತ್ತಾಂಶ);
	}

	ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}

/*
 * Free the memory used by a menu item. This includes the memory used by its
 * key.
 */
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ *menu_ಅಂಶ_destroy(ರಚನೆ menu *m,
				ರಚನೆ menu_ಅಂಶ *ಅಂಶ,
				ಶೂನ್ಯ *extra)
{
	ಆದರೆ (ಅಂಶ->ಕೀಲಿ)
		ಬಿಡುಗಡೆ(ಅಂಶ->ಕೀಲಿ);

	ಬಿಡುಗಡೆ(ಅಂಶ);

	ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}

/*
 * Display a menu so the user can make a choice of an item. First display its
 * title, if any, and then each item in the menu.
 */
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ menu_display(ರಚನೆ menu *m)
{
	ಆದರೆ (m->need_reprint) {
		ಆದರೆ (!m->need_reprint(m->ಅಂಶ_choice_ದತ್ತಾಂಶ))
			ಹಿಂತಿರುಗಿ;
	}

	ಆದರೆ (m->title) {
		puts(m->title);
		putc('\n');
	}
	ಆದರೆ (m->display_statusline)
		m->display_statusline(m);

	menu_ವಸ್ತುಗಳು_iter(m, menu_ಅಂಶ_print, ಖಾಲಿ);
}

/*
 * Check if an item's key matches a provided string, pointed to by extra. If
 * extra is NULL, an item with a NULL key will match. Otherwise, the item's
 * key has to match according to strcmp.
 *
 * This is called via menu_items_iter, so it returns a pointer to the item if
 * the key matches, and returns NULL otherwise.
 */
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ *menu_ಅಂಶ_ಕೀಲಿ_ಹೊಂದಿಕೆ(ರಚನೆ menu *m,
			ರಚನೆ menu_ಅಂಶ *ಅಂಶ, ಶೂನ್ಯ *extra)
{
	ಅಕ್ಷರ *ಅಂಶ_ಕೀಲಿ = extra;

	ಆದರೆ (!ಅಂಶ_ಕೀಲಿ || !ಅಂಶ->ಕೀಲಿ) {
		ಆದರೆ (ಅಂಶ_ಕೀಲಿ == ಅಂಶ->ಕೀಲಿ)
			ಹಿಂತಿರುಗಿ ಅಂಶ;

		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}

	ಆದರೆ (strcmp(ಅಂಶ->ಕೀಲಿ, ಅಂಶ_ಕೀಲಿ) == 0)
		ಹಿಂತಿರುಗಿ ಅಂಶ;

	ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}

/*
 * Find the first item with a key matching item_key, if any exists.
 */
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ರಚನೆ menu_ಅಂಶ *menu_ಅಂಶ_by_ಕೀಲಿ(ರಚನೆ menu *m,
							ಅಕ್ಷರ *ಅಂಶ_ಕೀಲಿ)
{
	ಹಿಂತಿರುಗಿ menu_ವಸ್ತುಗಳು_iter(m, menu_ಅಂಶ_ಕೀಲಿ_ಹೊಂದಿಕೆ, ಅಂಶ_ಕೀಲಿ);
}

/*
 * Set *choice to point to the default item's data, if any default item was
 * set, and returns 1. If no default item was set, returns -ENOENT.
 */
ಪೂರ್ಣಾಂಕ menu_default_choice(ರಚನೆ menu *m, ಶೂನ್ಯ **choice)
{
	ಆದರೆ (m->default_ಅಂಶ) {
		*choice = m->default_ಅಂಶ->ದತ್ತಾಂಶ;
		ಹಿಂತಿರುಗಿ 1;
	}

	ಹಿಂತಿರುಗಿ -ENOENT;
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
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಪೂರ್ಣಾಂಕ menu_interactive_choice(ರಚನೆ menu *m, ಶೂನ್ಯ **choice)
{
	ಅಕ್ಷರ cbuf[CONFIG_SYS_CBSIZE];
	ರಚನೆ menu_ಅಂಶ *choice_ಅಂಶ = ಖಾಲಿ;
	ಪೂರ್ಣಾಂಕ readret;

	ಆಗ (!choice_ಅಂಶ) {
		cbuf[0] = '\0';

		menu_display(m);

		ಆದರೆ (!m->ಅಂಶ_choice) {
			readret = cli_readline_into_ಬಫರ್("Enter choice: ",
							   cbuf, m->timeout);

			ಆದರೆ (readret >= 0) {
				choice_ಅಂಶ = menu_ಅಂಶ_by_ಕೀಲಿ(m, cbuf);
				ಆದರೆ (!choice_ಅಂಶ)
					ಬರೆ("%s not found\n", cbuf);
			} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (readret == -1)  {
				ಬರೆ("<INTERRUPT>\n");
				ಹಿಂತಿರುಗಿ -EINTR;
			} ಇಲ್ಲದಿದ್ದರೆ {
				ಹಿಂತಿರುಗಿ menu_default_choice(m, choice);
			}
		} ಇಲ್ಲದಿದ್ದರೆ {
			ಅಕ್ಷರ *ಕೀಲಿ = m->ಅಂಶ_choice(m->ಅಂಶ_choice_ದತ್ತಾಂಶ);

			ಆದರೆ (ಕೀಲಿ)
				choice_ಅಂಶ = menu_ಅಂಶ_by_ಕೀಲಿ(m, ಕೀಲಿ);
		}

		ಆದರೆ (!choice_ಅಂಶ)
			m->timeout = 0;
	}

	*choice = choice_ಅಂಶ->ದತ್ತಾಂಶ;

	ಹಿಂತಿರುಗಿ 1;
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
ಪೂರ್ಣಾಂಕ menu_default_ಗಣ(ರಚನೆ menu *m, ಅಕ್ಷರ *ಅಂಶ_ಕೀಲಿ)
{
	ರಚನೆ menu_ಅಂಶ *ಅಂಶ;

	ಆದರೆ (!m)
		ಹಿಂತಿರುಗಿ -EINVAL;

	ಅಂಶ = menu_ಅಂಶ_by_ಕೀಲಿ(m, ಅಂಶ_ಕೀಲಿ);

	ಆದರೆ (!ಅಂಶ)
		ಹಿಂತಿರುಗಿ -ENOENT;

	m->default_ಅಂಶ = ಅಂಶ;

	ಹಿಂತಿರುಗಿ 1;
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
ಪೂರ್ಣಾಂಕ menu_get_choice(ರಚನೆ menu *m, ಶೂನ್ಯ **choice)
{
	ಆದರೆ (!m || !choice)
		ಹಿಂತಿರುಗಿ -EINVAL;

	ಆದರೆ (!m->ಅಂಶ_cnt)
		ಹಿಂತಿರುಗಿ -ENOENT;

	ಆದರೆ (!m->prompt)
		ಹಿಂತಿರುಗಿ menu_default_choice(m, choice);

	ಹಿಂತಿರುಗಿ menu_interactive_choice(m, choice);
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
ಪೂರ್ಣಾಂಕ menu_ಅಂಶ_ಕೂಡಿಸು(ರಚನೆ menu *m, ಅಕ್ಷರ *ಅಂಶ_ಕೀಲಿ, ಶೂನ್ಯ *ಅಂಶ_ದತ್ತಾಂಶ)
{
	ರಚನೆ menu_ಅಂಶ *ಅಂಶ;

	ಆದರೆ (!m)
		ಹಿಂತಿರುಗಿ -EINVAL;

	ಅಂಶ = menu_ಅಂಶ_by_ಕೀಲಿ(m, ಅಂಶ_ಕೀಲಿ);

	ಆದರೆ (ಅಂಶ) {
		ಅಂಶ->ದತ್ತಾಂಶ = ಅಂಶ_ದತ್ತಾಂಶ;
		ಹಿಂತಿರುಗಿ 1;
	}

	ಅಂಶ = ಸ್ಮರಣೆ_ಕೊಡು(ಗಾತ್ರ *ಅಂಶ);
	ಆದರೆ (!ಅಂಶ)
		ಹಿಂತಿರುಗಿ -ENOMEM;

	ಅಂಶ->ಕೀಲಿ = strdup(ಅಂಶ_ಕೀಲಿ);

	ಆದರೆ (!ಅಂಶ->ಕೀಲಿ) {
		ಬಿಡುಗಡೆ(ಅಂಶ);
		ಹಿಂತಿರುಗಿ -ENOMEM;
	}

	ಅಂಶ->ದತ್ತಾಂಶ = ಅಂಶ_ದತ್ತಾಂಶ;

	list_ಕೂಡಿಸು_ಬಾಲ(&ಅಂಶ->list, &m->ವಸ್ತುಗಳು);
	m->ಅಂಶ_cnt++;

	ಹಿಂತಿರುಗಿ 1;
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
ರಚನೆ menu *menu_create(ಅಕ್ಷರ *title, ಪೂರ್ಣಾಂಕ timeout, ಪೂರ್ಣಾಂಕ prompt,
				ಶೂನ್ಯ (*display_statusline)(ರಚನೆ menu *),
				ಶೂನ್ಯ (*ಅಂಶ_ದತ್ತಾಂಶ_print)(ಶೂನ್ಯ *),
				ಅಕ್ಷರ *(*ಅಂಶ_choice)(ಶೂನ್ಯ *),
				bool (*need_reprint)(ಶೂನ್ಯ *),
				ಶೂನ್ಯ *ಅಂಶ_choice_ದತ್ತಾಂಶ)
{
	ರಚನೆ menu *m;

	m = ಸ್ಮರಣೆ_ಕೊಡು(ಗಾತ್ರ *m);

	ಆದರೆ (!m)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	m->default_ಅಂಶ = ಖಾಲಿ;
	m->prompt = prompt;
	m->timeout = timeout;
	m->display_statusline = display_statusline;
	m->ಅಂಶ_ದತ್ತಾಂಶ_print = ಅಂಶ_ದತ್ತಾಂಶ_print;
	m->ಅಂಶ_choice = ಅಂಶ_choice;
	m->need_reprint = need_reprint;
	m->ಅಂಶ_choice_ದತ್ತಾಂಶ = ಅಂಶ_choice_ದತ್ತಾಂಶ;
	m->ಅಂಶ_cnt = 0;

	ಆದರೆ (title) {
		m->title = strdup(title);
		ಆದರೆ (!m->title) {
			ಬಿಡುಗಡೆ(m);
			ಹಿಂತಿರುಗಿ ಖಾಲಿ;
		}
	} ಇಲ್ಲದಿದ್ದರೆ
		m->title = ಖಾಲಿ;

	INIT_LIST_ತಲೆ(&m->ವಸ್ತುಗಳು);

	ಹಿಂತಿರುಗಿ m;
}

/*
 * menu_destroy() - frees the memory used by a menu and its items.
 *
 * m - Points to a menu created by menu_create().
 *
 * Returns 1 if successful, or -EINVAL if m is NULL.
 */
ಪೂರ್ಣಾಂಕ menu_destroy(ರಚನೆ menu *m)
{
	ಆದರೆ (!m)
		ಹಿಂತಿರುಗಿ -EINVAL;

	menu_ವಸ್ತುಗಳು_iter(m, menu_ಅಂಶ_destroy, ಖಾಲಿ);

	ಆದರೆ (m->title)
		ಬಿಡುಗಡೆ(m->title);

	ಬಿಡುಗಡೆ(m);

	ಹಿಂತಿರುಗಿ 1;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ bootmenu_conv_shortcut_ಕೀಲಿ(ರಚನೆ bootmenu_ದತ್ತಾಂಶ *menu, ಪೂರ್ಣಾಂಕ ichar)
{
	ಪೂರ್ಣಾಂಕ shortcut_ಕೀಲಿ;

	ichar = tolower(ichar);
	ಆಯ್ಕೆ (ichar) {
	/* a-z for bootmenu entry > 9 */
	ಸ್ಥಿತಿ 'a' ... 'z':
		shortcut_ಕೀಲಿ = ichar - 'a' + 9;
		ನಿಲ್ಲಿಸು;
	/* 1-9 for bootmenu entry <= 9 */
	ಸ್ಥಿತಿ '1' ... '9':
		shortcut_ಕೀಲಿ = ichar - '1';
		ನಿಲ್ಲಿಸು;
	/* Reserve 0 for last option (aka Exit) */
	ಸ್ಥಿತಿ '0':
	ಪೂರ್ವನಿಯೋಜಿತ:
		ಹಿಂತಿರುಗಿ -1;
	}

	ಹಿಂತಿರುಗಿ shortcut_ಕೀಲಿ;
}

ಎಣಿಕೆ bootmenu_ಕೀಲಿ bootmenu_autoboot_loop(ರಚನೆ bootmenu_ದತ್ತಾಂಶ *menu,
					 ರಚನೆ cli_ch_state *cch)
{
	ಎಣಿಕೆ bootmenu_ಕೀಲಿ ಕೀಲಿ = BKEY_NONE;
	ಪೂರ್ಣಾಂಕ i, c;

	ಆಗ (menu->delay > 0) {
		ಪೂರ್ಣಾಂಕ ichar;

		ಆದರೆ (ansi)
			ಬರೆ(ANSI_CURSOR_POSITION, menu->ಎಣಿಕೆ_2 + 5, 3);
		ಬರೆ("Hit any key to stop autoboot: %d ", menu->delay);
		ಗಾಗಿ (i = 0; i < 100; ++i) {
			ಆದರೆ (!tstc()) {
				ವೇಳಾಪಟ್ಟಿ();
				mdelay(10);
				ಮುಂದುವರಿ;
			}

			menu->delay = -1;
			c = getchar();

			ichar = cli_ch_process(cch, c);

			ಆಯ್ಕೆ (ichar) {
			ಸ್ಥಿತಿ '\0':
				ಕೀಲಿ = BKEY_NONE;
				ನಿಲ್ಲಿಸು;
			ಸ್ಥಿತಿ '\n':
				ಕೀಲಿ = BKEY_SELECT;
				ನಿಲ್ಲಿಸು;
			ಸ್ಥಿತಿ 0x3: /* ^C */
				ಕೀಲಿ = BKEY_QUIT;
				ನಿಲ್ಲಿಸು;
			ಸ್ಥಿತಿ 'A' ... 'Z':
			ಸ್ಥಿತಿ 'a' ... 'z':
			ಸ್ಥಿತಿ '0' ... '9':
				ಕೀಲಿ = BKEY_SHORTCUT;
				ನಿಲ್ಲಿಸು;
			ಪೂರ್ವನಿಯೋಜಿತ:
				ಕೀಲಿ = BKEY_NONE;
				ನಿಲ್ಲಿಸು;
			}
			ನಿಲ್ಲಿಸು;
		}

		ಆದರೆ (ಕೀಲಿ == BKEY_SHORTCUT)
			cch->shortcut_ಕೀಲಿ = bootmenu_conv_shortcut_ಕೀಲಿ(menu, ichar);

		ಆದರೆ (menu->delay < 0)
			ನಿಲ್ಲಿಸು;

		--menu->delay;
	}

	ಆದರೆ (ansi)
		ಬರೆ(ANSI_CURSOR_POSITION ANSI_CLEAR_ಸಾಲು, menu->ಎಣಿಕೆ_2 + 5, 1);

	ಆದರೆ (menu->delay == 0)
		ಕೀಲಿ = BKEY_SELECT;

	ಹಿಂತಿರುಗಿ ಕೀಲಿ;
}

ಎಣಿಕೆ bootmenu_ಕೀಲಿ bootmenu_conv_ಕೀಲಿ(ಪೂರ್ಣಾಂಕ ichar)
{
	ಎಣಿಕೆ bootmenu_ಕೀಲಿ ಕೀಲಿ;

	ಆಯ್ಕೆ (ichar) {
	ಸ್ಥಿತಿ '\n':
		/* enter key was pressed */
		ಕೀಲಿ = BKEY_SELECT;
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CTL_CH('c'):
	ಸ್ಥಿತಿ '\e':
		/* ^C was pressed */
		ಕೀಲಿ = BKEY_QUIT;
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CTL_CH('p'):
		ಕೀಲಿ = BKEY_ಮೇಲೆ;
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CTL_CH('n'):
		ಕೀಲಿ = BKEY_ಕೆಳಕ್ಕೆ;
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CTL_CH('s'):
		ಕೀಲಿ = BKEY_SAVE;
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ '+':
		ಕೀಲಿ = BKEY_PLUS;
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ '-':
		ಕೀಲಿ = BKEY_MINUS;
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ ' ':
		ಕೀಲಿ = BKEY_SPACE;
		ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
		ಕೀಲಿ = BKEY_NONE;
		ನಿಲ್ಲಿಸು;
	}

	ಹಿಂತಿರುಗಿ ಕೀಲಿ;
}

ಎಣಿಕೆ bootmenu_ಕೀಲಿ bootmenu_loop(ರಚನೆ bootmenu_ದತ್ತಾಂಶ *menu,
				ರಚನೆ cli_ch_state *cch)
{
	ಎಣಿಕೆ bootmenu_ಕೀಲಿ ಕೀಲಿ;
	ಪೂರ್ಣಾಂಕ c, errchar = 0;

	c = cli_ch_process(cch, 0);
	ಆದರೆ (!c) {
		ಆಗ (!c && !tstc()) {
			ವೇಳಾಪಟ್ಟಿ();
			mdelay(10);
			c = cli_ch_process(cch, errchar);
			errchar = -ETIMEDOUT;
		}
		ಆದರೆ (!c) {
			c = getchar();
			c = cli_ch_process(cch, c);
		}
	}

	ಕೀಲಿ = bootmenu_conv_ಕೀಲಿ(c);

	ಆದರೆ (ಕೀಲಿ == BKEY_NONE && ಅಕ್ಷರವೇ_ಅಂಕಿಯೇ(c)) {
		ಕೀಲಿ = BKEY_SHORTCUT;
		cch->shortcut_ಕೀಲಿ = bootmenu_conv_shortcut_ಕೀಲಿ(menu, c);
	}

	ಹಿಂತಿರುಗಿ ಕೀಲಿ;
}
