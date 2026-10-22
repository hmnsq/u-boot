// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2010-2011 Calxeda, Inc.
 * Copyright (c) 2019, NVIDIA CORPORATION. All rights reserved.
 */

#అన్తర్భావ <ansi.h>
#అన్తర్భావ <cli.h>
#అన్తర్భావ <malloc.h>
#అన్తర్భావ <errno.h>
#అన్తర్భావ <linux/ctype.h>
#అన్తర్భావ <linux/delay.h>
#అన్తర్భావ <linux/list.h>
#అన్తర్భావ <watchdog.h>

#అన్తర్భావ "menu.h"

#నిర్వచించు ansi 1

/*
 * Internally, each item in a menu is represented by a struct menu_item.
 *
 * These items will be alloc'd and initialized by menu_item_add and destroyed
 * by menu_item_destroy, and the consumer of the interface never sees that
 * this struct is used at all.
 */
నిర్మాణం menu_అంశం {
	అక్షరం *కీలకం;
	శూన్య *దత్తాంశం;
	నిర్మాణం list_తల list;
};

/*
 * The menu is composed of a list of items along with settings and callbacks
 * provided by the user. An incomplete definition of this struct is available
 * in menu.h, but the full definition is here to prevent consumers from
 * relying on its contents.
 */
నిర్మాణం menu {
	నిర్మాణం menu_అంశం *default_అంశం;
	పూర్ణాంకం timeout;
	అక్షరం *title;
	పూర్ణాంకం prompt;
	శూన్య (*display_statusline)(నిర్మాణం menu *);
	శూన్య (*అంశం_దత్తాంశం_print)(శూన్య *);
	అక్షరం *(*అంశం_choice)(శూన్య *);
	bool (*need_reprint)(శూన్య *);
	శూన్య *అంశం_choice_దత్తాంశం;
	నిర్మాణం list_తల వస్తువులు;
	పూర్ణాంకం అంశం_cnt;
};

/*
 * An iterator function for menu items. callback will be called for each item
 * in m, with m, a pointer to the item, and extra being passed to callback. If
 * callback returns a value other than NULL, iteration stops and the value
 * return by callback is returned from menu_items_iter.  This allows it to be
 * used for search type operations. It is also safe for callback to remove the
 * item from the list of items.
 */
స్థిర అంతఃవరుస శూన్య *menu_వస్తువులు_iter(నిర్మాణం menu *m,
		శూన్య *(*callback)(నిర్మాణం menu *, నిర్మాణం menu_అంశం *, శూన్య *),
		శూన్య *extra)
{
	నిర్మాణం list_తల *pos, *n;
	నిర్మాణం menu_అంశం *అంశం;
	శూన్య *ret;

	list_for_each_సురక్షిత(pos, n, &m->వస్తువులు) {
		అంశం = list_నమోదు(pos, నిర్మాణం menu_అంశం, list);

		ret = callback(m, అంశం, extra);

		ఒకవేళ (ret)
			తిరిగి ret;
	}

	తిరిగి ఖాళీ;
}

/*
 * Print a menu_item. If the consumer provided an item_data_print function
 * when creating the menu, call it with a pointer to the item's private data.
 * Otherwise, print the key of the item.
 */
స్థిర అంతఃవరుస శూన్య *menu_అంశం_print(నిర్మాణం menu *m,
				నిర్మాణం menu_అంశం *అంశం,
				శూన్య *extra)
{
	ఒకవేళ (!m->అంశం_దత్తాంశం_print) {
		puts(అంశం->కీలకం);
		putc('\n');
	} లేకపోతే {
		m->అంశం_దత్తాంశం_print(అంశం->దత్తాంశం);
	}

	తిరిగి ఖాళీ;
}

/*
 * Free the memory used by a menu item. This includes the memory used by its
 * key.
 */
స్థిర అంతఃవరుస శూన్య *menu_అంశం_destroy(నిర్మాణం menu *m,
				నిర్మాణం menu_అంశం *అంశం,
				శూన్య *extra)
{
	ఒకవేళ (అంశం->కీలకం)
		విడుదల(అంశం->కీలకం);

	విడుదల(అంశం);

	తిరిగి ఖాళీ;
}

/*
 * Display a menu so the user can make a choice of an item. First display its
 * title, if any, and then each item in the menu.
 */
స్థిర అంతఃవరుస శూన్య menu_display(నిర్మాణం menu *m)
{
	ఒకవేళ (m->need_reprint) {
		ఒకవేళ (!m->need_reprint(m->అంశం_choice_దత్తాంశం))
			తిరిగి;
	}

	ఒకవేళ (m->title) {
		puts(m->title);
		putc('\n');
	}
	ఒకవేళ (m->display_statusline)
		m->display_statusline(m);

	menu_వస్తువులు_iter(m, menu_అంశం_print, ఖాళీ);
}

/*
 * Check if an item's key matches a provided string, pointed to by extra. If
 * extra is NULL, an item with a NULL key will match. Otherwise, the item's
 * key has to match according to strcmp.
 *
 * This is called via menu_items_iter, so it returns a pointer to the item if
 * the key matches, and returns NULL otherwise.
 */
స్థిర అంతఃవరుస శూన్య *menu_అంశం_కీలకం_సరిపోలిక(నిర్మాణం menu *m,
			నిర్మాణం menu_అంశం *అంశం, శూన్య *extra)
{
	అక్షరం *అంశం_కీలకం = extra;

	ఒకవేళ (!అంశం_కీలకం || !అంశం->కీలకం) {
		ఒకవేళ (అంశం_కీలకం == అంశం->కీలకం)
			తిరిగి అంశం;

		తిరిగి ఖాళీ;
	}

	ఒకవేళ (strcmp(అంశం->కీలకం, అంశం_కీలకం) == 0)
		తిరిగి అంశం;

	తిరిగి ఖాళీ;
}

/*
 * Find the first item with a key matching item_key, if any exists.
 */
స్థిర అంతఃవరుస నిర్మాణం menu_అంశం *menu_అంశం_by_కీలకం(నిర్మాణం menu *m,
							అక్షరం *అంశం_కీలకం)
{
	తిరిగి menu_వస్తువులు_iter(m, menu_అంశం_కీలకం_సరిపోలిక, అంశం_కీలకం);
}

/*
 * Set *choice to point to the default item's data, if any default item was
 * set, and returns 1. If no default item was set, returns -ENOENT.
 */
పూర్ణాంకం menu_default_choice(నిర్మాణం menu *m, శూన్య **choice)
{
	ఒకవేళ (m->default_అంశం) {
		*choice = m->default_అంశం->దత్తాంశం;
		తిరిగి 1;
	}

	తిరిగి -ENOENT;
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
స్థిర అంతఃవరుస పూర్ణాంకం menu_interactive_choice(నిర్మాణం menu *m, శూన్య **choice)
{
	అక్షరం cbuf[CONFIG_SYS_CBSIZE];
	నిర్మాణం menu_అంశం *choice_అంశం = ఖాళీ;
	పూర్ణాంకం readret;

	అయితే (!choice_అంశం) {
		cbuf[0] = '\0';

		menu_display(m);

		ఒకవేళ (!m->అంశం_choice) {
			readret = cli_readline_into_బఫరు("Enter choice: ",
							   cbuf, m->timeout);

			ఒకవేళ (readret >= 0) {
				choice_అంశం = menu_అంశం_by_కీలకం(m, cbuf);
				ఒకవేళ (!choice_అంశం)
					రాయి("%s not found\n", cbuf);
			} లేకపోతే ఒకవేళ (readret == -1)  {
				రాయి("<INTERRUPT>\n");
				తిరిగి -EINTR;
			} లేకపోతే {
				తిరిగి menu_default_choice(m, choice);
			}
		} లేకపోతే {
			అక్షరం *కీలకం = m->అంశం_choice(m->అంశం_choice_దత్తాంశం);

			ఒకవేళ (కీలకం)
				choice_అంశం = menu_అంశం_by_కీలకం(m, కీలకం);
		}

		ఒకవేళ (!choice_అంశం)
			m->timeout = 0;
	}

	*choice = choice_అంశం->దత్తాంశం;

	తిరిగి 1;
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
పూర్ణాంకం menu_default_సమితి(నిర్మాణం menu *m, అక్షరం *అంశం_కీలకం)
{
	నిర్మాణం menu_అంశం *అంశం;

	ఒకవేళ (!m)
		తిరిగి -EINVAL;

	అంశం = menu_అంశం_by_కీలకం(m, అంశం_కీలకం);

	ఒకవేళ (!అంశం)
		తిరిగి -ENOENT;

	m->default_అంశం = అంశం;

	తిరిగి 1;
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
పూర్ణాంకం menu_get_choice(నిర్మాణం menu *m, శూన్య **choice)
{
	ఒకవేళ (!m || !choice)
		తిరిగి -EINVAL;

	ఒకవేళ (!m->అంశం_cnt)
		తిరిగి -ENOENT;

	ఒకవేళ (!m->prompt)
		తిరిగి menu_default_choice(m, choice);

	తిరిగి menu_interactive_choice(m, choice);
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
పూర్ణాంకం menu_అంశం_కూడు(నిర్మాణం menu *m, అక్షరం *అంశం_కీలకం, శూన్య *అంశం_దత్తాంశం)
{
	నిర్మాణం menu_అంశం *అంశం;

	ఒకవేళ (!m)
		తిరిగి -EINVAL;

	అంశం = menu_అంశం_by_కీలకం(m, అంశం_కీలకం);

	ఒకవేళ (అంశం) {
		అంశం->దత్తాంశం = అంశం_దత్తాంశం;
		తిరిగి 1;
	}

	అంశం = జ్ఞాపకం_ఇవ్వు(పరిమాణం *అంశం);
	ఒకవేళ (!అంశం)
		తిరిగి -ENOMEM;

	అంశం->కీలకం = strdup(అంశం_కీలకం);

	ఒకవేళ (!అంశం->కీలకం) {
		విడుదల(అంశం);
		తిరిగి -ENOMEM;
	}

	అంశం->దత్తాంశం = అంశం_దత్తాంశం;

	list_కూడు_తోక(&అంశం->list, &m->వస్తువులు);
	m->అంశం_cnt++;

	తిరిగి 1;
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
నిర్మాణం menu *menu_create(అక్షరం *title, పూర్ణాంకం timeout, పూర్ణాంకం prompt,
				శూన్య (*display_statusline)(నిర్మాణం menu *),
				శూన్య (*అంశం_దత్తాంశం_print)(శూన్య *),
				అక్షరం *(*అంశం_choice)(శూన్య *),
				bool (*need_reprint)(శూన్య *),
				శూన్య *అంశం_choice_దత్తాంశం)
{
	నిర్మాణం menu *m;

	m = జ్ఞాపకం_ఇవ్వు(పరిమాణం *m);

	ఒకవేళ (!m)
		తిరిగి ఖాళీ;

	m->default_అంశం = ఖాళీ;
	m->prompt = prompt;
	m->timeout = timeout;
	m->display_statusline = display_statusline;
	m->అంశం_దత్తాంశం_print = అంశం_దత్తాంశం_print;
	m->అంశం_choice = అంశం_choice;
	m->need_reprint = need_reprint;
	m->అంశం_choice_దత్తాంశం = అంశం_choice_దత్తాంశం;
	m->అంశం_cnt = 0;

	ఒకవేళ (title) {
		m->title = strdup(title);
		ఒకవేళ (!m->title) {
			విడుదల(m);
			తిరిగి ఖాళీ;
		}
	} లేకపోతే
		m->title = ఖాళీ;

	INIT_LIST_తల(&m->వస్తువులు);

	తిరిగి m;
}

/*
 * menu_destroy() - frees the memory used by a menu and its items.
 *
 * m - Points to a menu created by menu_create().
 *
 * Returns 1 if successful, or -EINVAL if m is NULL.
 */
పూర్ణాంకం menu_destroy(నిర్మాణం menu *m)
{
	ఒకవేళ (!m)
		తిరిగి -EINVAL;

	menu_వస్తువులు_iter(m, menu_అంశం_destroy, ఖాళీ);

	ఒకవేళ (m->title)
		విడుదల(m->title);

	విడుదల(m);

	తిరిగి 1;
}

స్థిర పూర్ణాంకం bootmenu_conv_shortcut_కీలకం(నిర్మాణం bootmenu_దత్తాంశం *menu, పూర్ణాంకం ichar)
{
	పూర్ణాంకం shortcut_కీలకం;

	ichar = tolower(ichar);
	ఎంపిక (ichar) {
	/* a-z for bootmenu entry > 9 */
	స్థితి 'a' ... 'z':
		shortcut_కీలకం = ichar - 'a' + 9;
		ఆపు;
	/* 1-9 for bootmenu entry <= 9 */
	స్థితి '1' ... '9':
		shortcut_కీలకం = ichar - '1';
		ఆపు;
	/* Reserve 0 for last option (aka Exit) */
	స్థితి '0':
	అప్రమేయ:
		తిరిగి -1;
	}

	తిరిగి shortcut_కీలకం;
}

గణన bootmenu_కీలకం bootmenu_autoboot_loop(నిర్మాణం bootmenu_దత్తాంశం *menu,
					 నిర్మాణం cli_ch_state *cch)
{
	గణన bootmenu_కీలకం కీలకం = BKEY_NONE;
	పూర్ణాంకం i, c;

	అయితే (menu->delay > 0) {
		పూర్ణాంకం ichar;

		ఒకవేళ (ansi)
			రాయి(ANSI_CURSOR_POSITION, menu->లెక్క + 5, 3);
		రాయి("Hit any key to stop autoboot: %d ", menu->delay);
		కొరకు (i = 0; i < 100; ++i) {
			ఒకవేళ (!tstc()) {
				కాలపట్టిక();
				mdelay(10);
				కొనసాగు;
			}

			menu->delay = -1;
			c = getchar();

			ichar = cli_ch_process(cch, c);

			ఎంపిక (ichar) {
			స్థితి '\0':
				కీలకం = BKEY_NONE;
				ఆపు;
			స్థితి '\n':
				కీలకం = BKEY_SELECT;
				ఆపు;
			స్థితి 0x3: /* ^C */
				కీలకం = BKEY_QUIT;
				ఆపు;
			స్థితి 'A' ... 'Z':
			స్థితి 'a' ... 'z':
			స్థితి '0' ... '9':
				కీలకం = BKEY_SHORTCUT;
				ఆపు;
			అప్రమేయ:
				కీలకం = BKEY_NONE;
				ఆపు;
			}
			ఆపు;
		}

		ఒకవేళ (కీలకం == BKEY_SHORTCUT)
			cch->shortcut_కీలకం = bootmenu_conv_shortcut_కీలకం(menu, ichar);

		ఒకవేళ (menu->delay < 0)
			ఆపు;

		--menu->delay;
	}

	ఒకవేళ (ansi)
		రాయి(ANSI_CURSOR_POSITION ANSI_CLEAR_పంక్తి, menu->లెక్క + 5, 1);

	ఒకవేళ (menu->delay == 0)
		కీలకం = BKEY_SELECT;

	తిరిగి కీలకం;
}

గణన bootmenu_కీలకం bootmenu_conv_కీలకం(పూర్ణాంకం ichar)
{
	గణన bootmenu_కీలకం కీలకం;

	ఎంపిక (ichar) {
	స్థితి '\n':
		/* enter key was pressed */
		కీలకం = BKEY_SELECT;
		ఆపు;
	స్థితి CTL_CH('c'):
	స్థితి '\e':
		/* ^C was pressed */
		కీలకం = BKEY_QUIT;
		ఆపు;
	స్థితి CTL_CH('p'):
		కీలకం = BKEY_పైకి;
		ఆపు;
	స్థితి CTL_CH('n'):
		కీలకం = BKEY_కిందికి;
		ఆపు;
	స్థితి CTL_CH('s'):
		కీలకం = BKEY_SAVE;
		ఆపు;
	స్థితి '+':
		కీలకం = BKEY_PLUS;
		ఆపు;
	స్థితి '-':
		కీలకం = BKEY_MINUS;
		ఆపు;
	స్థితి ' ':
		కీలకం = BKEY_SPACE;
		ఆపు;
	అప్రమేయ:
		కీలకం = BKEY_NONE;
		ఆపు;
	}

	తిరిగి కీలకం;
}

గణన bootmenu_కీలకం bootmenu_loop(నిర్మాణం bootmenu_దత్తాంశం *menu,
				నిర్మాణం cli_ch_state *cch)
{
	గణన bootmenu_కీలకం కీలకం;
	పూర్ణాంకం c, errchar = 0;

	c = cli_ch_process(cch, 0);
	ఒకవేళ (!c) {
		అయితే (!c && !tstc()) {
			కాలపట్టిక();
			mdelay(10);
			c = cli_ch_process(cch, errchar);
			errchar = -ETIMEDOUT;
		}
		ఒకవేళ (!c) {
			c = getchar();
			c = cli_ch_process(cch, c);
		}
	}

	కీలకం = bootmenu_conv_కీలకం(c);

	ఒకవేళ (కీలకం == BKEY_NONE && అక్షరమా_అంకెనా(c)) {
		కీలకం = BKEY_SHORTCUT;
		cch->shortcut_కీలకం = bootmenu_conv_shortcut_కీలకం(menu, c);
	}

	తిరిగి కీలకం;
}
