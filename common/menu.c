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

#وصف_ڪريو ansi 1

/*
 * Internally, each item in a menu is represented by a struct menu_item.
 *
 * These items will be alloc'd and initialized by menu_item_add and destroyed
 * by menu_item_destroy, and the consumer of the interface never sees that
 * this struct is used at all.
 */
بنية menu_شي {
	حرف *ڪنجي;
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
	بنية menu_شي *default_شي;
	صحيح timeout;
	حرف *title;
	صحيح prompt;
	فراغ (*display_statusline)(بنية menu *);
	فراغ (*شي_معلومات_print)(فراغ *);
	حرف *(*شي_choice)(فراغ *);
	bool (*need_reprint)(فراغ *);
	فراغ *شي_choice_معلومات;
	بنية list_سر شيون;
	صحيح شي_cnt;
};

/*
 * An iterator function for menu items. callback will be called for each item
 * in m, with m, a pointer to the item, and extra being passed to callback. If
 * callback returns a value other than NULL, iteration stops and the value
 * return by callback is returned from menu_items_iter.  This allows it to be
 * used for search type operations. It is also safe for callback to remove the
 * item from the list of items.
 */
ساكن مضمن فراغ *menu_شيون_iter(بنية menu *m,
		فراغ *(*callback)(بنية menu *, بنية menu_شي *, فراغ *),
		فراغ *extra)
{
	بنية list_سر *pos, *n;
	بنية menu_شي *شي;
	فراغ *ret;

	list_for_each_محفوظ(pos, n, &m->شيون) {
		شي = list_داخلا(pos, بنية menu_شي, list);

		ret = callback(m, شي, extra);

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
ساكن مضمن فراغ *menu_شي_print(بنية menu *m,
				بنية menu_شي *شي,
				فراغ *extra)
{
	إذا (!m->شي_معلومات_print) {
		puts(شي->ڪنجي);
		putc('\n');
	} وإلا {
		m->شي_معلومات_print(شي->معلومات);
	}

	أرجع عدم;
}

/*
 * Free the memory used by a menu item. This includes the memory used by its
 * key.
 */
ساكن مضمن فراغ *menu_شي_destroy(بنية menu *m,
				بنية menu_شي *شي,
				فراغ *extra)
{
	إذا (شي->ڪنجي)
		حرر(شي->ڪنجي);

	حرر(شي);

	أرجع عدم;
}

/*
 * Display a menu so the user can make a choice of an item. First display its
 * title, if any, and then each item in the menu.
 */
ساكن مضمن فراغ menu_display(بنية menu *m)
{
	إذا (m->need_reprint) {
		إذا (!m->need_reprint(m->شي_choice_معلومات))
			أرجع;
	}

	إذا (m->title) {
		puts(m->title);
		putc('\n');
	}
	إذا (m->display_statusline)
		m->display_statusline(m);

	menu_شيون_iter(m, menu_شي_print, عدم);
}

/*
 * Check if an item's key matches a provided string, pointed to by extra. If
 * extra is NULL, an item with a NULL key will match. Otherwise, the item's
 * key has to match according to strcmp.
 *
 * This is called via menu_items_iter, so it returns a pointer to the item if
 * the key matches, and returns NULL otherwise.
 */
ساكن مضمن فراغ *menu_شي_ڪنجي_ميلاپ(بنية menu *m,
			بنية menu_شي *شي, فراغ *extra)
{
	حرف *شي_ڪنجي = extra;

	إذا (!شي_ڪنجي || !شي->ڪنجي) {
		إذا (شي_ڪنجي == شي->ڪنجي)
			أرجع شي;

		أرجع عدم;
	}

	إذا (strcmp(شي->ڪنجي, شي_ڪنجي) == 0)
		أرجع شي;

	أرجع عدم;
}

/*
 * Find the first item with a key matching item_key, if any exists.
 */
ساكن مضمن بنية menu_شي *menu_شي_by_ڪنجي(بنية menu *m,
							حرف *شي_ڪنجي)
{
	أرجع menu_شيون_iter(m, menu_شي_ڪنجي_ميلاپ, شي_ڪنجي);
}

/*
 * Set *choice to point to the default item's data, if any default item was
 * set, and returns 1. If no default item was set, returns -ENOENT.
 */
صحيح menu_default_choice(بنية menu *m, فراغ **choice)
{
	إذا (m->default_شي) {
		*choice = m->default_شي->معلومات;
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
	بنية menu_شي *choice_شي = عدم;
	صحيح readret;

	طالما (!choice_شي) {
		cbuf[0] = '\0';

		menu_display(m);

		إذا (!m->شي_choice) {
			readret = cli_readline_into_بفر("Enter choice: ",
							   cbuf, m->timeout);

			إذا (readret >= 0) {
				choice_شي = menu_شي_by_ڪنجي(m, cbuf);
				إذا (!choice_شي)
					لکو("%s not found\n", cbuf);
			} وإلا إذا (readret == -1)  {
				لکو("<INTERRUPT>\n");
				أرجع -EINTR;
			} وإلا {
				أرجع menu_default_choice(m, choice);
			}
		} وإلا {
			حرف *ڪنجي = m->شي_choice(m->شي_choice_معلومات);

			إذا (ڪنجي)
				choice_شي = menu_شي_by_ڪنجي(m, ڪنجي);
		}

		إذا (!choice_شي)
			m->timeout = 0;
	}

	*choice = choice_شي->معلومات;

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
صحيح menu_default_سيٽ(بنية menu *m, حرف *شي_ڪنجي)
{
	بنية menu_شي *شي;

	إذا (!m)
		أرجع -EINVAL;

	شي = menu_شي_by_ڪنجي(m, شي_ڪنجي);

	إذا (!شي)
		أرجع -ENOENT;

	m->default_شي = شي;

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

	إذا (!m->شي_cnt)
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
صحيح menu_شي_جوڙيو(بنية menu *m, حرف *شي_ڪنجي, فراغ *شي_معلومات)
{
	بنية menu_شي *شي;

	إذا (!m)
		أرجع -EINVAL;

	شي = menu_شي_by_ڪنجي(m, شي_ڪنجي);

	إذا (شي) {
		شي->معلومات = شي_معلومات;
		أرجع 1;
	}

	شي = يادگيري_ڏيو(حجم *شي);
	إذا (!شي)
		أرجع -ENOMEM;

	شي->ڪنجي = strdup(شي_ڪنجي);

	إذا (!شي->ڪنجي) {
		حرر(شي);
		أرجع -ENOMEM;
	}

	شي->معلومات = شي_معلومات;

	list_جوڙيو_پڇ(&شي->list, &m->شيون);
	m->شي_cnt++;

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
				فراغ (*شي_معلومات_print)(فراغ *),
				حرف *(*شي_choice)(فراغ *),
				bool (*need_reprint)(فراغ *),
				فراغ *شي_choice_معلومات)
{
	بنية menu *m;

	m = يادگيري_ڏيو(حجم *m);

	إذا (!m)
		أرجع عدم;

	m->default_شي = عدم;
	m->prompt = prompt;
	m->timeout = timeout;
	m->display_statusline = display_statusline;
	m->شي_معلومات_print = شي_معلومات_print;
	m->شي_choice = شي_choice;
	m->need_reprint = need_reprint;
	m->شي_choice_معلومات = شي_choice_معلومات;
	m->شي_cnt = 0;

	إذا (title) {
		m->title = strdup(title);
		إذا (!m->title) {
			حرر(m);
			أرجع عدم;
		}
	} وإلا
		m->title = عدم;

	INIT_LIST_سر(&m->شيون);

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

	menu_شيون_iter(m, menu_شي_destroy, عدم);

	إذا (m->title)
		حرر(m->title);

	حرر(m);

	أرجع 1;
}

ساكن صحيح bootmenu_conv_shortcut_ڪنجي(بنية bootmenu_معلومات *menu, صحيح ichar)
{
	صحيح shortcut_ڪنجي;

	ichar = tolower(ichar);
	بدل (ichar) {
	/* a-z for bootmenu entry > 9 */
	حالة 'a' ... 'z':
		shortcut_ڪنجي = ichar - 'a' + 9;
		اكسر;
	/* 1-9 for bootmenu entry <= 9 */
	حالة '1' ... '9':
		shortcut_ڪنجي = ichar - '1';
		اكسر;
	/* Reserve 0 for last option (aka Exit) */
	حالة '0':
	افتراضي:
		أرجع -1;
	}

	أرجع shortcut_ڪنجي;
}

تعداد bootmenu_ڪنجي bootmenu_autoboot_loop(بنية bootmenu_معلومات *menu,
					 بنية cli_ch_state *cch)
{
	تعداد bootmenu_ڪنجي ڪنجي = BKEY_NONE;
	صحيح i, c;

	طالما (menu->delay > 0) {
		صحيح ichar;

		إذا (ansi)
			لکو(ANSI_CURSOR_POSITION, menu->ڳڻپ + 5, 3);
		لکو("Hit any key to stop autoboot: %d ", menu->delay);
		لكل (i = 0; i < 100; ++i) {
			إذا (!tstc()) {
				وقت_جدول();
				mdelay(10);
				استمر;
			}

			menu->delay = -1;
			c = getchar();

			ichar = cli_ch_process(cch, c);

			بدل (ichar) {
			حالة '\0':
				ڪنجي = BKEY_NONE;
				اكسر;
			حالة '\n':
				ڪنجي = BKEY_SELECT;
				اكسر;
			حالة 0x3: /* ^C */
				ڪنجي = BKEY_QUIT;
				اكسر;
			حالة 'A' ... 'Z':
			حالة 'a' ... 'z':
			حالة '0' ... '9':
				ڪنجي = BKEY_SHORTCUT;
				اكسر;
			افتراضي:
				ڪنجي = BKEY_NONE;
				اكسر;
			}
			اكسر;
		}

		إذا (ڪنجي == BKEY_SHORTCUT)
			cch->shortcut_ڪنجي = bootmenu_conv_shortcut_ڪنجي(menu, ichar);

		إذا (menu->delay < 0)
			اكسر;

		--menu->delay;
	}

	إذا (ansi)
		لکو(ANSI_CURSOR_POSITION ANSI_CLEAR_سٽ, menu->ڳڻپ + 5, 1);

	إذا (menu->delay == 0)
		ڪنجي = BKEY_SELECT;

	أرجع ڪنجي;
}

تعداد bootmenu_ڪنجي bootmenu_conv_ڪنجي(صحيح ichar)
{
	تعداد bootmenu_ڪنجي ڪنجي;

	بدل (ichar) {
	حالة '\n':
		/* enter key was pressed */
		ڪنجي = BKEY_SELECT;
		اكسر;
	حالة CTL_CH('c'):
	حالة '\e':
		/* ^C was pressed */
		ڪنجي = BKEY_QUIT;
		اكسر;
	حالة CTL_CH('p'):
		ڪنجي = BKEY_مٿي;
		اكسر;
	حالة CTL_CH('n'):
		ڪنجي = BKEY_هيٺ_ڏانهن;
		اكسر;
	حالة CTL_CH('s'):
		ڪنجي = BKEY_SAVE;
		اكسر;
	حالة '+':
		ڪنجي = BKEY_PLUS;
		اكسر;
	حالة '-':
		ڪنجي = BKEY_MINUS;
		اكسر;
	حالة ' ':
		ڪنجي = BKEY_SPACE;
		اكسر;
	افتراضي:
		ڪنجي = BKEY_NONE;
		اكسر;
	}

	أرجع ڪنجي;
}

تعداد bootmenu_ڪنجي bootmenu_loop(بنية bootmenu_معلومات *menu,
				بنية cli_ch_state *cch)
{
	تعداد bootmenu_ڪنجي ڪنجي;
	صحيح c, errchar = 0;

	c = cli_ch_process(cch, 0);
	إذا (!c) {
		طالما (!c && !tstc()) {
			وقت_جدول();
			mdelay(10);
			c = cli_ch_process(cch, errchar);
			errchar = -ETIMEDOUT;
		}
		إذا (!c) {
			c = getchar();
			c = cli_ch_process(cch, c);
		}
	}

	ڪنجي = bootmenu_conv_ڪنجي(c);

	إذا (ڪنجي == BKEY_NONE && اکر_يا_انگ(c)) {
		ڪنجي = BKEY_SHORTCUT;
		cch->shortcut_ڪنجي = bootmenu_conv_shortcut_ڪنجي(menu, c);
	}

	أرجع ڪنجي;
}
