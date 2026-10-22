// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2010-2011 Calxeda, Inc.
 * Copyright (c) 2019, NVIDIA CORPORATION. All rights reserved.
 */

#અન્તર્ભાવ <ansi.h>
#અન્તર્ભાવ <cli.h>
#અન્તર્ભાવ <malloc.h>
#અન્તર્ભાવ <errno.h>
#અન્તર્ભાવ <linux/ctype.h>
#અન્તર્ભાવ <linux/delay.h>
#અન્તર્ભાવ <linux/list.h>
#અન્તર્ભાવ <watchdog.h>

#અન્તર્ભાવ "menu.h"

#define ansi 1

/*
 * Internally, each item in a menu is represented by a struct menu_item.
 *
 * These items will be alloc'd and initialized by menu_item_add and destroyed
 * by menu_item_destroy, and the consumer of the interface never sees that
 * this struct is used at all.
 */
સંરચના menu_item {
	અક્ષર *ચાવી;
	શૂન્ય *data;
	સંરચના list_head list;
};

/*
 * The menu is composed of a list of items along with settings and callbacks
 * provided by the user. An incomplete definition of this struct is available
 * in menu.h, but the full definition is here to prevent consumers from
 * relying on its contents.
 */
સંરચના menu {
	સંરચના menu_item *default_item;
	પૂર્ણાંક timeout;
	અક્ષર *title;
	પૂર્ણાંક prompt;
	શૂન્ય (*display_statusline)(સંરચના menu *);
	શૂન્ય (*item_data_print)(શૂન્ય *);
	અક્ષર *(*item_choice)(શૂન્ય *);
	bool (*need_reprint)(શૂન્ય *);
	શૂન્ય *item_choice_data;
	સંરચના list_head વસ્તુઓ;
	પૂર્ણાંક item_cnt;
};

/*
 * An iterator function for menu items. callback will be called for each item
 * in m, with m, a pointer to the item, and extra being passed to callback. If
 * callback returns a value other than NULL, iteration stops and the value
 * return by callback is returned from menu_items_iter.  This allows it to be
 * used for search type operations. It is also safe for callback to remove the
 * item from the list of items.
 */
સ્થિર અંતઃરેખ શૂન્ય *menu_વસ્તુઓ_iter(સંરચના menu *m,
		શૂન્ય *(*callback)(સંરચના menu *, સંરચના menu_item *, શૂન્ય *),
		શૂન્ય *extra)
{
	સંરચના list_head *pos, *n;
	સંરચના menu_item *item;
	શૂન્ય *ret;

	list_for_each_safe(pos, n, &m->વસ્તુઓ) {
		item = list_entry(pos, સંરચના menu_item, list);

		ret = callback(m, item, extra);

		જો (ret)
			પાછા ret;
	}

	પાછા ખાલી;
}

/*
 * Print a menu_item. If the consumer provided an item_data_print function
 * when creating the menu, call it with a pointer to the item's private data.
 * Otherwise, print the key of the item.
 */
સ્થિર અંતઃરેખ શૂન્ય *menu_item_print(સંરચના menu *m,
				સંરચના menu_item *item,
				શૂન્ય *extra)
{
	જો (!m->item_data_print) {
		puts(item->ચાવી);
		putc('\n');
	} નહીંતર {
		m->item_data_print(item->data);
	}

	પાછા ખાલી;
}

/*
 * Free the memory used by a menu item. This includes the memory used by its
 * key.
 */
સ્થિર અંતઃરેખ શૂન્ય *menu_item_destroy(સંરચના menu *m,
				સંરચના menu_item *item,
				શૂન્ય *extra)
{
	જો (item->ચાવી)
		મુક્ત(item->ચાવી);

	મુક્ત(item);

	પાછા ખાલી;
}

/*
 * Display a menu so the user can make a choice of an item. First display its
 * title, if any, and then each item in the menu.
 */
સ્થિર અંતઃરેખ શૂન્ય menu_display(સંરચના menu *m)
{
	જો (m->need_reprint) {
		જો (!m->need_reprint(m->item_choice_data))
			પાછા;
	}

	જો (m->title) {
		puts(m->title);
		putc('\n');
	}
	જો (m->display_statusline)
		m->display_statusline(m);

	menu_વસ્તુઓ_iter(m, menu_item_print, ખાલી);
}

/*
 * Check if an item's key matches a provided string, pointed to by extra. If
 * extra is NULL, an item with a NULL key will match. Otherwise, the item's
 * key has to match according to strcmp.
 *
 * This is called via menu_items_iter, so it returns a pointer to the item if
 * the key matches, and returns NULL otherwise.
 */
સ્થિર અંતઃરેખ શૂન્ય *menu_item_ચાવી_match(સંરચના menu *m,
			સંરચના menu_item *item, શૂન્ય *extra)
{
	અક્ષર *item_ચાવી = extra;

	જો (!item_ચાવી || !item->ચાવી) {
		જો (item_ચાવી == item->ચાવી)
			પાછા item;

		પાછા ખાલી;
	}

	જો (strcmp(item->ચાવી, item_ચાવી) == 0)
		પાછા item;

	પાછા ખાલી;
}

/*
 * Find the first item with a key matching item_key, if any exists.
 */
સ્થિર અંતઃરેખ સંરચના menu_item *menu_item_by_ચાવી(સંરચના menu *m,
							અક્ષર *item_ચાવી)
{
	પાછા menu_વસ્તુઓ_iter(m, menu_item_ચાવી_match, item_ચાવી);
}

/*
 * Set *choice to point to the default item's data, if any default item was
 * set, and returns 1. If no default item was set, returns -ENOENT.
 */
પૂર્ણાંક menu_default_choice(સંરચના menu *m, શૂન્ય **choice)
{
	જો (m->default_item) {
		*choice = m->default_item->data;
		પાછા 1;
	}

	પાછા -ENOENT;
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
સ્થિર અંતઃરેખ પૂર્ણાંક menu_interactive_choice(સંરચના menu *m, શૂન્ય **choice)
{
	અક્ષર cbuf[CONFIG_SYS_CBSIZE];
	સંરચના menu_item *choice_item = ખાલી;
	પૂર્ણાંક readret;

	જ્યારે (!choice_item) {
		cbuf[0] = '\0';

		menu_display(m);

		જો (!m->item_choice) {
			readret = cli_readline_into_buffer("Enter choice: ",
							   cbuf, m->timeout);

			જો (readret >= 0) {
				choice_item = menu_item_by_ચાવી(m, cbuf);
				જો (!choice_item)
					printf("%s not found\n", cbuf);
			} નહીંતર જો (readret == -1)  {
				printf("<INTERRUPT>\n");
				પાછા -EINTR;
			} નહીંતર {
				પાછા menu_default_choice(m, choice);
			}
		} નહીંતર {
			અક્ષર *ચાવી = m->item_choice(m->item_choice_data);

			જો (ચાવી)
				choice_item = menu_item_by_ચાવી(m, ચાવી);
		}

		જો (!choice_item)
			m->timeout = 0;
	}

	*choice = choice_item->data;

	પાછા 1;
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
પૂર્ણાંક menu_default_set(સંરચના menu *m, અક્ષર *item_ચાવી)
{
	સંરચના menu_item *item;

	જો (!m)
		પાછા -EINVAL;

	item = menu_item_by_ચાવી(m, item_ચાવી);

	જો (!item)
		પાછા -ENOENT;

	m->default_item = item;

	પાછા 1;
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
પૂર્ણાંક menu_get_choice(સંરચના menu *m, શૂન્ય **choice)
{
	જો (!m || !choice)
		પાછા -EINVAL;

	જો (!m->item_cnt)
		પાછા -ENOENT;

	જો (!m->prompt)
		પાછા menu_default_choice(m, choice);

	પાછા menu_interactive_choice(m, choice);
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
પૂર્ણાંક menu_item_add(સંરચના menu *m, અક્ષર *item_ચાવી, શૂન્ય *item_data)
{
	સંરચના menu_item *item;

	જો (!m)
		પાછા -EINVAL;

	item = menu_item_by_ચાવી(m, item_ચાવી);

	જો (item) {
		item->data = item_data;
		પાછા 1;
	}

	item = malloc(કદ *item);
	જો (!item)
		પાછા -ENOMEM;

	item->ચાવી = strdup(item_ચાવી);

	જો (!item->ચાવી) {
		મુક્ત(item);
		પાછા -ENOMEM;
	}

	item->data = item_data;

	list_add_tail(&item->list, &m->વસ્તુઓ);
	m->item_cnt++;

	પાછા 1;
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
સંરચના menu *menu_create(અક્ષર *title, પૂર્ણાંક timeout, પૂર્ણાંક prompt,
				શૂન્ય (*display_statusline)(સંરચના menu *),
				શૂન્ય (*item_data_print)(શૂન્ય *),
				અક્ષર *(*item_choice)(શૂન્ય *),
				bool (*need_reprint)(શૂન્ય *),
				શૂન્ય *item_choice_data)
{
	સંરચના menu *m;

	m = malloc(કદ *m);

	જો (!m)
		પાછા ખાલી;

	m->default_item = ખાલી;
	m->prompt = prompt;
	m->timeout = timeout;
	m->display_statusline = display_statusline;
	m->item_data_print = item_data_print;
	m->item_choice = item_choice;
	m->need_reprint = need_reprint;
	m->item_choice_data = item_choice_data;
	m->item_cnt = 0;

	જો (title) {
		m->title = strdup(title);
		જો (!m->title) {
			મુક્ત(m);
			પાછા ખાલી;
		}
	} નહીંતર
		m->title = ખાલી;

	INIT_LIST_HEAD(&m->વસ્તુઓ);

	પાછા m;
}

/*
 * menu_destroy() - frees the memory used by a menu and its items.
 *
 * m - Points to a menu created by menu_create().
 *
 * Returns 1 if successful, or -EINVAL if m is NULL.
 */
પૂર્ણાંક menu_destroy(સંરચના menu *m)
{
	જો (!m)
		પાછા -EINVAL;

	menu_વસ્તુઓ_iter(m, menu_item_destroy, ખાલી);

	જો (m->title)
		મુક્ત(m->title);

	મુક્ત(m);

	પાછા 1;
}

સ્થિર પૂર્ણાંક bootmenu_conv_shortcut_ચાવી(સંરચના bootmenu_data *menu, પૂર્ણાંક ichar)
{
	પૂર્ણાંક shortcut_ચાવી;

	ichar = tolower(ichar);
	પસંદગી (ichar) {
	/* a-z for bootmenu entry > 9 */
	સ્થિતિ 'a' ... 'z':
		shortcut_ચાવી = ichar - 'a' + 9;
		તોડો;
	/* 1-9 for bootmenu entry <= 9 */
	સ્થિતિ '1' ... '9':
		shortcut_ચાવી = ichar - '1';
		તોડો;
	/* Reserve 0 for last option (aka Exit) */
	સ્થિતિ '0':
	મૂળભૂત:
		પાછા -1;
	}

	પાછા shortcut_ચાવી;
}

ગણના bootmenu_ચાવી bootmenu_autoboot_loop(સંરચના bootmenu_data *menu,
					 સંરચના cli_ch_state *cch)
{
	ગણના bootmenu_ચાવી ચાવી = BKEY_NONE;
	પૂર્ણાંક i, c;

	જ્યારે (menu->delay > 0) {
		પૂર્ણાંક ichar;

		જો (ansi)
			printf(ANSI_CURSOR_POSITION, menu->ગણતરી + 5, 3);
		printf("Hit any key to stop autoboot: %d ", menu->delay);
		માટે (i = 0; i < 100; ++i) {
			જો (!tstc()) {
				schedule();
				mdelay(10);
				ચાલુ;
			}

			menu->delay = -1;
			c = getchar();

			ichar = cli_ch_process(cch, c);

			પસંદગી (ichar) {
			સ્થિતિ '\0':
				ચાવી = BKEY_NONE;
				તોડો;
			સ્થિતિ '\n':
				ચાવી = BKEY_SELECT;
				તોડો;
			સ્થિતિ 0x3: /* ^C */
				ચાવી = BKEY_QUIT;
				તોડો;
			સ્થિતિ 'A' ... 'Z':
			સ્થિતિ 'a' ... 'z':
			સ્થિતિ '0' ... '9':
				ચાવી = BKEY_SHORTCUT;
				તોડો;
			મૂળભૂત:
				ચાવી = BKEY_NONE;
				તોડો;
			}
			તોડો;
		}

		જો (ચાવી == BKEY_SHORTCUT)
			cch->shortcut_ચાવી = bootmenu_conv_shortcut_ચાવી(menu, ichar);

		જો (menu->delay < 0)
			તોડો;

		--menu->delay;
	}

	જો (ansi)
		printf(ANSI_CURSOR_POSITION ANSI_CLEAR_લીટી, menu->ગણતરી + 5, 1);

	જો (menu->delay == 0)
		ચાવી = BKEY_SELECT;

	પાછા ચાવી;
}

ગણના bootmenu_ચાવી bootmenu_conv_ચાવી(પૂર્ણાંક ichar)
{
	ગણના bootmenu_ચાવી ચાવી;

	પસંદગી (ichar) {
	સ્થિતિ '\n':
		/* enter key was pressed */
		ચાવી = BKEY_SELECT;
		તોડો;
	સ્થિતિ CTL_CH('c'):
	સ્થિતિ '\e':
		/* ^C was pressed */
		ચાવી = BKEY_QUIT;
		તોડો;
	સ્થિતિ CTL_CH('p'):
		ચાવી = BKEY_UP;
		તોડો;
	સ્થિતિ CTL_CH('n'):
		ચાવી = BKEY_DOWN;
		તોડો;
	સ્થિતિ CTL_CH('s'):
		ચાવી = BKEY_SAVE;
		તોડો;
	સ્થિતિ '+':
		ચાવી = BKEY_PLUS;
		તોડો;
	સ્થિતિ '-':
		ચાવી = BKEY_MINUS;
		તોડો;
	સ્થિતિ ' ':
		ચાવી = BKEY_SPACE;
		તોડો;
	મૂળભૂત:
		ચાવી = BKEY_NONE;
		તોડો;
	}

	પાછા ચાવી;
}

ગણના bootmenu_ચાવી bootmenu_loop(સંરચના bootmenu_data *menu,
				સંરચના cli_ch_state *cch)
{
	ગણના bootmenu_ચાવી ચાવી;
	પૂર્ણાંક c, errchar = 0;

	c = cli_ch_process(cch, 0);
	જો (!c) {
		જ્યારે (!c && !tstc()) {
			schedule();
			mdelay(10);
			c = cli_ch_process(cch, errchar);
			errchar = -ETIMEDOUT;
		}
		જો (!c) {
			c = getchar();
			c = cli_ch_process(cch, c);
		}
	}

	ચાવી = bootmenu_conv_ચાવી(c);

	જો (ચાવી == BKEY_NONE && isalnum(c)) {
		ચાવી = BKEY_SHORTCUT;
		cch->shortcut_ચાવી = bootmenu_conv_shortcut_ચાવી(menu, c);
	}

	પાછા ચાવી;
}
