// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2010-2011 Calxeda, Inc.
 * Copyright (c) 2019, NVIDIA CORPORATION. All rights reserved.
 */

#شامل <ansi.h>
#شامل <cli.h>
#شامل <malloc.h>
#شامل <errno.h>
#شامل <linux/ctype.h>
#شامل <linux/delay.h>
#شامل <linux/list.h>
#شامل <watchdog.h>

#شامل "menu.h"

#define ansi 1

/*
 * Internally, each item in a menu is represented by a struct menu_item.
 *
 * These items will be alloc'd and initialized by menu_item_add and destroyed
 * by menu_item_destroy, and the consumer of the interface never sees that
 * this struct is used at all.
 */
ساختار menu_item {
	نویسه *کلید;
	تهی *data;
	ساختار list_head list;
};

/*
 * The menu is composed of a list of items along with settings and callbacks
 * provided by the user. An incomplete definition of this struct is available
 * in menu.h, but the full definition is here to prevent consumers from
 * relying on its contents.
 */
ساختار menu {
	ساختار menu_item *default_item;
	صحیح timeout;
	نویسه *title;
	صحیح prompt;
	تهی (*display_statusline)(ساختار menu *);
	تهی (*item_data_print)(تهی *);
	نویسه *(*item_choice)(تهی *);
	bool (*need_reprint)(تهی *);
	تهی *item_choice_data;
	ساختار list_head اقلام;
	صحیح item_cnt;
};

/*
 * An iterator function for menu items. callback will be called for each item
 * in m, with m, a pointer to the item, and extra being passed to callback. If
 * callback returns a value other than NULL, iteration stops and the value
 * return by callback is returned from menu_items_iter.  This allows it to be
 * used for search type operations. It is also safe for callback to remove the
 * item from the list of items.
 */
ایستا درونخطی تهی *menu_اقلام_iter(ساختار menu *m,
		تهی *(*callback)(ساختار menu *, ساختار menu_item *, تهی *),
		تهی *extra)
{
	ساختار list_head *pos, *n;
	ساختار menu_item *item;
	تهی *ret;

	list_for_each_safe(pos, n, &m->اقلام) {
		item = list_entry(pos, ساختار menu_item, list);

		ret = callback(m, item, extra);

		اگر (ret)
			بازگشت ret;
	}

	بازگشت پوچ;
}

/*
 * Print a menu_item. If the consumer provided an item_data_print function
 * when creating the menu, call it with a pointer to the item's private data.
 * Otherwise, print the key of the item.
 */
ایستا درونخطی تهی *menu_item_print(ساختار menu *m,
				ساختار menu_item *item,
				تهی *extra)
{
	اگر (!m->item_data_print) {
		puts(item->کلید);
		putc('\n');
	} وگرنه {
		m->item_data_print(item->data);
	}

	بازگشت پوچ;
}

/*
 * Free the memory used by a menu item. This includes the memory used by its
 * key.
 */
ایستا درونخطی تهی *menu_item_destroy(ساختار menu *m,
				ساختار menu_item *item,
				تهی *extra)
{
	اگر (item->کلید)
		آزاد(item->کلید);

	آزاد(item);

	بازگشت پوچ;
}

/*
 * Display a menu so the user can make a choice of an item. First display its
 * title, if any, and then each item in the menu.
 */
ایستا درونخطی تهی menu_display(ساختار menu *m)
{
	اگر (m->need_reprint) {
		اگر (!m->need_reprint(m->item_choice_data))
			بازگشت;
	}

	اگر (m->title) {
		puts(m->title);
		putc('\n');
	}
	اگر (m->display_statusline)
		m->display_statusline(m);

	menu_اقلام_iter(m, menu_item_print, پوچ);
}

/*
 * Check if an item's key matches a provided string, pointed to by extra. If
 * extra is NULL, an item with a NULL key will match. Otherwise, the item's
 * key has to match according to strcmp.
 *
 * This is called via menu_items_iter, so it returns a pointer to the item if
 * the key matches, and returns NULL otherwise.
 */
ایستا درونخطی تهی *menu_item_کلید_match(ساختار menu *m,
			ساختار menu_item *item, تهی *extra)
{
	نویسه *item_کلید = extra;

	اگر (!item_کلید || !item->کلید) {
		اگر (item_کلید == item->کلید)
			بازگشت item;

		بازگشت پوچ;
	}

	اگر (strcmp(item->کلید, item_کلید) == 0)
		بازگشت item;

	بازگشت پوچ;
}

/*
 * Find the first item with a key matching item_key, if any exists.
 */
ایستا درونخطی ساختار menu_item *menu_item_by_کلید(ساختار menu *m,
							نویسه *item_کلید)
{
	بازگشت menu_اقلام_iter(m, menu_item_کلید_match, item_کلید);
}

/*
 * Set *choice to point to the default item's data, if any default item was
 * set, and returns 1. If no default item was set, returns -ENOENT.
 */
صحیح menu_default_choice(ساختار menu *m, تهی **choice)
{
	اگر (m->default_item) {
		*choice = m->default_item->data;
		بازگشت 1;
	}

	بازگشت -ENOENT;
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
ایستا درونخطی صحیح menu_interactive_choice(ساختار menu *m, تهی **choice)
{
	نویسه cbuf[CONFIG_SYS_CBSIZE];
	ساختار menu_item *choice_item = پوچ;
	صحیح readret;

	وقتی (!choice_item) {
		cbuf[0] = '\0';

		menu_display(m);

		اگر (!m->item_choice) {
			readret = cli_readline_into_buffer("Enter choice: ",
							   cbuf, m->timeout);

			اگر (readret >= 0) {
				choice_item = menu_item_by_کلید(m, cbuf);
				اگر (!choice_item)
					printf("%s not found\n", cbuf);
			} وگرنه اگر (readret == -1)  {
				printf("<INTERRUPT>\n");
				بازگشت -EINTR;
			} وگرنه {
				بازگشت menu_default_choice(m, choice);
			}
		} وگرنه {
			نویسه *کلید = m->item_choice(m->item_choice_data);

			اگر (کلید)
				choice_item = menu_item_by_کلید(m, کلید);
		}

		اگر (!choice_item)
			m->timeout = 0;
	}

	*choice = choice_item->data;

	بازگشت 1;
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
صحیح menu_default_set(ساختار menu *m, نویسه *item_کلید)
{
	ساختار menu_item *item;

	اگر (!m)
		بازگشت -EINVAL;

	item = menu_item_by_کلید(m, item_کلید);

	اگر (!item)
		بازگشت -ENOENT;

	m->default_item = item;

	بازگشت 1;
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
صحیح menu_get_choice(ساختار menu *m, تهی **choice)
{
	اگر (!m || !choice)
		بازگشت -EINVAL;

	اگر (!m->item_cnt)
		بازگشت -ENOENT;

	اگر (!m->prompt)
		بازگشت menu_default_choice(m, choice);

	بازگشت menu_interactive_choice(m, choice);
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
صحیح menu_item_add(ساختار menu *m, نویسه *item_کلید, تهی *item_data)
{
	ساختار menu_item *item;

	اگر (!m)
		بازگشت -EINVAL;

	item = menu_item_by_کلید(m, item_کلید);

	اگر (item) {
		item->data = item_data;
		بازگشت 1;
	}

	item = malloc(اندازه *item);
	اگر (!item)
		بازگشت -ENOMEM;

	item->کلید = strdup(item_کلید);

	اگر (!item->کلید) {
		آزاد(item);
		بازگشت -ENOMEM;
	}

	item->data = item_data;

	list_add_tail(&item->list, &m->اقلام);
	m->item_cnt++;

	بازگشت 1;
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
ساختار menu *menu_create(نویسه *title, صحیح timeout, صحیح prompt,
				تهی (*display_statusline)(ساختار menu *),
				تهی (*item_data_print)(تهی *),
				نویسه *(*item_choice)(تهی *),
				bool (*need_reprint)(تهی *),
				تهی *item_choice_data)
{
	ساختار menu *m;

	m = malloc(اندازه *m);

	اگر (!m)
		بازگشت پوچ;

	m->default_item = پوچ;
	m->prompt = prompt;
	m->timeout = timeout;
	m->display_statusline = display_statusline;
	m->item_data_print = item_data_print;
	m->item_choice = item_choice;
	m->need_reprint = need_reprint;
	m->item_choice_data = item_choice_data;
	m->item_cnt = 0;

	اگر (title) {
		m->title = strdup(title);
		اگر (!m->title) {
			آزاد(m);
			بازگشت پوچ;
		}
	} وگرنه
		m->title = پوچ;

	INIT_LIST_HEAD(&m->اقلام);

	بازگشت m;
}

/*
 * menu_destroy() - frees the memory used by a menu and its items.
 *
 * m - Points to a menu created by menu_create().
 *
 * Returns 1 if successful, or -EINVAL if m is NULL.
 */
صحیح menu_destroy(ساختار menu *m)
{
	اگر (!m)
		بازگشت -EINVAL;

	menu_اقلام_iter(m, menu_item_destroy, پوچ);

	اگر (m->title)
		آزاد(m->title);

	آزاد(m);

	بازگشت 1;
}

ایستا صحیح bootmenu_conv_shortcut_کلید(ساختار bootmenu_data *menu, صحیح ichar)
{
	صحیح shortcut_کلید;

	ichar = tolower(ichar);
	انتخاب (ichar) {
	/* a-z for bootmenu entry > 9 */
	حالت 'a' ... 'z':
		shortcut_کلید = ichar - 'a' + 9;
		بشکن;
	/* 1-9 for bootmenu entry <= 9 */
	حالت '1' ... '9':
		shortcut_کلید = ichar - '1';
		بشکن;
	/* Reserve 0 for last option (aka Exit) */
	حالت '0':
	پیشفرض:
		بازگشت -1;
	}

	بازگشت shortcut_کلید;
}

شمارش bootmenu_کلید bootmenu_autoboot_loop(ساختار bootmenu_data *menu,
					 ساختار cli_ch_state *cch)
{
	شمارش bootmenu_کلید کلید = BKEY_NONE;
	صحیح i, c;

	وقتی (menu->delay > 0) {
		صحیح ichar;

		اگر (ansi)
			printf(ANSI_CURSOR_POSITION, menu->شمار + 5, 3);
		printf("Hit any key to stop autoboot: %d ", menu->delay);
		برای (i = 0; i < 100; ++i) {
			اگر (!tstc()) {
				schedule();
				mdelay(10);
				ادامه;
			}

			menu->delay = -1;
			c = getchar();

			ichar = cli_ch_process(cch, c);

			انتخاب (ichar) {
			حالت '\0':
				کلید = BKEY_NONE;
				بشکن;
			حالت '\n':
				کلید = BKEY_SELECT;
				بشکن;
			حالت 0x3: /* ^C */
				کلید = BKEY_QUIT;
				بشکن;
			حالت 'A' ... 'Z':
			حالت 'a' ... 'z':
			حالت '0' ... '9':
				کلید = BKEY_SHORTCUT;
				بشکن;
			پیشفرض:
				کلید = BKEY_NONE;
				بشکن;
			}
			بشکن;
		}

		اگر (کلید == BKEY_SHORTCUT)
			cch->shortcut_کلید = bootmenu_conv_shortcut_کلید(menu, ichar);

		اگر (menu->delay < 0)
			بشکن;

		--menu->delay;
	}

	اگر (ansi)
		printf(ANSI_CURSOR_POSITION ANSI_CLEAR_سطر, menu->شمار + 5, 1);

	اگر (menu->delay == 0)
		کلید = BKEY_SELECT;

	بازگشت کلید;
}

شمارش bootmenu_کلید bootmenu_conv_کلید(صحیح ichar)
{
	شمارش bootmenu_کلید کلید;

	انتخاب (ichar) {
	حالت '\n':
		/* enter key was pressed */
		کلید = BKEY_SELECT;
		بشکن;
	حالت CTL_CH('c'):
	حالت '\e':
		/* ^C was pressed */
		کلید = BKEY_QUIT;
		بشکن;
	حالت CTL_CH('p'):
		کلید = BKEY_UP;
		بشکن;
	حالت CTL_CH('n'):
		کلید = BKEY_DOWN;
		بشکن;
	حالت CTL_CH('s'):
		کلید = BKEY_SAVE;
		بشکن;
	حالت '+':
		کلید = BKEY_PLUS;
		بشکن;
	حالت '-':
		کلید = BKEY_MINUS;
		بشکن;
	حالت ' ':
		کلید = BKEY_SPACE;
		بشکن;
	پیشفرض:
		کلید = BKEY_NONE;
		بشکن;
	}

	بازگشت کلید;
}

شمارش bootmenu_کلید bootmenu_loop(ساختار bootmenu_data *menu,
				ساختار cli_ch_state *cch)
{
	شمارش bootmenu_کلید کلید;
	صحیح c, errchar = 0;

	c = cli_ch_process(cch, 0);
	اگر (!c) {
		وقتی (!c && !tstc()) {
			schedule();
			mdelay(10);
			c = cli_ch_process(cch, errchar);
			errchar = -ETIMEDOUT;
		}
		اگر (!c) {
			c = getchar();
			c = cli_ch_process(cch, c);
		}
	}

	کلید = bootmenu_conv_کلید(c);

	اگر (کلید == BKEY_NONE && isalnum(c)) {
		کلید = BKEY_SHORTCUT;
		cch->shortcut_کلید = bootmenu_conv_shortcut_کلید(menu, c);
	}

	بازگشت کلید;
}
