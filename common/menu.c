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

#تعریف_کرو ansi 1

/*
 * Internally, each item in a menu is represented by a struct menu_item.
 *
 * These items will be alloc'd and initialized by menu_item_add and destroyed
 * by menu_item_destroy, and the consumer of the interface never sees that
 * this struct is used at all.
 */
ساخت menu_شے {
	حرف *کلید;
	خالی *ڈیٹا;
	ساخت list_سر list;
};

/*
 * The menu is composed of a list of items along with settings and callbacks
 * provided by the user. An incomplete definition of this struct is available
 * in menu.h, but the full definition is here to prevent consumers from
 * relying on its contents.
 */
ساخت menu {
	ساخت menu_شے *default_شے;
	صحیح timeout;
	حرف *title;
	صحیح prompt;
	خالی (*display_statusline)(ساخت menu *);
	خالی (*شے_ڈیٹا_print)(خالی *);
	حرف *(*شے_choice)(خالی *);
	bool (*need_reprint)(خالی *);
	خالی *شے_choice_ڈیٹا;
	ساخت list_سر اشیاء;
	صحیح شے_cnt;
};

/*
 * An iterator function for menu items. callback will be called for each item
 * in m, with m, a pointer to the item, and extra being passed to callback. If
 * callback returns a value other than NULL, iteration stops and the value
 * return by callback is returned from menu_items_iter.  This allows it to be
 * used for search type operations. It is also safe for callback to remove the
 * item from the list of items.
 */
ساکن اندرونی خالی *menu_اشیاء_iter(ساخت menu *m,
		خالی *(*callback)(ساخت menu *, ساخت menu_شے *, خالی *),
		خالی *extra)
{
	ساخت list_سر *pos, *n;
	ساخت menu_شے *شے;
	خالی *ret;

	list_for_each_محفوظ(pos, n, &m->اشیاء) {
		شے = list_اندراج(pos, ساخت menu_شے, list);

		ret = callback(m, شے, extra);

		اگر (ret)
			واپس ret;
	}

	واپس تہی;
}

/*
 * Print a menu_item. If the consumer provided an item_data_print function
 * when creating the menu, call it with a pointer to the item's private data.
 * Otherwise, print the key of the item.
 */
ساکن اندرونی خالی *menu_شے_print(ساخت menu *m,
				ساخت menu_شے *شے,
				خالی *extra)
{
	اگر (!m->شے_ڈیٹا_print) {
		puts(شے->کلید);
		putc('\n');
	} ورنہ {
		m->شے_ڈیٹا_print(شے->ڈیٹا);
	}

	واپس تہی;
}

/*
 * Free the memory used by a menu item. This includes the memory used by its
 * key.
 */
ساکن اندرونی خالی *menu_شے_destroy(ساخت menu *m,
				ساخت menu_شے *شے,
				خالی *extra)
{
	اگر (شے->کلید)
		آزاد(شے->کلید);

	آزاد(شے);

	واپس تہی;
}

/*
 * Display a menu so the user can make a choice of an item. First display its
 * title, if any, and then each item in the menu.
 */
ساکن اندرونی خالی menu_display(ساخت menu *m)
{
	اگر (m->need_reprint) {
		اگر (!m->need_reprint(m->شے_choice_ڈیٹا))
			واپس;
	}

	اگر (m->title) {
		puts(m->title);
		putc('\n');
	}
	اگر (m->display_statusline)
		m->display_statusline(m);

	menu_اشیاء_iter(m, menu_شے_print, تہی);
}

/*
 * Check if an item's key matches a provided string, pointed to by extra. If
 * extra is NULL, an item with a NULL key will match. Otherwise, the item's
 * key has to match according to strcmp.
 *
 * This is called via menu_items_iter, so it returns a pointer to the item if
 * the key matches, and returns NULL otherwise.
 */
ساکن اندرونی خالی *menu_شے_کلید_مطابقت(ساخت menu *m,
			ساخت menu_شے *شے, خالی *extra)
{
	حرف *شے_کلید = extra;

	اگر (!شے_کلید || !شے->کلید) {
		اگر (شے_کلید == شے->کلید)
			واپس شے;

		واپس تہی;
	}

	اگر (strcmp(شے->کلید, شے_کلید) == 0)
		واپس شے;

	واپس تہی;
}

/*
 * Find the first item with a key matching item_key, if any exists.
 */
ساکن اندرونی ساخت menu_شے *menu_شے_by_کلید(ساخت menu *m,
							حرف *شے_کلید)
{
	واپس menu_اشیاء_iter(m, menu_شے_کلید_مطابقت, شے_کلید);
}

/*
 * Set *choice to point to the default item's data, if any default item was
 * set, and returns 1. If no default item was set, returns -ENOENT.
 */
صحیح menu_default_choice(ساخت menu *m, خالی **choice)
{
	اگر (m->default_شے) {
		*choice = m->default_شے->ڈیٹا;
		واپس 1;
	}

	واپس -ENOENT;
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
ساکن اندرونی صحیح menu_interactive_choice(ساخت menu *m, خالی **choice)
{
	حرف cbuf[CONFIG_SYS_CBSIZE];
	ساخت menu_شے *choice_شے = تہی;
	صحیح readret;

	جب (!choice_شے) {
		cbuf[0] = '\0';

		menu_display(m);

		اگر (!m->شے_choice) {
			readret = cli_readline_into_بفر("Enter choice: ",
							   cbuf, m->timeout);

			اگر (readret >= 0) {
				choice_شے = menu_شے_by_کلید(m, cbuf);
				اگر (!choice_شے)
					لکھو("%s not found\n", cbuf);
			} ورنہ اگر (readret == -1)  {
				لکھو("<INTERRUPT>\n");
				واپس -EINTR;
			} ورنہ {
				واپس menu_default_choice(m, choice);
			}
		} ورنہ {
			حرف *کلید = m->شے_choice(m->شے_choice_ڈیٹا);

			اگر (کلید)
				choice_شے = menu_شے_by_کلید(m, کلید);
		}

		اگر (!choice_شے)
			m->timeout = 0;
	}

	*choice = choice_شے->ڈیٹا;

	واپس 1;
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
صحیح menu_default_سیٹ(ساخت menu *m, حرف *شے_کلید)
{
	ساخت menu_شے *شے;

	اگر (!m)
		واپس -EINVAL;

	شے = menu_شے_by_کلید(m, شے_کلید);

	اگر (!شے)
		واپس -ENOENT;

	m->default_شے = شے;

	واپس 1;
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
صحیح menu_get_choice(ساخت menu *m, خالی **choice)
{
	اگر (!m || !choice)
		واپس -EINVAL;

	اگر (!m->شے_cnt)
		واپس -ENOENT;

	اگر (!m->prompt)
		واپس menu_default_choice(m, choice);

	واپس menu_interactive_choice(m, choice);
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
صحیح menu_شے_جوڑو(ساخت menu *m, حرف *شے_کلید, خالی *شے_ڈیٹا)
{
	ساخت menu_شے *شے;

	اگر (!m)
		واپس -EINVAL;

	شے = menu_شے_by_کلید(m, شے_کلید);

	اگر (شے) {
		شے->ڈیٹا = شے_ڈیٹا;
		واپس 1;
	}

	شے = حافظہ_دو(حجم *شے);
	اگر (!شے)
		واپس -ENOMEM;

	شے->کلید = strdup(شے_کلید);

	اگر (!شے->کلید) {
		آزاد(شے);
		واپس -ENOMEM;
	}

	شے->ڈیٹا = شے_ڈیٹا;

	list_جوڑو_دم(&شے->list, &m->اشیاء);
	m->شے_cnt++;

	واپس 1;
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
ساخت menu *menu_create(حرف *title, صحیح timeout, صحیح prompt,
				خالی (*display_statusline)(ساخت menu *),
				خالی (*شے_ڈیٹا_print)(خالی *),
				حرف *(*شے_choice)(خالی *),
				bool (*need_reprint)(خالی *),
				خالی *شے_choice_ڈیٹا)
{
	ساخت menu *m;

	m = حافظہ_دو(حجم *m);

	اگر (!m)
		واپس تہی;

	m->default_شے = تہی;
	m->prompt = prompt;
	m->timeout = timeout;
	m->display_statusline = display_statusline;
	m->شے_ڈیٹا_print = شے_ڈیٹا_print;
	m->شے_choice = شے_choice;
	m->need_reprint = need_reprint;
	m->شے_choice_ڈیٹا = شے_choice_ڈیٹا;
	m->شے_cnt = 0;

	اگر (title) {
		m->title = strdup(title);
		اگر (!m->title) {
			آزاد(m);
			واپس تہی;
		}
	} ورنہ
		m->title = تہی;

	INIT_LIST_سر(&m->اشیاء);

	واپس m;
}

/*
 * menu_destroy() - frees the memory used by a menu and its items.
 *
 * m - Points to a menu created by menu_create().
 *
 * Returns 1 if successful, or -EINVAL if m is NULL.
 */
صحیح menu_destroy(ساخت menu *m)
{
	اگر (!m)
		واپس -EINVAL;

	menu_اشیاء_iter(m, menu_شے_destroy, تہی);

	اگر (m->title)
		آزاد(m->title);

	آزاد(m);

	واپس 1;
}

ساکن صحیح bootmenu_conv_shortcut_کلید(ساخت bootmenu_ڈیٹا *menu, صحیح ichar)
{
	صحیح shortcut_کلید;

	ichar = tolower(ichar);
	انتخاب (ichar) {
	/* a-z for bootmenu entry > 9 */
	حالت 'a' ... 'z':
		shortcut_کلید = ichar - 'a' + 9;
		توڑو;
	/* 1-9 for bootmenu entry <= 9 */
	حالت '1' ... '9':
		shortcut_کلید = ichar - '1';
		توڑو;
	/* Reserve 0 for last option (aka Exit) */
	حالت '0':
	طےشدہ:
		واپس -1;
	}

	واپس shortcut_کلید;
}

شمار bootmenu_کلید bootmenu_autoboot_loop(ساخت bootmenu_ڈیٹا *menu,
					 ساخت cli_ch_state *cch)
{
	شمار bootmenu_کلید کلید = BKEY_NONE;
	صحیح i, c;

	جب (menu->delay > 0) {
		صحیح ichar;

		اگر (ansi)
			لکھو(ANSI_CURSOR_POSITION, menu->گنتی + 5, 3);
		لکھو("Hit any key to stop autoboot: %d ", menu->delay);
		برائے (i = 0; i < 100; ++i) {
			اگر (!tstc()) {
				جدول_وقت();
				mdelay(10);
				جاری;
			}

			menu->delay = -1;
			c = getchar();

			ichar = cli_ch_process(cch, c);

			انتخاب (ichar) {
			حالت '\0':
				کلید = BKEY_NONE;
				توڑو;
			حالت '\n':
				کلید = BKEY_SELECT;
				توڑو;
			حالت 0x3: /* ^C */
				کلید = BKEY_QUIT;
				توڑو;
			حالت 'A' ... 'Z':
			حالت 'a' ... 'z':
			حالت '0' ... '9':
				کلید = BKEY_SHORTCUT;
				توڑو;
			طےشدہ:
				کلید = BKEY_NONE;
				توڑو;
			}
			توڑو;
		}

		اگر (کلید == BKEY_SHORTCUT)
			cch->shortcut_کلید = bootmenu_conv_shortcut_کلید(menu, ichar);

		اگر (menu->delay < 0)
			توڑو;

		--menu->delay;
	}

	اگر (ansi)
		لکھو(ANSI_CURSOR_POSITION ANSI_CLEAR_سطر, menu->گنتی + 5, 1);

	اگر (menu->delay == 0)
		کلید = BKEY_SELECT;

	واپس کلید;
}

شمار bootmenu_کلید bootmenu_conv_کلید(صحیح ichar)
{
	شمار bootmenu_کلید کلید;

	انتخاب (ichar) {
	حالت '\n':
		/* enter key was pressed */
		کلید = BKEY_SELECT;
		توڑو;
	حالت CTL_CH('c'):
	حالت '\e':
		/* ^C was pressed */
		کلید = BKEY_QUIT;
		توڑو;
	حالت CTL_CH('p'):
		کلید = BKEY_اوپر;
		توڑو;
	حالت CTL_CH('n'):
		کلید = BKEY_نیچے_کو;
		توڑو;
	حالت CTL_CH('s'):
		کلید = BKEY_SAVE;
		توڑو;
	حالت '+':
		کلید = BKEY_PLUS;
		توڑو;
	حالت '-':
		کلید = BKEY_MINUS;
		توڑو;
	حالت ' ':
		کلید = BKEY_SPACE;
		توڑو;
	طےشدہ:
		کلید = BKEY_NONE;
		توڑو;
	}

	واپس کلید;
}

شمار bootmenu_کلید bootmenu_loop(ساخت bootmenu_ڈیٹا *menu,
				ساخت cli_ch_state *cch)
{
	شمار bootmenu_کلید کلید;
	صحیح c, errchar = 0;

	c = cli_ch_process(cch, 0);
	اگر (!c) {
		جب (!c && !tstc()) {
			جدول_وقت();
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

	اگر (کلید == BKEY_NONE && حرف_یا_ہندسہ(c)) {
		کلید = BKEY_SHORTCUT;
		cch->shortcut_کلید = bootmenu_conv_shortcut_کلید(menu, c);
	}

	واپس کلید;
}
