// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2010-2011 Calxeda, Inc.
 * Copyright (c) 2019, NVIDIA CORPORATION. All rights reserved.
 */

#अन्तर्भाव <ansi.h>
#अन्तर्भाव <cli.h>
#अन्तर्भाव <malloc.h>
#अन्तर्भाव <errno.h>
#अन्तर्भाव <linux/ctype.h>
#अन्तर्भाव <linux/delay.h>
#अन्तर्भाव <linux/list.h>
#अन्तर्भाव <watchdog.h>

#अन्तर्भाव "menu.h"

#परिभाषय ansi 1

/*
 * Internally, each item in a menu is represented by a struct menu_item.
 *
 * These items will be alloc'd and initialized by menu_item_add and destroyed
 * by menu_item_destroy, and the consumer of the interface never sees that
 * this struct is used at all.
 */
संरचना menu_item {
	वर्ण *कुञ्चिका;
	शून्य *दत्त;
	संरचना list_शीर्ष list;
};

/*
 * The menu is composed of a list of items along with settings and callbacks
 * provided by the user. An incomplete definition of this struct is available
 * in menu.h, but the full definition is here to prevent consumers from
 * relying on its contents.
 */
संरचना menu {
	संरचना menu_item *default_item;
	पूर्णाङ्क timeout;
	वर्ण *title;
	पूर्णाङ्क prompt;
	शून्य (*display_statusline)(संरचना menu *);
	शून्य (*item_दत्त_print)(शून्य *);
	वर्ण *(*item_choice)(शून्य *);
	bool (*need_reprint)(शून्य *);
	शून्य *item_choice_दत्त;
	संरचना list_शीर्ष वस्तूनि;
	पूर्णाङ्क item_cnt;
};

/*
 * An iterator function for menu items. callback will be called for each item
 * in m, with m, a pointer to the item, and extra being passed to callback. If
 * callback returns a value other than NULL, iteration stops and the value
 * return by callback is returned from menu_items_iter.  This allows it to be
 * used for search type operations. It is also safe for callback to remove the
 * item from the list of items.
 */
स्थिर अन्तःस्थ शून्य *menu_वस्तूनि_iter(संरचना menu *m,
		शून्य *(*callback)(संरचना menu *, संरचना menu_item *, शून्य *),
		शून्य *extra)
{
	संरचना list_शीर्ष *pos, *n;
	संरचना menu_item *item;
	शून्य *ret;

	list_for_each_safe(pos, n, &m->वस्तूनि) {
		item = list_प्रविष्टिः(pos, संरचना menu_item, list);

		ret = callback(m, item, extra);

		यदि (ret)
			प्रत्यागम ret;
	}

	प्रत्यागम रिक्त;
}

/*
 * Print a menu_item. If the consumer provided an item_data_print function
 * when creating the menu, call it with a pointer to the item's private data.
 * Otherwise, print the key of the item.
 */
स्थिर अन्तःस्थ शून्य *menu_item_print(संरचना menu *m,
				संरचना menu_item *item,
				शून्य *extra)
{
	यदि (!m->item_दत्त_print) {
		पङ्क्तिं_लिख(item->कुञ्चिका);
		putc('\n');
	} अन्यथा {
		m->item_दत्त_print(item->दत्त);
	}

	प्रत्यागम रिक्त;
}

/*
 * Free the memory used by a menu item. This includes the memory used by its
 * key.
 */
स्थिर अन्तःस्थ शून्य *menu_item_destroy(संरचना menu *m,
				संरचना menu_item *item,
				शून्य *extra)
{
	यदि (item->कुञ्चिका)
		मुक्त(item->कुञ्चिका);

	मुक्त(item);

	प्रत्यागम रिक्त;
}

/*
 * Display a menu so the user can make a choice of an item. First display its
 * title, if any, and then each item in the menu.
 */
स्थिर अन्तःस्थ शून्य menu_display(संरचना menu *m)
{
	यदि (m->need_reprint) {
		यदि (!m->need_reprint(m->item_choice_दत्त))
			प्रत्यागम;
	}

	यदि (m->title) {
		पङ्क्तिं_लिख(m->title);
		putc('\n');
	}
	यदि (m->display_statusline)
		m->display_statusline(m);

	menu_वस्तूनि_iter(m, menu_item_print, रिक्त);
}

/*
 * Check if an item's key matches a provided string, pointed to by extra. If
 * extra is NULL, an item with a NULL key will match. Otherwise, the item's
 * key has to match according to strcmp.
 *
 * This is called via menu_items_iter, so it returns a pointer to the item if
 * the key matches, and returns NULL otherwise.
 */
स्थिर अन्तःस्थ शून्य *menu_item_कुञ्चिका_मेलनम्(संरचना menu *m,
			संरचना menu_item *item, शून्य *extra)
{
	वर्ण *item_कुञ्चिका = extra;

	यदि (!item_कुञ्चिका || !item->कुञ्चिका) {
		यदि (item_कुञ्चिका == item->कुञ्चिका)
			प्रत्यागम item;

		प्रत्यागम रिक्त;
	}

	यदि (वाक्यं_तुलय(item->कुञ्चिका, item_कुञ्चिका) == 0)
		प्रत्यागम item;

	प्रत्यागम रिक्त;
}

/*
 * Find the first item with a key matching item_key, if any exists.
 */
स्थिर अन्तःस्थ संरचना menu_item *menu_item_by_कुञ्चिका(संरचना menu *m,
							वर्ण *item_कुञ्चिका)
{
	प्रत्यागम menu_वस्तूनि_iter(m, menu_item_कुञ्चिका_मेलनम्, item_कुञ्चिका);
}

/*
 * Set *choice to point to the default item's data, if any default item was
 * set, and returns 1. If no default item was set, returns -ENOENT.
 */
पूर्णाङ्क menu_default_choice(संरचना menu *m, शून्य **choice)
{
	यदि (m->default_item) {
		*choice = m->default_item->दत्त;
		प्रत्यागम 1;
	}

	प्रत्यागम -ENOENT;
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
स्थिर अन्तःस्थ पूर्णाङ्क menu_interactive_choice(संरचना menu *m, शून्य **choice)
{
	वर्ण cbuf[CONFIG_SYS_CBSIZE];
	संरचना menu_item *choice_item = रिक्त;
	पूर्णाङ्क readret;

	यावत् (!choice_item) {
		cbuf[0] = '\0';

		menu_display(m);

		यदि (!m->item_choice) {
			readret = cli_readline_into_बफर("Enter choice: ",
							   cbuf, m->timeout);

			यदि (readret >= 0) {
				choice_item = menu_item_by_कुञ्चिका(m, cbuf);
				यदि (!choice_item)
					प्रारूप_लिख("%s not found\n", cbuf);
			} अन्यथा यदि (readret == -1)  {
				प्रारूप_लिख("<INTERRUPT>\n");
				प्रत्यागम -EINTR;
			} अन्यथा {
				प्रत्यागम menu_default_choice(m, choice);
			}
		} अन्यथा {
			वर्ण *कुञ्चिका = m->item_choice(m->item_choice_दत्त);

			यदि (कुञ्चिका)
				choice_item = menu_item_by_कुञ्चिका(m, कुञ्चिका);
		}

		यदि (!choice_item)
			m->timeout = 0;
	}

	*choice = choice_item->दत्त;

	प्रत्यागम 1;
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
पूर्णाङ्क menu_default_समुच्चयः(संरचना menu *m, वर्ण *item_कुञ्चिका)
{
	संरचना menu_item *item;

	यदि (!m)
		प्रत्यागम -EINVAL;

	item = menu_item_by_कुञ्चिका(m, item_कुञ्चिका);

	यदि (!item)
		प्रत्यागम -ENOENT;

	m->default_item = item;

	प्रत्यागम 1;
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
पूर्णाङ्क menu_get_choice(संरचना menu *m, शून्य **choice)
{
	यदि (!m || !choice)
		प्रत्यागम -EINVAL;

	यदि (!m->item_cnt)
		प्रत्यागम -ENOENT;

	यदि (!m->prompt)
		प्रत्यागम menu_default_choice(m, choice);

	प्रत्यागम menu_interactive_choice(m, choice);
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
पूर्णाङ्क menu_item_सङ्कलय(संरचना menu *m, वर्ण *item_कुञ्चिका, शून्य *item_दत्त)
{
	संरचना menu_item *item;

	यदि (!m)
		प्रत्यागम -EINVAL;

	item = menu_item_by_कुञ्चिका(m, item_कुञ्चिका);

	यदि (item) {
		item->दत्त = item_दत्त;
		प्रत्यागम 1;
	}

	item = स्मृतिं_देहि(परिमाण *item);
	यदि (!item)
		प्रत्यागम -ENOMEM;

	item->कुञ्चिका = strdup(item_कुञ्चिका);

	यदि (!item->कुञ्चिका) {
		मुक्त(item);
		प्रत्यागम -ENOMEM;
	}

	item->दत्त = item_दत्त;

	list_सङ्कलय_पुच्छ(&item->list, &m->वस्तूनि);
	m->item_cnt++;

	प्रत्यागम 1;
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
संरचना menu *menu_create(वर्ण *title, पूर्णाङ्क timeout, पूर्णाङ्क prompt,
				शून्य (*display_statusline)(संरचना menu *),
				शून्य (*item_दत्त_print)(शून्य *),
				वर्ण *(*item_choice)(शून्य *),
				bool (*need_reprint)(शून्य *),
				शून्य *item_choice_दत्त)
{
	संरचना menu *m;

	m = स्मृतिं_देहि(परिमाण *m);

	यदि (!m)
		प्रत्यागम रिक्त;

	m->default_item = रिक्त;
	m->prompt = prompt;
	m->timeout = timeout;
	m->display_statusline = display_statusline;
	m->item_दत्त_print = item_दत्त_print;
	m->item_choice = item_choice;
	m->need_reprint = need_reprint;
	m->item_choice_दत्त = item_choice_दत्त;
	m->item_cnt = 0;

	यदि (title) {
		m->title = strdup(title);
		यदि (!m->title) {
			मुक्त(m);
			प्रत्यागम रिक्त;
		}
	} अन्यथा
		m->title = रिक्त;

	INIT_LIST_शीर्ष(&m->वस्तूनि);

	प्रत्यागम m;
}

/*
 * menu_destroy() - frees the memory used by a menu and its items.
 *
 * m - Points to a menu created by menu_create().
 *
 * Returns 1 if successful, or -EINVAL if m is NULL.
 */
पूर्णाङ्क menu_destroy(संरचना menu *m)
{
	यदि (!m)
		प्रत्यागम -EINVAL;

	menu_वस्तूनि_iter(m, menu_item_destroy, रिक्त);

	यदि (m->title)
		मुक्त(m->title);

	मुक्त(m);

	प्रत्यागम 1;
}

स्थिर पूर्णाङ्क bootmenu_conv_shortcut_कुञ्चिका(संरचना bootmenu_दत्त *menu, पूर्णाङ्क ichar)
{
	पूर्णाङ्क shortcut_कुञ्चिका;

	ichar = tolower(ichar);
	विकल्प (ichar) {
	/* a-z for bootmenu entry > 9 */
	पक्ष 'a' ... 'z':
		shortcut_कुञ्चिका = ichar - 'a' + 9;
		भङ्ग;
	/* 1-9 for bootmenu entry <= 9 */
	पक्ष '1' ... '9':
		shortcut_कुञ्चिका = ichar - '1';
		भङ्ग;
	/* Reserve 0 for last option (aka Exit) */
	पक्ष '0':
	स्वभाव:
		प्रत्यागम -1;
	}

	प्रत्यागम shortcut_कुञ्चिका;
}

गणना bootmenu_कुञ्चिका bootmenu_autoboot_loop(संरचना bootmenu_दत्त *menu,
					 संरचना cli_ch_state *cch)
{
	गणना bootmenu_कुञ्चिका कुञ्चिका = BKEY_NONE;
	पूर्णाङ्क i, c;

	यावत् (menu->delay > 0) {
		पूर्णाङ्क ichar;

		यदि (ansi)
			प्रारूप_लिख(ANSI_CURSOR_POSITION, menu->गणनम् + 5, 3);
		प्रारूप_लिख("Hit any key to stop autoboot: %d ", menu->delay);
		प्रति (i = 0; i < 100; ++i) {
			यदि (!tstc()) {
				कालसारणी();
				mdelay(10);
				अनुवर्त;
			}

			menu->delay = -1;
			c = अक्षरं_पठ();

			ichar = cli_ch_process(cch, c);

			विकल्प (ichar) {
			पक्ष '\0':
				कुञ्चिका = BKEY_NONE;
				भङ्ग;
			पक्ष '\n':
				कुञ्चिका = BKEY_SELECT;
				भङ्ग;
			पक्ष 0x3: /* ^C */
				कुञ्चिका = BKEY_QUIT;
				भङ्ग;
			पक्ष 'A' ... 'Z':
			पक्ष 'a' ... 'z':
			पक्ष '0' ... '9':
				कुञ्चिका = BKEY_SHORTCUT;
				भङ्ग;
			स्वभाव:
				कुञ्चिका = BKEY_NONE;
				भङ्ग;
			}
			भङ्ग;
		}

		यदि (कुञ्चिका == BKEY_SHORTCUT)
			cch->shortcut_कुञ्चिका = bootmenu_conv_shortcut_कुञ्चिका(menu, ichar);

		यदि (menu->delay < 0)
			भङ्ग;

		--menu->delay;
	}

	यदि (ansi)
		प्रारूप_लिख(ANSI_CURSOR_POSITION ANSI_CLEAR_पङ्क्तिः, menu->गणनम् + 5, 1);

	यदि (menu->delay == 0)
		कुञ्चिका = BKEY_SELECT;

	प्रत्यागम कुञ्चिका;
}

गणना bootmenu_कुञ्चिका bootmenu_conv_कुञ्चिका(पूर्णाङ्क ichar)
{
	गणना bootmenu_कुञ्चिका कुञ्चिका;

	विकल्प (ichar) {
	पक्ष '\n':
		/* enter key was pressed */
		कुञ्चिका = BKEY_SELECT;
		भङ्ग;
	पक्ष CTL_CH('c'):
	पक्ष '\e':
		/* ^C was pressed */
		कुञ्चिका = BKEY_QUIT;
		भङ्ग;
	पक्ष CTL_CH('p'):
		कुञ्चिका = BKEY_ऊर्ध्वम्;
		भङ्ग;
	पक्ष CTL_CH('n'):
		कुञ्चिका = BKEY_अधस्तात्;
		भङ्ग;
	पक्ष CTL_CH('s'):
		कुञ्चिका = BKEY_SAVE;
		भङ्ग;
	पक्ष '+':
		कुञ्चिका = BKEY_PLUS;
		भङ्ग;
	पक्ष '-':
		कुञ्चिका = BKEY_MINUS;
		भङ्ग;
	पक्ष ' ':
		कुञ्चिका = BKEY_SPACE;
		भङ्ग;
	स्वभाव:
		कुञ्चिका = BKEY_NONE;
		भङ्ग;
	}

	प्रत्यागम कुञ्चिका;
}

गणना bootmenu_कुञ्चिका bootmenu_loop(संरचना bootmenu_दत्त *menu,
				संरचना cli_ch_state *cch)
{
	गणना bootmenu_कुञ्चिका कुञ्चिका;
	पूर्णाङ्क c, errchar = 0;

	c = cli_ch_process(cch, 0);
	यदि (!c) {
		यावत् (!c && !tstc()) {
			कालसारणी();
			mdelay(10);
			c = cli_ch_process(cch, errchar);
			errchar = -ETIMEDOUT;
		}
		यदि (!c) {
			c = अक्षरं_पठ();
			c = cli_ch_process(cch, c);
		}
	}

	कुञ्चिका = bootmenu_conv_कुञ्चिका(c);

	यदि (कुञ्चिका == BKEY_NONE && अक्षरम्_अङ्कः_वा(c)) {
		कुञ्चिका = BKEY_SHORTCUT;
		cch->shortcut_कुञ्चिका = bootmenu_conv_shortcut_कुञ्चिका(menu, c);
	}

	प्रत्यागम कुञ्चिका;
}
