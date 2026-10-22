// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2010-2011 Calxeda, Inc.
 * Copyright (c) 2019, NVIDIA CORPORATION. All rights reserved.
 */

#ଅନ୍ତର୍ଭାଵ <ansi.h>
#ଅନ୍ତର୍ଭାଵ <cli.h>
#ଅନ୍ତର୍ଭାଵ <malloc.h>
#ଅନ୍ତର୍ଭାଵ <errno.h>
#ଅନ୍ତର୍ଭାଵ <linux/ctype.h>
#ଅନ୍ତର୍ଭାଵ <linux/delay.h>
#ଅନ୍ତର୍ଭାଵ <linux/list.h>
#ଅନ୍ତର୍ଭାଵ <watchdog.h>

#ଅନ୍ତର୍ଭାଵ "menu.h"

#define ansi 1

/*
 * Internally, each item in a menu is represented by a struct menu_item.
 *
 * These items will be alloc'd and initialized by menu_item_add and destroyed
 * by menu_item_destroy, and the consumer of the interface never sees that
 * this struct is used at all.
 */
ସଂରଚନା menu_item {
	ଅକ୍ଷର *ଚାବି;
	ଶୂନ୍ୟ *data;
	ସଂରଚନା list_head list;
};

/*
 * The menu is composed of a list of items along with settings and callbacks
 * provided by the user. An incomplete definition of this struct is available
 * in menu.h, but the full definition is here to prevent consumers from
 * relying on its contents.
 */
ସଂରଚନା menu {
	ସଂରଚନା menu_item *default_item;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା timeout;
	ଅକ୍ଷର *title;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା prompt;
	ଶୂନ୍ୟ (*display_statusline)(ସଂରଚନା menu *);
	ଶୂନ୍ୟ (*item_data_print)(ଶୂନ୍ୟ *);
	ଅକ୍ଷର *(*item_choice)(ଶୂନ୍ୟ *);
	bool (*need_reprint)(ଶୂନ୍ୟ *);
	ଶୂନ୍ୟ *item_choice_data;
	ସଂରଚନା list_head ବସ୍ତୁଗୁଡ଼ିକ;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା item_cnt;
};

/*
 * An iterator function for menu items. callback will be called for each item
 * in m, with m, a pointer to the item, and extra being passed to callback. If
 * callback returns a value other than NULL, iteration stops and the value
 * return by callback is returned from menu_items_iter.  This allows it to be
 * used for search type operations. It is also safe for callback to remove the
 * item from the list of items.
 */
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ *menu_ବସ୍ତୁଗୁଡ଼ିକ_iter(ସଂରଚନା menu *m,
		ଶୂନ୍ୟ *(*callback)(ସଂରଚନା menu *, ସଂରଚନା menu_item *, ଶୂନ୍ୟ *),
		ଶୂନ୍ୟ *extra)
{
	ସଂରଚନା list_head *pos, *n;
	ସଂରଚନା menu_item *item;
	ଶୂନ୍ୟ *ret;

	list_for_each_safe(pos, n, &m->ବସ୍ତୁଗୁଡ଼ିକ) {
		item = list_entry(pos, ସଂରଚନା menu_item, list);

		ret = callback(m, item, extra);

		ଯଦି (ret)
			ଫେରନ୍ତୁ ret;
	}

	ଫେରନ୍ତୁ ଖାଲି;
}

/*
 * Print a menu_item. If the consumer provided an item_data_print function
 * when creating the menu, call it with a pointer to the item's private data.
 * Otherwise, print the key of the item.
 */
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ *menu_item_print(ସଂରଚନା menu *m,
				ସଂରଚନା menu_item *item,
				ଶୂନ୍ୟ *extra)
{
	ଯଦି (!m->item_data_print) {
		puts(item->ଚାବି);
		putc('\n');
	} ଅନ୍ୟଥା {
		m->item_data_print(item->data);
	}

	ଫେରନ୍ତୁ ଖାଲି;
}

/*
 * Free the memory used by a menu item. This includes the memory used by its
 * key.
 */
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ *menu_item_destroy(ସଂରଚନା menu *m,
				ସଂରଚନା menu_item *item,
				ଶୂନ୍ୟ *extra)
{
	ଯଦି (item->ଚାବି)
		ମୁକ୍ତ(item->ଚାବି);

	ମୁକ୍ତ(item);

	ଫେରନ୍ତୁ ଖାଲି;
}

/*
 * Display a menu so the user can make a choice of an item. First display its
 * title, if any, and then each item in the menu.
 */
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ menu_display(ସଂରଚନା menu *m)
{
	ଯଦି (m->need_reprint) {
		ଯଦି (!m->need_reprint(m->item_choice_data))
			ଫେରନ୍ତୁ;
	}

	ଯଦି (m->title) {
		puts(m->title);
		putc('\n');
	}
	ଯଦି (m->display_statusline)
		m->display_statusline(m);

	menu_ବସ୍ତୁଗୁଡ଼ିକ_iter(m, menu_item_print, ଖାଲି);
}

/*
 * Check if an item's key matches a provided string, pointed to by extra. If
 * extra is NULL, an item with a NULL key will match. Otherwise, the item's
 * key has to match according to strcmp.
 *
 * This is called via menu_items_iter, so it returns a pointer to the item if
 * the key matches, and returns NULL otherwise.
 */
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ *menu_item_ଚାବି_match(ସଂରଚନା menu *m,
			ସଂରଚନା menu_item *item, ଶୂନ୍ୟ *extra)
{
	ଅକ୍ଷର *item_ଚାବି = extra;

	ଯଦି (!item_ଚାବି || !item->ଚାବି) {
		ଯଦି (item_ଚାବି == item->ଚାବି)
			ଫେରନ୍ତୁ item;

		ଫେରନ୍ତୁ ଖାଲି;
	}

	ଯଦି (strcmp(item->ଚାବି, item_ଚାବି) == 0)
		ଫେରନ୍ତୁ item;

	ଫେରନ୍ତୁ ଖାଲି;
}

/*
 * Find the first item with a key matching item_key, if any exists.
 */
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ସଂରଚନା menu_item *menu_item_by_ଚାବି(ସଂରଚନା menu *m,
							ଅକ୍ଷର *item_ଚାବି)
{
	ଫେରନ୍ତୁ menu_ବସ୍ତୁଗୁଡ଼ିକ_iter(m, menu_item_ଚାବି_match, item_ଚାବି);
}

/*
 * Set *choice to point to the default item's data, if any default item was
 * set, and returns 1. If no default item was set, returns -ENOENT.
 */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା menu_default_choice(ସଂରଚନା menu *m, ଶୂନ୍ୟ **choice)
{
	ଯଦି (m->default_item) {
		*choice = m->default_item->data;
		ଫେରନ୍ତୁ 1;
	}

	ଫେରନ୍ତୁ -ENOENT;
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
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ପୂର୍ଣ୍ଣସଂଖ୍ୟା menu_interactive_choice(ସଂରଚନା menu *m, ଶୂନ୍ୟ **choice)
{
	ଅକ୍ଷର cbuf[CONFIG_SYS_CBSIZE];
	ସଂରଚନା menu_item *choice_item = ଖାଲି;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା readret;

	ଯେତେବେଳେ (!choice_item) {
		cbuf[0] = '\0';

		menu_display(m);

		ଯଦି (!m->item_choice) {
			readret = cli_readline_into_buffer("Enter choice: ",
							   cbuf, m->timeout);

			ଯଦି (readret >= 0) {
				choice_item = menu_item_by_ଚାବି(m, cbuf);
				ଯଦି (!choice_item)
					printf("%s not found\n", cbuf);
			} ଅନ୍ୟଥା ଯଦି (readret == -1)  {
				printf("<INTERRUPT>\n");
				ଫେରନ୍ତୁ -EINTR;
			} ଅନ୍ୟଥା {
				ଫେରନ୍ତୁ menu_default_choice(m, choice);
			}
		} ଅନ୍ୟଥା {
			ଅକ୍ଷର *ଚାବି = m->item_choice(m->item_choice_data);

			ଯଦି (ଚାବି)
				choice_item = menu_item_by_ଚାବି(m, ଚାବି);
		}

		ଯଦି (!choice_item)
			m->timeout = 0;
	}

	*choice = choice_item->data;

	ଫେରନ୍ତୁ 1;
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
ପୂର୍ଣ୍ଣସଂଖ୍ୟା menu_default_set(ସଂରଚନା menu *m, ଅକ୍ଷର *item_ଚାବି)
{
	ସଂରଚନା menu_item *item;

	ଯଦି (!m)
		ଫେରନ୍ତୁ -EINVAL;

	item = menu_item_by_ଚାବି(m, item_ଚାବି);

	ଯଦି (!item)
		ଫେରନ୍ତୁ -ENOENT;

	m->default_item = item;

	ଫେରନ୍ତୁ 1;
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
ପୂର୍ଣ୍ଣସଂଖ୍ୟା menu_get_choice(ସଂରଚନା menu *m, ଶୂନ୍ୟ **choice)
{
	ଯଦି (!m || !choice)
		ଫେରନ୍ତୁ -EINVAL;

	ଯଦି (!m->item_cnt)
		ଫେରନ୍ତୁ -ENOENT;

	ଯଦି (!m->prompt)
		ଫେରନ୍ତୁ menu_default_choice(m, choice);

	ଫେରନ୍ତୁ menu_interactive_choice(m, choice);
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
ପୂର୍ଣ୍ଣସଂଖ୍ୟା menu_item_add(ସଂରଚନା menu *m, ଅକ୍ଷର *item_ଚାବି, ଶୂନ୍ୟ *item_data)
{
	ସଂରଚନା menu_item *item;

	ଯଦି (!m)
		ଫେରନ୍ତୁ -EINVAL;

	item = menu_item_by_ଚାବି(m, item_ଚାବି);

	ଯଦି (item) {
		item->data = item_data;
		ଫେରନ୍ତୁ 1;
	}

	item = malloc(ଆକାର *item);
	ଯଦି (!item)
		ଫେରନ୍ତୁ -ENOMEM;

	item->ଚାବି = strdup(item_ଚାବି);

	ଯଦି (!item->ଚାବି) {
		ମୁକ୍ତ(item);
		ଫେରନ୍ତୁ -ENOMEM;
	}

	item->data = item_data;

	list_add_tail(&item->list, &m->ବସ୍ତୁଗୁଡ଼ିକ);
	m->item_cnt++;

	ଫେରନ୍ତୁ 1;
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
ସଂରଚନା menu *menu_create(ଅକ୍ଷର *title, ପୂର୍ଣ୍ଣସଂଖ୍ୟା timeout, ପୂର୍ଣ୍ଣସଂଖ୍ୟା prompt,
				ଶୂନ୍ୟ (*display_statusline)(ସଂରଚନା menu *),
				ଶୂନ୍ୟ (*item_data_print)(ଶୂନ୍ୟ *),
				ଅକ୍ଷର *(*item_choice)(ଶୂନ୍ୟ *),
				bool (*need_reprint)(ଶୂନ୍ୟ *),
				ଶୂନ୍ୟ *item_choice_data)
{
	ସଂରଚନା menu *m;

	m = malloc(ଆକାର *m);

	ଯଦି (!m)
		ଫେରନ୍ତୁ ଖାଲି;

	m->default_item = ଖାଲି;
	m->prompt = prompt;
	m->timeout = timeout;
	m->display_statusline = display_statusline;
	m->item_data_print = item_data_print;
	m->item_choice = item_choice;
	m->need_reprint = need_reprint;
	m->item_choice_data = item_choice_data;
	m->item_cnt = 0;

	ଯଦି (title) {
		m->title = strdup(title);
		ଯଦି (!m->title) {
			ମୁକ୍ତ(m);
			ଫେରନ୍ତୁ ଖାଲି;
		}
	} ଅନ୍ୟଥା
		m->title = ଖାଲି;

	INIT_LIST_HEAD(&m->ବସ୍ତୁଗୁଡ଼ିକ);

	ଫେରନ୍ତୁ m;
}

/*
 * menu_destroy() - frees the memory used by a menu and its items.
 *
 * m - Points to a menu created by menu_create().
 *
 * Returns 1 if successful, or -EINVAL if m is NULL.
 */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା menu_destroy(ସଂରଚନା menu *m)
{
	ଯଦି (!m)
		ଫେରନ୍ତୁ -EINVAL;

	menu_ବସ୍ତୁଗୁଡ଼ିକ_iter(m, menu_item_destroy, ଖାଲି);

	ଯଦି (m->title)
		ମୁକ୍ତ(m->title);

	ମୁକ୍ତ(m);

	ଫେରନ୍ତୁ 1;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା bootmenu_conv_shortcut_ଚାବି(ସଂରଚନା bootmenu_data *menu, ପୂର୍ଣ୍ଣସଂଖ୍ୟା ichar)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା shortcut_ଚାବି;

	ichar = tolower(ichar);
	ବାଛ (ichar) {
	/* a-z for bootmenu entry > 9 */
	ସ୍ଥିତି 'a' ... 'z':
		shortcut_ଚାବି = ichar - 'a' + 9;
		ଭାଙ୍ଗ;
	/* 1-9 for bootmenu entry <= 9 */
	ସ୍ଥିତି '1' ... '9':
		shortcut_ଚାବି = ichar - '1';
		ଭାଙ୍ଗ;
	/* Reserve 0 for last option (aka Exit) */
	ସ୍ଥିତି '0':
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ଫେରନ୍ତୁ -1;
	}

	ଫେରନ୍ତୁ shortcut_ଚାବି;
}

ଗଣନା bootmenu_ଚାବି bootmenu_autoboot_loop(ସଂରଚନା bootmenu_data *menu,
					 ସଂରଚନା cli_ch_state *cch)
{
	ଗଣନା bootmenu_ଚାବି ଚାବି = BKEY_NONE;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, c;

	ଯେତେବେଳେ (menu->delay > 0) {
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା ichar;

		ଯଦି (ansi)
			printf(ANSI_CURSOR_POSITION, menu->ଗଣନା_2 + 5, 3);
		printf("Hit any key to stop autoboot: %d ", menu->delay);
		ପାଇଁ (i = 0; i < 100; ++i) {
			ଯଦି (!tstc()) {
				schedule();
				mdelay(10);
				ଜାରି;
			}

			menu->delay = -1;
			c = getchar();

			ichar = cli_ch_process(cch, c);

			ବାଛ (ichar) {
			ସ୍ଥିତି '\0':
				ଚାବି = BKEY_NONE;
				ଭାଙ୍ଗ;
			ସ୍ଥିତି '\n':
				ଚାବି = BKEY_SELECT;
				ଭାଙ୍ଗ;
			ସ୍ଥିତି 0x3: /* ^C */
				ଚାବି = BKEY_QUIT;
				ଭାଙ୍ଗ;
			ସ୍ଥିତି 'A' ... 'Z':
			ସ୍ଥିତି 'a' ... 'z':
			ସ୍ଥିତି '0' ... '9':
				ଚାବି = BKEY_SHORTCUT;
				ଭାଙ୍ଗ;
			ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
				ଚାବି = BKEY_NONE;
				ଭାଙ୍ଗ;
			}
			ଭାଙ୍ଗ;
		}

		ଯଦି (ଚାବି == BKEY_SHORTCUT)
			cch->shortcut_ଚାବି = bootmenu_conv_shortcut_ଚାବି(menu, ichar);

		ଯଦି (menu->delay < 0)
			ଭାଙ୍ଗ;

		--menu->delay;
	}

	ଯଦି (ansi)
		printf(ANSI_CURSOR_POSITION ANSI_CLEAR_ଧାଡ଼ି, menu->ଗଣନା_2 + 5, 1);

	ଯଦି (menu->delay == 0)
		ଚାବି = BKEY_SELECT;

	ଫେରନ୍ତୁ ଚାବି;
}

ଗଣନା bootmenu_ଚାବି bootmenu_conv_ଚାବି(ପୂର୍ଣ୍ଣସଂଖ୍ୟା ichar)
{
	ଗଣନା bootmenu_ଚାବି ଚାବି;

	ବାଛ (ichar) {
	ସ୍ଥିତି '\n':
		/* enter key was pressed */
		ଚାବି = BKEY_SELECT;
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CTL_CH('c'):
	ସ୍ଥିତି '\e':
		/* ^C was pressed */
		ଚାବି = BKEY_QUIT;
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CTL_CH('p'):
		ଚାବି = BKEY_UP;
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CTL_CH('n'):
		ଚାବି = BKEY_DOWN;
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CTL_CH('s'):
		ଚାବି = BKEY_SAVE;
		ଭାଙ୍ଗ;
	ସ୍ଥିତି '+':
		ଚାବି = BKEY_PLUS;
		ଭାଙ୍ଗ;
	ସ୍ଥିତି '-':
		ଚାବି = BKEY_MINUS;
		ଭାଙ୍ଗ;
	ସ୍ଥିତି ' ':
		ଚାବି = BKEY_SPACE;
		ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ଚାବି = BKEY_NONE;
		ଭାଙ୍ଗ;
	}

	ଫେରନ୍ତୁ ଚାବି;
}

ଗଣନା bootmenu_ଚାବି bootmenu_loop(ସଂରଚନା bootmenu_data *menu,
				ସଂରଚନା cli_ch_state *cch)
{
	ଗଣନା bootmenu_ଚାବି ଚାବି;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା c, errchar = 0;

	c = cli_ch_process(cch, 0);
	ଯଦି (!c) {
		ଯେତେବେଳେ (!c && !tstc()) {
			schedule();
			mdelay(10);
			c = cli_ch_process(cch, errchar);
			errchar = -ETIMEDOUT;
		}
		ଯଦି (!c) {
			c = getchar();
			c = cli_ch_process(cch, c);
		}
	}

	ଚାବି = bootmenu_conv_ଚାବି(c);

	ଯଦି (ଚାବି == BKEY_NONE && isalnum(c)) {
		ଚାବି = BKEY_SHORTCUT;
		cch->shortcut_ଚାବି = bootmenu_conv_shortcut_ଚାବି(menu, c);
	}

	ଫେରନ୍ତୁ ଚାବି;
}
