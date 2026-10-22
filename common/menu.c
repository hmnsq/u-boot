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

#تعریف_کړه ansi 1

/*
 * Internally, each item in a menu is represented by a struct menu_item.
 *
 * These items will be alloc'd and initialized by menu_item_add and destroyed
 * by menu_item_destroy, and the consumer of the interface never sees that
 * this struct is used at all.
 */
بنية menu_توکی {
	حرف *کلی;
	فراغ *معلومات;
	بنية list_سر list;
};

/*
 * The menu is composed of a list of items along with settings and callbacks
 * provided by the user. An incomplete definition of this struct is available
 * in menu.h, but the full definition is here to prevent consumers from
 * relying on its contents.
 */
بنية menu {
	بنية menu_توکی *default_توکی;
	صحيح timeout;
	حرف *title;
	صحيح prompt;
	فراغ (*display_statusline)(بنية menu *);
	فراغ (*توکی_معلومات_print)(فراغ *);
	حرف *(*توکی_choice)(فراغ *);
	bool (*need_reprint)(فراغ *);
	فراغ *توکی_choice_معلومات;
	بنية list_سر توکي;
	صحيح توکی_cnt;
};

/*
 * An iterator function for menu items. callback will be called for each item
 * in m, with m, a pointer to the item, and extra being passed to callback. If
 * callback returns a value other than NULL, iteration stops and the value
 * return by callback is returned from menu_items_iter.  This allows it to be
 * used for search type operations. It is also safe for callback to remove the
 * item from the list of items.
 */
ساكن مضمن فراغ *menu_توکي_iter(بنية menu *m,
		فراغ *(*callback)(بنية menu *, بنية menu_توکی *, فراغ *),
		فراغ *extra)
{
	بنية list_سر *pos, *n;
	بنية menu_توکی *توکی;
	فراغ *ret;

	list_for_each_خوندي(pos, n, &m->توکي) {
		توکی = list_ثبت(pos, بنية menu_توکی, list);

		ret = callback(m, توکی, extra);

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
ساكن مضمن فراغ *menu_توکی_print(بنية menu *m,
				بنية menu_توکی *توکی,
				فراغ *extra)
{
	إذا (!m->توکی_معلومات_print) {
		puts(توکی->کلی);
		putc('\n');
	} وإلا {
		m->توکی_معلومات_print(توکی->معلومات);
	}

	أرجع عدم;
}

/*
 * Free the memory used by a menu item. This includes the memory used by its
 * key.
 */
ساكن مضمن فراغ *menu_توکی_destroy(بنية menu *m,
				بنية menu_توکی *توکی,
				فراغ *extra)
{
	إذا (توکی->کلی)
		حرر(توکی->کلی);

	حرر(توکی);

	أرجع عدم;
}

/*
 * Display a menu so the user can make a choice of an item. First display its
 * title, if any, and then each item in the menu.
 */
ساكن مضمن فراغ menu_display(بنية menu *m)
{
	إذا (m->need_reprint) {
		إذا (!m->need_reprint(m->توکی_choice_معلومات))
			أرجع;
	}

	إذا (m->title) {
		puts(m->title);
		putc('\n');
	}
	إذا (m->display_statusline)
		m->display_statusline(m);

	menu_توکي_iter(m, menu_توکی_print, عدم);
}

/*
 * Check if an item's key matches a provided string, pointed to by extra. If
 * extra is NULL, an item with a NULL key will match. Otherwise, the item's
 * key has to match according to strcmp.
 *
 * This is called via menu_items_iter, so it returns a pointer to the item if
 * the key matches, and returns NULL otherwise.
 */
ساكن مضمن فراغ *menu_توکی_کلی_سمون(بنية menu *m,
			بنية menu_توکی *توکی, فراغ *extra)
{
	حرف *توکی_کلی = extra;

	إذا (!توکی_کلی || !توکی->کلی) {
		إذا (توکی_کلی == توکی->کلی)
			أرجع توکی;

		أرجع عدم;
	}

	إذا (strcmp(توکی->کلی, توکی_کلی) == 0)
		أرجع توکی;

	أرجع عدم;
}

/*
 * Find the first item with a key matching item_key, if any exists.
 */
ساكن مضمن بنية menu_توکی *menu_توکی_by_کلی(بنية menu *m,
							حرف *توکی_کلی)
{
	أرجع menu_توکي_iter(m, menu_توکی_کلی_سمون, توکی_کلی);
}

/*
 * Set *choice to point to the default item's data, if any default item was
 * set, and returns 1. If no default item was set, returns -ENOENT.
 */
صحيح menu_default_choice(بنية menu *m, فراغ **choice)
{
	إذا (m->default_توکی) {
		*choice = m->default_توکی->معلومات;
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
	بنية menu_توکی *choice_توکی = عدم;
	صحيح readret;

	طالما (!choice_توکی) {
		cbuf[0] = '\0';

		menu_display(m);

		إذا (!m->توکی_choice) {
			readret = cli_readline_into_بفر("Enter choice: ",
							   cbuf, m->timeout);

			إذا (readret >= 0) {
				choice_توکی = menu_توکی_by_کلی(m, cbuf);
				إذا (!choice_توکی)
					چاپ_کړه("%s not found\n", cbuf);
			} وإلا إذا (readret == -1)  {
				چاپ_کړه("<INTERRUPT>\n");
				أرجع -EINTR;
			} وإلا {
				أرجع menu_default_choice(m, choice);
			}
		} وإلا {
			حرف *کلی = m->توکی_choice(m->توکی_choice_معلومات);

			إذا (کلی)
				choice_توکی = menu_توکی_by_کلی(m, کلی);
		}

		إذا (!choice_توکی)
			m->timeout = 0;
	}

	*choice = choice_توکی->معلومات;

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
صحيح menu_default_سېټ(بنية menu *m, حرف *توکی_کلی)
{
	بنية menu_توکی *توکی;

	إذا (!m)
		أرجع -EINVAL;

	توکی = menu_توکی_by_کلی(m, توکی_کلی);

	إذا (!توکی)
		أرجع -ENOENT;

	m->default_توکی = توکی;

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

	إذا (!m->توکی_cnt)
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
صحيح menu_توکی_جمع_کړه(بنية menu *m, حرف *توکی_کلی, فراغ *توکی_معلومات)
{
	بنية menu_توکی *توکی;

	إذا (!m)
		أرجع -EINVAL;

	توکی = menu_توکی_by_کلی(m, توکی_کلی);

	إذا (توکی) {
		توکی->معلومات = توکی_معلومات;
		أرجع 1;
	}

	توکی = حافظه_ورکړه(حجم *توکی);
	إذا (!توکی)
		أرجع -ENOMEM;

	توکی->کلی = strdup(توکی_کلی);

	إذا (!توکی->کلی) {
		حرر(توکی);
		أرجع -ENOMEM;
	}

	توکی->معلومات = توکی_معلومات;

	list_جمع_کړه_لکۍ(&توکی->list, &m->توکي);
	m->توکی_cnt++;

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
				فراغ (*توکی_معلومات_print)(فراغ *),
				حرف *(*توکی_choice)(فراغ *),
				bool (*need_reprint)(فراغ *),
				فراغ *توکی_choice_معلومات)
{
	بنية menu *m;

	m = حافظه_ورکړه(حجم *m);

	إذا (!m)
		أرجع عدم;

	m->default_توکی = عدم;
	m->prompt = prompt;
	m->timeout = timeout;
	m->display_statusline = display_statusline;
	m->توکی_معلومات_print = توکی_معلومات_print;
	m->توکی_choice = توکی_choice;
	m->need_reprint = need_reprint;
	m->توکی_choice_معلومات = توکی_choice_معلومات;
	m->توکی_cnt = 0;

	إذا (title) {
		m->title = strdup(title);
		إذا (!m->title) {
			حرر(m);
			أرجع عدم;
		}
	} وإلا
		m->title = عدم;

	INIT_LIST_سر(&m->توکي);

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

	menu_توکي_iter(m, menu_توکی_destroy, عدم);

	إذا (m->title)
		حرر(m->title);

	حرر(m);

	أرجع 1;
}

ساكن صحيح bootmenu_conv_shortcut_کلی(بنية bootmenu_معلومات *menu, صحيح ichar)
{
	صحيح shortcut_کلی;

	ichar = tolower(ichar);
	بدل (ichar) {
	/* a-z for bootmenu entry > 9 */
	حالة 'a' ... 'z':
		shortcut_کلی = ichar - 'a' + 9;
		اكسر;
	/* 1-9 for bootmenu entry <= 9 */
	حالة '1' ... '9':
		shortcut_کلی = ichar - '1';
		اكسر;
	/* Reserve 0 for last option (aka Exit) */
	حالة '0':
	افتراضي:
		أرجع -1;
	}

	أرجع shortcut_کلی;
}

تعداد bootmenu_کلی bootmenu_autoboot_loop(بنية bootmenu_معلومات *menu,
					 بنية cli_ch_state *cch)
{
	تعداد bootmenu_کلی کلی = BKEY_NONE;
	صحيح i, c;

	طالما (menu->delay > 0) {
		صحيح ichar;

		إذا (ansi)
			چاپ_کړه(ANSI_CURSOR_POSITION, menu->شمېر + 5, 3);
		چاپ_کړه("Hit any key to stop autoboot: %d ", menu->delay);
		لكل (i = 0; i < 100; ++i) {
			إذا (!tstc()) {
				مهالوېش();
				mdelay(10);
				استمر;
			}

			menu->delay = -1;
			c = getchar();

			ichar = cli_ch_process(cch, c);

			بدل (ichar) {
			حالة '\0':
				کلی = BKEY_NONE;
				اكسر;
			حالة '\n':
				کلی = BKEY_SELECT;
				اكسر;
			حالة 0x3: /* ^C */
				کلی = BKEY_QUIT;
				اكسر;
			حالة 'A' ... 'Z':
			حالة 'a' ... 'z':
			حالة '0' ... '9':
				کلی = BKEY_SHORTCUT;
				اكسر;
			افتراضي:
				کلی = BKEY_NONE;
				اكسر;
			}
			اكسر;
		}

		إذا (کلی == BKEY_SHORTCUT)
			cch->shortcut_کلی = bootmenu_conv_shortcut_کلی(menu, ichar);

		إذا (menu->delay < 0)
			اكسر;

		--menu->delay;
	}

	إذا (ansi)
		چاپ_کړه(ANSI_CURSOR_POSITION ANSI_CLEAR_کرښه, menu->شمېر + 5, 1);

	إذا (menu->delay == 0)
		کلی = BKEY_SELECT;

	أرجع کلی;
}

تعداد bootmenu_کلی bootmenu_conv_کلی(صحيح ichar)
{
	تعداد bootmenu_کلی کلی;

	بدل (ichar) {
	حالة '\n':
		/* enter key was pressed */
		کلی = BKEY_SELECT;
		اكسر;
	حالة CTL_CH('c'):
	حالة '\e':
		/* ^C was pressed */
		کلی = BKEY_QUIT;
		اكسر;
	حالة CTL_CH('p'):
		کلی = BKEY_پورته;
		اكسر;
	حالة CTL_CH('n'):
		کلی = BKEY_ښکته;
		اكسر;
	حالة CTL_CH('s'):
		کلی = BKEY_SAVE;
		اكسر;
	حالة '+':
		کلی = BKEY_PLUS;
		اكسر;
	حالة '-':
		کلی = BKEY_MINUS;
		اكسر;
	حالة ' ':
		کلی = BKEY_SPACE;
		اكسر;
	افتراضي:
		کلی = BKEY_NONE;
		اكسر;
	}

	أرجع کلی;
}

تعداد bootmenu_کلی bootmenu_loop(بنية bootmenu_معلومات *menu,
				بنية cli_ch_state *cch)
{
	تعداد bootmenu_کلی کلی;
	صحيح c, errchar = 0;

	c = cli_ch_process(cch, 0);
	إذا (!c) {
		طالما (!c && !tstc()) {
			مهالوېش();
			mdelay(10);
			c = cli_ch_process(cch, errchar);
			errchar = -ETIMEDOUT;
		}
		إذا (!c) {
			c = getchar();
			c = cli_ch_process(cch, c);
		}
	}

	کلی = bootmenu_conv_کلی(c);

	إذا (کلی == BKEY_NONE && که_توری_یا_عدد(c)) {
		کلی = BKEY_SHORTCUT;
		cch->shortcut_کلی = bootmenu_conv_shortcut_کلی(menu, c);
	}

	أرجع کلی;
}
