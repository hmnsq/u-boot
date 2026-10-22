// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2010-2011 Calxeda, Inc.
 * Copyright (c) 2019, NVIDIA CORPORATION. All rights reserved.
 */

#ضمن <ansi.h>
#ضمن <cli.h>
#ضمن <malloc.h>
#ضمن <errno.h>
#ضمن <linux/ctype.h>
#ضمن <linux/delay.h>
#ضمن <linux/list.h>
#ضمن <watchdog.h>

#ضمن "menu.h"

#تعریف_کرو ansi 1

/*
 * Internally, each item in a menu is represented by a struct menu_item.
 *
 * These items will be alloc'd and initialized by menu_item_add and destroyed
 * by menu_item_destroy, and the consumer of the interface never sees that
 * this struct is used at all.
 */
بنية menu_شے {
	حرف *کلید;
	فراغ *انکڑے;
	بنية list_سر list;
};

/*
 * The menu is composed of a list of items along with settings and callbacks
 * provided by the user. An incomplete definition of this struct is available
 * in menu.h, but the full definition is here to prevent consumers from
 * relying on its contents.
 */
بنية menu {
	بنية menu_شے *default_شے;
	صحيح timeout;
	حرف *title;
	صحيح prompt;
	فراغ (*display_statusline)(بنية menu *);
	فراغ (*شے_انکڑے_print)(فراغ *);
	حرف *(*شے_choice)(فراغ *);
	bool (*need_reprint)(فراغ *);
	فراغ *شے_choice_انکڑے;
	بنية list_سر اشیاء;
	صحيح شے_cnt;
};

/*
 * An iterator function for menu items. callback will be called for each item
 * in m, with m, a pointer to the item, and extra being passed to callback. If
 * callback returns a value other than NULL, iteration stops and the value
 * return by callback is returned from menu_items_iter.  This allows it to be
 * used for search type operations. It is also safe for callback to remove the
 * item from the list of items.
 */
ساكن مضمن فراغ *menu_اشیاء_iter(بنية menu *m,
		فراغ *(*callback)(بنية menu *, بنية menu_شے *, فراغ *),
		فراغ *extra)
{
	بنية list_سر *pos, *n;
	بنية menu_شے *شے;
	فراغ *ret;

	list_for_each_محفوظ(pos, n, &m->اشیاء) {
		شے = list_اندراج(pos, بنية menu_شے, list);

		ret = callback(m, شے, extra);

		إذا (ret)
			أرجع ret;
	}

	أرجع عدم;
}

/*
 * Print a menu_item. If the consumer provided an item_data_print function
 * when creating the menu, call it with a pointer to the item's private data.
 * Otherwise, print the key of the item.
 */
ساكن مضمن فراغ *menu_شے_print(بنية menu *m,
				بنية menu_شے *شے,
				فراغ *extra)
{
	إذا (!m->شے_انکڑے_print) {
		puts(شے->کلید);
		putc('\n');
	} وإلا {
		m->شے_انکڑے_print(شے->انکڑے);
	}

	أرجع عدم;
}

/*
 * Free the memory used by a menu item. This includes the memory used by its
 * key.
 */
ساكن مضمن فراغ *menu_شے_destroy(بنية menu *m,
				بنية menu_شے *شے,
				فراغ *extra)
{
	إذا (شے->کلید)
		حرر(شے->کلید);

	حرر(شے);

	أرجع عدم;
}

/*
 * Display a menu so the user can make a choice of an item. First display its
 * title, if any, and then each item in the menu.
 */
ساكن مضمن فراغ menu_display(بنية menu *m)
{
	إذا (m->need_reprint) {
		إذا (!m->need_reprint(m->شے_choice_انکڑے))
			أرجع;
	}

	إذا (m->title) {
		puts(m->title);
		putc('\n');
	}
	إذا (m->display_statusline)
		m->display_statusline(m);

	menu_اشیاء_iter(m, menu_شے_print, عدم);
}

/*
 * Check if an item's key matches a provided string, pointed to by extra. If
 * extra is NULL, an item with a NULL key will match. Otherwise, the item's
 * key has to match according to strcmp.
 *
 * This is called via menu_items_iter, so it returns a pointer to the item if
 * the key matches, and returns NULL otherwise.
 */
ساكن مضمن فراغ *menu_شے_کلید_میل(بنية menu *m,
			بنية menu_شے *شے, فراغ *extra)
{
	حرف *شے_کلید = extra;

	إذا (!شے_کلید || !شے->کلید) {
		إذا (شے_کلید == شے->کلید)
			أرجع شے;

		أرجع عدم;
	}

	إذا (strcmp(شے->کلید, شے_کلید) == 0)
		أرجع شے;

	أرجع عدم;
}

/*
 * Find the first item with a key matching item_key, if any exists.
 */
ساكن مضمن بنية menu_شے *menu_شے_by_کلید(بنية menu *m,
							حرف *شے_کلید)
{
	أرجع menu_اشیاء_iter(m, menu_شے_کلید_میل, شے_کلید);
}

/*
 * Set *choice to point to the default item's data, if any default item was
 * set, and returns 1. If no default item was set, returns -ENOENT.
 */
صحيح menu_default_choice(بنية menu *m, فراغ **choice)
{
	إذا (m->default_شے) {
		*choice = m->default_شے->انکڑے;
		أرجع 1;
	}

	أرجع -ENOENT;
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
ساكن مضمن صحيح menu_interactive_choice(بنية menu *m, فراغ **choice)
{
	حرف cbuf[CONFIG_SYS_CBSIZE];
	بنية menu_شے *choice_شے = عدم;
	صحيح readret;

	طالما (!choice_شے) {
		cbuf[0] = '\0';

		menu_display(m);

		إذا (!m->شے_choice) {
			readret = cli_readline_into_بفر("Enter choice: ",
							   cbuf, m->timeout);

			إذا (readret >= 0) {
				choice_شے = menu_شے_by_کلید(m, cbuf);
				إذا (!choice_شے)
					لکھو("%s not found\n", cbuf);
			} وإلا إذا (readret == -1)  {
				لکھو("<INTERRUPT>\n");
				أرجع -EINTR;
			} وإلا {
				أرجع menu_default_choice(m, choice);
			}
		} وإلا {
			حرف *کلید = m->شے_choice(m->شے_choice_انکڑے);

			إذا (کلید)
				choice_شے = menu_شے_by_کلید(m, کلید);
		}

		إذا (!choice_شے)
			m->timeout = 0;
	}

	*choice = choice_شے->انکڑے;

	أرجع 1;
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
صحيح menu_default_سیٹ(بنية menu *m, حرف *شے_کلید)
{
	بنية menu_شے *شے;

	إذا (!m)
		أرجع -EINVAL;

	شے = menu_شے_by_کلید(m, شے_کلید);

	إذا (!شے)
		أرجع -ENOENT;

	m->default_شے = شے;

	أرجع 1;
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
صحيح menu_get_choice(بنية menu *m, فراغ **choice)
{
	إذا (!m || !choice)
		أرجع -EINVAL;

	إذا (!m->شے_cnt)
		أرجع -ENOENT;

	إذا (!m->prompt)
		أرجع menu_default_choice(m, choice);

	أرجع menu_interactive_choice(m, choice);
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
صحيح menu_شے_جوڑو(بنية menu *m, حرف *شے_کلید, فراغ *شے_انکڑے)
{
	بنية menu_شے *شے;

	إذا (!m)
		أرجع -EINVAL;

	شے = menu_شے_by_کلید(m, شے_کلید);

	إذا (شے) {
		شے->انکڑے = شے_انکڑے;
		أرجع 1;
	}

	شے = حافظہ_دیو(حجم *شے);
	إذا (!شے)
		أرجع -ENOMEM;

	شے->کلید = strdup(شے_کلید);

	إذا (!شے->کلید) {
		حرر(شے);
		أرجع -ENOMEM;
	}

	شے->انکڑے = شے_انکڑے;

	list_جوڑو_پوچھ(&شے->list, &m->اشیاء);
	m->شے_cnt++;

	أرجع 1;
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
بنية menu *menu_create(حرف *title, صحيح timeout, صحيح prompt,
				فراغ (*display_statusline)(بنية menu *),
				فراغ (*شے_انکڑے_print)(فراغ *),
				حرف *(*شے_choice)(فراغ *),
				bool (*need_reprint)(فراغ *),
				فراغ *شے_choice_انکڑے)
{
	بنية menu *m;

	m = حافظہ_دیو(حجم *m);

	إذا (!m)
		أرجع عدم;

	m->default_شے = عدم;
	m->prompt = prompt;
	m->timeout = timeout;
	m->display_statusline = display_statusline;
	m->شے_انکڑے_print = شے_انکڑے_print;
	m->شے_choice = شے_choice;
	m->need_reprint = need_reprint;
	m->شے_choice_انکڑے = شے_choice_انکڑے;
	m->شے_cnt = 0;

	إذا (title) {
		m->title = strdup(title);
		إذا (!m->title) {
			حرر(m);
			أرجع عدم;
		}
	} وإلا
		m->title = عدم;

	INIT_LIST_سر(&m->اشیاء);

	أرجع m;
}

/*
 * menu_destroy() - frees the memory used by a menu and its items.
 *
 * m - Points to a menu created by menu_create().
 *
 * Returns 1 if successful, or -EINVAL if m is NULL.
 */
صحيح menu_destroy(بنية menu *m)
{
	إذا (!m)
		أرجع -EINVAL;

	menu_اشیاء_iter(m, menu_شے_destroy, عدم);

	إذا (m->title)
		حرر(m->title);

	حرر(m);

	أرجع 1;
}

ساكن صحيح bootmenu_conv_shortcut_کلید(بنية bootmenu_انکڑے *menu, صحيح ichar)
{
	صحيح shortcut_کلید;

	ichar = tolower(ichar);
	بدل (ichar) {
	/* a-z for bootmenu entry > 9 */
	حالة 'a' ... 'z':
		shortcut_کلید = ichar - 'a' + 9;
		اكسر;
	/* 1-9 for bootmenu entry <= 9 */
	حالة '1' ... '9':
		shortcut_کلید = ichar - '1';
		اكسر;
	/* Reserve 0 for last option (aka Exit) */
	حالة '0':
	افتراضي:
		أرجع -1;
	}

	أرجع shortcut_کلید;
}

تعداد bootmenu_کلید bootmenu_autoboot_loop(بنية bootmenu_انکڑے *menu,
					 بنية cli_ch_state *cch)
{
	تعداد bootmenu_کلید کلید = BKEY_NONE;
	صحيح i, c;

	طالما (menu->delay > 0) {
		صحيح ichar;

		إذا (ansi)
			لکھو(ANSI_CURSOR_POSITION, menu->گنتی + 5, 3);
		لکھو("Hit any key to stop autoboot: %d ", menu->delay);
		لكل (i = 0; i < 100; ++i) {
			إذا (!tstc()) {
				ویلا_سارنی();
				mdelay(10);
				استمر;
			}

			menu->delay = -1;
			c = getchar();

			ichar = cli_ch_process(cch, c);

			بدل (ichar) {
			حالة '\0':
				کلید = BKEY_NONE;
				اكسر;
			حالة '\n':
				کلید = BKEY_SELECT;
				اكسر;
			حالة 0x3: /* ^C */
				کلید = BKEY_QUIT;
				اكسر;
			حالة 'A' ... 'Z':
			حالة 'a' ... 'z':
			حالة '0' ... '9':
				کلید = BKEY_SHORTCUT;
				اكسر;
			افتراضي:
				کلید = BKEY_NONE;
				اكسر;
			}
			اكسر;
		}

		إذا (کلید == BKEY_SHORTCUT)
			cch->shortcut_کلید = bootmenu_conv_shortcut_کلید(menu, ichar);

		إذا (menu->delay < 0)
			اكسر;

		--menu->delay;
	}

	إذا (ansi)
		لکھو(ANSI_CURSOR_POSITION ANSI_CLEAR_سطر, menu->گنتی + 5, 1);

	إذا (menu->delay == 0)
		کلید = BKEY_SELECT;

	أرجع کلید;
}

تعداد bootmenu_کلید bootmenu_conv_کلید(صحيح ichar)
{
	تعداد bootmenu_کلید کلید;

	بدل (ichar) {
	حالة '\n':
		/* enter key was pressed */
		کلید = BKEY_SELECT;
		اكسر;
	حالة CTL_CH('c'):
	حالة '\e':
		/* ^C was pressed */
		کلید = BKEY_QUIT;
		اكسر;
	حالة CTL_CH('p'):
		کلید = BKEY_اتے;
		اكسر;
	حالة CTL_CH('n'):
		کلید = BKEY_تھلے;
		اكسر;
	حالة CTL_CH('s'):
		کلید = BKEY_SAVE;
		اكسر;
	حالة '+':
		کلید = BKEY_PLUS;
		اكسر;
	حالة '-':
		کلید = BKEY_MINUS;
		اكسر;
	حالة ' ':
		کلید = BKEY_SPACE;
		اكسر;
	افتراضي:
		کلید = BKEY_NONE;
		اكسر;
	}

	أرجع کلید;
}

تعداد bootmenu_کلید bootmenu_loop(بنية bootmenu_انکڑے *menu,
				بنية cli_ch_state *cch)
{
	تعداد bootmenu_کلید کلید;
	صحيح c, errchar = 0;

	c = cli_ch_process(cch, 0);
	إذا (!c) {
		طالما (!c && !tstc()) {
			ویلا_سارنی();
			mdelay(10);
			c = cli_ch_process(cch, errchar);
			errchar = -ETIMEDOUT;
		}
		إذا (!c) {
			c = getchar();
			c = cli_ch_process(cch, c);
		}
	}

	کلید = bootmenu_conv_کلید(c);

	إذا (کلید == BKEY_NONE && حرف_یا_ہندسہ(c)) {
		کلید = BKEY_SHORTCUT;
		cch->shortcut_کلید = bootmenu_conv_shortcut_کلید(menu, c);
	}

	أرجع کلید;
}
