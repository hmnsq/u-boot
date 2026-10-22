// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2010-2011 Calxeda, Inc.
 * Copyright (c) 2019, NVIDIA CORPORATION. All rights reserved.
 */

#அந்தர்பாவ <ansi.h>
#அந்தர்பாவ <cli.h>
#அந்தர்பாவ <malloc.h>
#அந்தர்பாவ <errno.h>
#அந்தர்பாவ <linux/ctype.h>
#அந்தர்பாவ <linux/delay.h>
#அந்தர்பாவ <linux/list.h>
#அந்தர்பாவ <watchdog.h>

#அந்தர்பாவ "menu.h"

#define ansi 1

/*
 * Internally, each item in a menu is represented by a struct menu_item.
 *
 * These items will be alloc'd and initialized by menu_item_add and destroyed
 * by menu_item_destroy, and the consumer of the interface never sees that
 * this struct is used at all.
 */
கட்டமைப்பு menu_item {
	எழுத்து *சாவி;
	வெறுமை *data;
	கட்டமைப்பு list_head list;
};

/*
 * The menu is composed of a list of items along with settings and callbacks
 * provided by the user. An incomplete definition of this struct is available
 * in menu.h, but the full definition is here to prevent consumers from
 * relying on its contents.
 */
கட்டமைப்பு menu {
	கட்டமைப்பு menu_item *default_item;
	முழு timeout;
	எழுத்து *title;
	முழு prompt;
	வெறுமை (*display_statusline)(கட்டமைப்பு menu *);
	வெறுமை (*item_data_print)(வெறுமை *);
	எழுத்து *(*item_choice)(வெறுமை *);
	bool (*need_reprint)(வெறுமை *);
	வெறுமை *item_choice_data;
	கட்டமைப்பு list_head பொருட்கள்;
	முழு item_cnt;
};

/*
 * An iterator function for menu items. callback will be called for each item
 * in m, with m, a pointer to the item, and extra being passed to callback. If
 * callback returns a value other than NULL, iteration stops and the value
 * return by callback is returned from menu_items_iter.  This allows it to be
 * used for search type operations. It is also safe for callback to remove the
 * item from the list of items.
 */
நிலையான உள்வரி வெறுமை *menu_பொருட்கள்_iter(கட்டமைப்பு menu *m,
		வெறுமை *(*callback)(கட்டமைப்பு menu *, கட்டமைப்பு menu_item *, வெறுமை *),
		வெறுமை *extra)
{
	கட்டமைப்பு list_head *pos, *n;
	கட்டமைப்பு menu_item *item;
	வெறுமை *ret;

	list_for_each_safe(pos, n, &m->பொருட்கள்) {
		item = list_entry(pos, கட்டமைப்பு menu_item, list);

		ret = callback(m, item, extra);

		எனில் (ret)
			திரும்பு ret;
	}

	திரும்பு வெற்று;
}

/*
 * Print a menu_item. If the consumer provided an item_data_print function
 * when creating the menu, call it with a pointer to the item's private data.
 * Otherwise, print the key of the item.
 */
நிலையான உள்வரி வெறுமை *menu_item_print(கட்டமைப்பு menu *m,
				கட்டமைப்பு menu_item *item,
				வெறுமை *extra)
{
	எனில் (!m->item_data_print) {
		puts(item->சாவி);
		putc('\n');
	} இல்லையெனில் {
		m->item_data_print(item->data);
	}

	திரும்பு வெற்று;
}

/*
 * Free the memory used by a menu item. This includes the memory used by its
 * key.
 */
நிலையான உள்வரி வெறுமை *menu_item_destroy(கட்டமைப்பு menu *m,
				கட்டமைப்பு menu_item *item,
				வெறுமை *extra)
{
	எனில் (item->சாவி)
		விடுவி(item->சாவி);

	விடுவி(item);

	திரும்பு வெற்று;
}

/*
 * Display a menu so the user can make a choice of an item. First display its
 * title, if any, and then each item in the menu.
 */
நிலையான உள்வரி வெறுமை menu_display(கட்டமைப்பு menu *m)
{
	எனில் (m->need_reprint) {
		எனில் (!m->need_reprint(m->item_choice_data))
			திரும்பு;
	}

	எனில் (m->title) {
		puts(m->title);
		putc('\n');
	}
	எனில் (m->display_statusline)
		m->display_statusline(m);

	menu_பொருட்கள்_iter(m, menu_item_print, வெற்று);
}

/*
 * Check if an item's key matches a provided string, pointed to by extra. If
 * extra is NULL, an item with a NULL key will match. Otherwise, the item's
 * key has to match according to strcmp.
 *
 * This is called via menu_items_iter, so it returns a pointer to the item if
 * the key matches, and returns NULL otherwise.
 */
நிலையான உள்வரி வெறுமை *menu_item_சாவி_match(கட்டமைப்பு menu *m,
			கட்டமைப்பு menu_item *item, வெறுமை *extra)
{
	எழுத்து *item_சாவி = extra;

	எனில் (!item_சாவி || !item->சாவி) {
		எனில் (item_சாவி == item->சாவி)
			திரும்பு item;

		திரும்பு வெற்று;
	}

	எனில் (strcmp(item->சாவி, item_சாவி) == 0)
		திரும்பு item;

	திரும்பு வெற்று;
}

/*
 * Find the first item with a key matching item_key, if any exists.
 */
நிலையான உள்வரி கட்டமைப்பு menu_item *menu_item_by_சாவி(கட்டமைப்பு menu *m,
							எழுத்து *item_சாவி)
{
	திரும்பு menu_பொருட்கள்_iter(m, menu_item_சாவி_match, item_சாவி);
}

/*
 * Set *choice to point to the default item's data, if any default item was
 * set, and returns 1. If no default item was set, returns -ENOENT.
 */
முழு menu_default_choice(கட்டமைப்பு menu *m, வெறுமை **choice)
{
	எனில் (m->default_item) {
		*choice = m->default_item->data;
		திரும்பு 1;
	}

	திரும்பு -ENOENT;
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
நிலையான உள்வரி முழு menu_interactive_choice(கட்டமைப்பு menu *m, வெறுமை **choice)
{
	எழுத்து cbuf[CONFIG_SYS_CBSIZE];
	கட்டமைப்பு menu_item *choice_item = வெற்று;
	முழு readret;

	போது (!choice_item) {
		cbuf[0] = '\0';

		menu_display(m);

		எனில் (!m->item_choice) {
			readret = cli_readline_into_buffer("Enter choice: ",
							   cbuf, m->timeout);

			எனில் (readret >= 0) {
				choice_item = menu_item_by_சாவி(m, cbuf);
				எனில் (!choice_item)
					printf("%s not found\n", cbuf);
			} இல்லையெனில் எனில் (readret == -1)  {
				printf("<INTERRUPT>\n");
				திரும்பு -EINTR;
			} இல்லையெனில் {
				திரும்பு menu_default_choice(m, choice);
			}
		} இல்லையெனில் {
			எழுத்து *சாவி = m->item_choice(m->item_choice_data);

			எனில் (சாவி)
				choice_item = menu_item_by_சாவி(m, சாவி);
		}

		எனில் (!choice_item)
			m->timeout = 0;
	}

	*choice = choice_item->data;

	திரும்பு 1;
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
முழு menu_default_set(கட்டமைப்பு menu *m, எழுத்து *item_சாவி)
{
	கட்டமைப்பு menu_item *item;

	எனில் (!m)
		திரும்பு -EINVAL;

	item = menu_item_by_சாவி(m, item_சாவி);

	எனில் (!item)
		திரும்பு -ENOENT;

	m->default_item = item;

	திரும்பு 1;
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
முழு menu_get_choice(கட்டமைப்பு menu *m, வெறுமை **choice)
{
	எனில் (!m || !choice)
		திரும்பு -EINVAL;

	எனில் (!m->item_cnt)
		திரும்பு -ENOENT;

	எனில் (!m->prompt)
		திரும்பு menu_default_choice(m, choice);

	திரும்பு menu_interactive_choice(m, choice);
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
முழு menu_item_add(கட்டமைப்பு menu *m, எழுத்து *item_சாவி, வெறுமை *item_data)
{
	கட்டமைப்பு menu_item *item;

	எனில் (!m)
		திரும்பு -EINVAL;

	item = menu_item_by_சாவி(m, item_சாவி);

	எனில் (item) {
		item->data = item_data;
		திரும்பு 1;
	}

	item = malloc(அளவு *item);
	எனில் (!item)
		திரும்பு -ENOMEM;

	item->சாவி = strdup(item_சாவி);

	எனில் (!item->சாவி) {
		விடுவி(item);
		திரும்பு -ENOMEM;
	}

	item->data = item_data;

	list_add_tail(&item->list, &m->பொருட்கள்);
	m->item_cnt++;

	திரும்பு 1;
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
கட்டமைப்பு menu *menu_create(எழுத்து *title, முழு timeout, முழு prompt,
				வெறுமை (*display_statusline)(கட்டமைப்பு menu *),
				வெறுமை (*item_data_print)(வெறுமை *),
				எழுத்து *(*item_choice)(வெறுமை *),
				bool (*need_reprint)(வெறுமை *),
				வெறுமை *item_choice_data)
{
	கட்டமைப்பு menu *m;

	m = malloc(அளவு *m);

	எனில் (!m)
		திரும்பு வெற்று;

	m->default_item = வெற்று;
	m->prompt = prompt;
	m->timeout = timeout;
	m->display_statusline = display_statusline;
	m->item_data_print = item_data_print;
	m->item_choice = item_choice;
	m->need_reprint = need_reprint;
	m->item_choice_data = item_choice_data;
	m->item_cnt = 0;

	எனில் (title) {
		m->title = strdup(title);
		எனில் (!m->title) {
			விடுவி(m);
			திரும்பு வெற்று;
		}
	} இல்லையெனில்
		m->title = வெற்று;

	INIT_LIST_HEAD(&m->பொருட்கள்);

	திரும்பு m;
}

/*
 * menu_destroy() - frees the memory used by a menu and its items.
 *
 * m - Points to a menu created by menu_create().
 *
 * Returns 1 if successful, or -EINVAL if m is NULL.
 */
முழு menu_destroy(கட்டமைப்பு menu *m)
{
	எனில் (!m)
		திரும்பு -EINVAL;

	menu_பொருட்கள்_iter(m, menu_item_destroy, வெற்று);

	எனில் (m->title)
		விடுவி(m->title);

	விடுவி(m);

	திரும்பு 1;
}

நிலையான முழு bootmenu_conv_shortcut_சாவி(கட்டமைப்பு bootmenu_data *menu, முழு ichar)
{
	முழு shortcut_சாவி;

	ichar = tolower(ichar);
	தேர்வு (ichar) {
	/* a-z for bootmenu entry > 9 */
	நிலை 'a' ... 'z':
		shortcut_சாவி = ichar - 'a' + 9;
		நிறுத்து;
	/* 1-9 for bootmenu entry <= 9 */
	நிலை '1' ... '9':
		shortcut_சாவி = ichar - '1';
		நிறுத்து;
	/* Reserve 0 for last option (aka Exit) */
	நிலை '0':
	இயல்பு:
		திரும்பு -1;
	}

	திரும்பு shortcut_சாவி;
}

எண்ணிக்கை bootmenu_சாவி bootmenu_autoboot_loop(கட்டமைப்பு bootmenu_data *menu,
					 கட்டமைப்பு cli_ch_state *cch)
{
	எண்ணிக்கை bootmenu_சாவி சாவி = BKEY_NONE;
	முழு i, c;

	போது (menu->delay > 0) {
		முழு ichar;

		எனில் (ansi)
			printf(ANSI_CURSOR_POSITION, menu->மொத்தம் + 5, 3);
		printf("Hit any key to stop autoboot: %d ", menu->delay);
		க்கு (i = 0; i < 100; ++i) {
			எனில் (!tstc()) {
				schedule();
				mdelay(10);
				தொடர்;
			}

			menu->delay = -1;
			c = getchar();

			ichar = cli_ch_process(cch, c);

			தேர்வு (ichar) {
			நிலை '\0':
				சாவி = BKEY_NONE;
				நிறுத்து;
			நிலை '\n':
				சாவி = BKEY_SELECT;
				நிறுத்து;
			நிலை 0x3: /* ^C */
				சாவி = BKEY_QUIT;
				நிறுத்து;
			நிலை 'A' ... 'Z':
			நிலை 'a' ... 'z':
			நிலை '0' ... '9':
				சாவி = BKEY_SHORTCUT;
				நிறுத்து;
			இயல்பு:
				சாவி = BKEY_NONE;
				நிறுத்து;
			}
			நிறுத்து;
		}

		எனில் (சாவி == BKEY_SHORTCUT)
			cch->shortcut_சாவி = bootmenu_conv_shortcut_சாவி(menu, ichar);

		எனில் (menu->delay < 0)
			நிறுத்து;

		--menu->delay;
	}

	எனில் (ansi)
		printf(ANSI_CURSOR_POSITION ANSI_CLEAR_வரி, menu->மொத்தம் + 5, 1);

	எனில் (menu->delay == 0)
		சாவி = BKEY_SELECT;

	திரும்பு சாவி;
}

எண்ணிக்கை bootmenu_சாவி bootmenu_conv_சாவி(முழு ichar)
{
	எண்ணிக்கை bootmenu_சாவி சாவி;

	தேர்வு (ichar) {
	நிலை '\n':
		/* enter key was pressed */
		சாவி = BKEY_SELECT;
		நிறுத்து;
	நிலை CTL_CH('c'):
	நிலை '\e':
		/* ^C was pressed */
		சாவி = BKEY_QUIT;
		நிறுத்து;
	நிலை CTL_CH('p'):
		சாவி = BKEY_UP;
		நிறுத்து;
	நிலை CTL_CH('n'):
		சாவி = BKEY_DOWN;
		நிறுத்து;
	நிலை CTL_CH('s'):
		சாவி = BKEY_SAVE;
		நிறுத்து;
	நிலை '+':
		சாவி = BKEY_PLUS;
		நிறுத்து;
	நிலை '-':
		சாவி = BKEY_MINUS;
		நிறுத்து;
	நிலை ' ':
		சாவி = BKEY_SPACE;
		நிறுத்து;
	இயல்பு:
		சாவி = BKEY_NONE;
		நிறுத்து;
	}

	திரும்பு சாவி;
}

எண்ணிக்கை bootmenu_சாவி bootmenu_loop(கட்டமைப்பு bootmenu_data *menu,
				கட்டமைப்பு cli_ch_state *cch)
{
	எண்ணிக்கை bootmenu_சாவி சாவி;
	முழு c, errchar = 0;

	c = cli_ch_process(cch, 0);
	எனில் (!c) {
		போது (!c && !tstc()) {
			schedule();
			mdelay(10);
			c = cli_ch_process(cch, errchar);
			errchar = -ETIMEDOUT;
		}
		எனில் (!c) {
			c = getchar();
			c = cli_ch_process(cch, c);
		}
	}

	சாவி = bootmenu_conv_சாவி(c);

	எனில் (சாவி == BKEY_NONE && isalnum(c)) {
		சாவி = BKEY_SHORTCUT;
		cch->shortcut_சாவி = bootmenu_conv_shortcut_சாவி(menu, c);
	}

	திரும்பு சாவி;
}
