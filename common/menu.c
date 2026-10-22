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

#define ansi 1

/*
 * Internally, each item in a menu is represented by a struct menu_item.
 *
 * These items will be alloc'd and initialized by menu_item_add and destroyed
 * by menu_item_destroy, and the consumer of the interface never sees that
 * this struct is used at all.
 */
संरचना menu_item {
	अक्षर *किल्ली;
	शून्य *data;
	संरचना list_head list;
};

/*
 * The menu is composed of a list of items along with settings and callbacks
 * provided by the user. An incomplete definition of this struct is available
 * in menu.h, but the full definition is here to prevent consumers from
 * relying on its contents.
 */
संरचना menu {
	संरचना menu_item *default_item;
	पूर्णांक timeout;
	अक्षर *title;
	पूर्णांक prompt;
	शून्य (*display_statusline)(संरचना menu *);
	शून्य (*item_data_print)(शून्य *);
	अक्षर *(*item_choice)(शून्य *);
	bool (*need_reprint)(शून्य *);
	शून्य *item_choice_data;
	संरचना list_head वस्तुसंच;
	पूर्णांक item_cnt;
};

/*
 * An iterator function for menu items. callback will be called for each item
 * in m, with m, a pointer to the item, and extra being passed to callback. If
 * callback returns a value other than NULL, iteration stops and the value
 * return by callback is returned from menu_items_iter.  This allows it to be
 * used for search type operations. It is also safe for callback to remove the
 * item from the list of items.
 */
स्थिर ओळीत शून्य *menu_वस्तुसंच_iter(संरचना menu *m,
		शून्य *(*callback)(संरचना menu *, संरचना menu_item *, शून्य *),
		शून्य *extra)
{
	संरचना list_head *pos, *n;
	संरचना menu_item *item;
	शून्य *ret;

	list_for_each_safe(pos, n, &m->वस्तुसंच) {
		item = list_entry(pos, संरचना menu_item, list);

		ret = callback(m, item, extra);

		जर (ret)
			परत ret;
	}

	परत रिक्त;
}

/*
 * Print a menu_item. If the consumer provided an item_data_print function
 * when creating the menu, call it with a pointer to the item's private data.
 * Otherwise, print the key of the item.
 */
स्थिर ओळीत शून्य *menu_item_print(संरचना menu *m,
				संरचना menu_item *item,
				शून्य *extra)
{
	जर (!m->item_data_print) {
		माला_दो(item->किल्ली);
		अ_दो('\n');
	} अन्यथा {
		m->item_data_print(item->data);
	}

	परत रिक्त;
}

/*
 * Free the memory used by a menu item. This includes the memory used by its
 * key.
 */
स्थिर ओळीत शून्य *menu_item_destroy(संरचना menu *m,
				संरचना menu_item *item,
				शून्य *extra)
{
	जर (item->किल्ली)
		मुक्त(item->किल्ली);

	मुक्त(item);

	परत रिक्त;
}

/*
 * Display a menu so the user can make a choice of an item. First display its
 * title, if any, and then each item in the menu.
 */
स्थिर ओळीत शून्य menu_display(संरचना menu *m)
{
	जर (m->need_reprint) {
		जर (!m->need_reprint(m->item_choice_data))
			परत;
	}

	जर (m->title) {
		माला_दो(m->title);
		अ_दो('\n');
	}
	जर (m->display_statusline)
		m->display_statusline(m);

	menu_वस्तुसंच_iter(m, menu_item_print, रिक्त);
}

/*
 * Check if an item's key matches a provided string, pointed to by extra. If
 * extra is NULL, an item with a NULL key will match. Otherwise, the item's
 * key has to match according to strcmp.
 *
 * This is called via menu_items_iter, so it returns a pointer to the item if
 * the key matches, and returns NULL otherwise.
 */
स्थिर ओळीत शून्य *menu_item_किल्ली_match(संरचना menu *m,
			संरचना menu_item *item, शून्य *extra)
{
	अक्षर *item_किल्ली = extra;

	जर (!item_किल्ली || !item->किल्ली) {
		जर (item_किल्ली == item->किल्ली)
			परत item;

		परत रिक्त;
	}

	जर (म_भेद(item->किल्ली, item_किल्ली) == 0)
		परत item;

	परत रिक्त;
}

/*
 * Find the first item with a key matching item_key, if any exists.
 */
स्थिर ओळीत संरचना menu_item *menu_item_by_किल्ली(संरचना menu *m,
							अक्षर *item_किल्ली)
{
	परत menu_वस्तुसंच_iter(m, menu_item_किल्ली_match, item_किल्ली);
}

/*
 * Set *choice to point to the default item's data, if any default item was
 * set, and returns 1. If no default item was set, returns -ENOENT.
 */
पूर्णांक menu_default_choice(संरचना menu *m, शून्य **choice)
{
	जर (m->default_item) {
		*choice = m->default_item->data;
		परत 1;
	}

	परत -ENOENT;
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
स्थिर ओळीत पूर्णांक menu_interactive_choice(संरचना menu *m, शून्य **choice)
{
	अक्षर cbuf[CONFIG_SYS_CBSIZE];
	संरचना menu_item *choice_item = रिक्त;
	पूर्णांक readret;

	पर्यंत (!choice_item) {
		cbuf[0] = '\0';

		menu_display(m);

		जर (!m->item_choice) {
			readret = cli_readline_into_buffer("Enter choice: ",
							   cbuf, m->timeout);

			जर (readret >= 0) {
				choice_item = menu_item_by_किल्ली(m, cbuf);
				जर (!choice_item)
					म_लिखो("%s not found\n", cbuf);
			} अन्यथा जर (readret == -1)  {
				म_लिखो("<INTERRUPT>\n");
				परत -EINTR;
			} अन्यथा {
				परत menu_default_choice(m, choice);
			}
		} अन्यथा {
			अक्षर *किल्ली = m->item_choice(m->item_choice_data);

			जर (किल्ली)
				choice_item = menu_item_by_किल्ली(m, किल्ली);
		}

		जर (!choice_item)
			m->timeout = 0;
	}

	*choice = choice_item->data;

	परत 1;
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
पूर्णांक menu_default_set(संरचना menu *m, अक्षर *item_किल्ली)
{
	संरचना menu_item *item;

	जर (!m)
		परत -EINVAL;

	item = menu_item_by_किल्ली(m, item_किल्ली);

	जर (!item)
		परत -ENOENT;

	m->default_item = item;

	परत 1;
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
पूर्णांक menu_get_choice(संरचना menu *m, शून्य **choice)
{
	जर (!m || !choice)
		परत -EINVAL;

	जर (!m->item_cnt)
		परत -ENOENT;

	जर (!m->prompt)
		परत menu_default_choice(m, choice);

	परत menu_interactive_choice(m, choice);
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
पूर्णांक menu_item_add(संरचना menu *m, अक्षर *item_किल्ली, शून्य *item_data)
{
	संरचना menu_item *item;

	जर (!m)
		परत -EINVAL;

	item = menu_item_by_किल्ली(m, item_किल्ली);

	जर (item) {
		item->data = item_data;
		परत 1;
	}

	item = दो_स्मृति(आकार *item);
	जर (!item)
		परत -ENOMEM;

	item->किल्ली = strdup(item_किल्ली);

	जर (!item->किल्ली) {
		मुक्त(item);
		परत -ENOMEM;
	}

	item->data = item_data;

	list_add_tail(&item->list, &m->वस्तुसंच);
	m->item_cnt++;

	परत 1;
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
संरचना menu *menu_create(अक्षर *title, पूर्णांक timeout, पूर्णांक prompt,
				शून्य (*display_statusline)(संरचना menu *),
				शून्य (*item_data_print)(शून्य *),
				अक्षर *(*item_choice)(शून्य *),
				bool (*need_reprint)(शून्य *),
				शून्य *item_choice_data)
{
	संरचना menu *m;

	m = दो_स्मृति(आकार *m);

	जर (!m)
		परत रिक्त;

	m->default_item = रिक्त;
	m->prompt = prompt;
	m->timeout = timeout;
	m->display_statusline = display_statusline;
	m->item_data_print = item_data_print;
	m->item_choice = item_choice;
	m->need_reprint = need_reprint;
	m->item_choice_data = item_choice_data;
	m->item_cnt = 0;

	जर (title) {
		m->title = strdup(title);
		जर (!m->title) {
			मुक्त(m);
			परत रिक्त;
		}
	} अन्यथा
		m->title = रिक्त;

	INIT_LIST_HEAD(&m->वस्तुसंच);

	परत m;
}

/*
 * menu_destroy() - frees the memory used by a menu and its items.
 *
 * m - Points to a menu created by menu_create().
 *
 * Returns 1 if successful, or -EINVAL if m is NULL.
 */
पूर्णांक menu_destroy(संरचना menu *m)
{
	जर (!m)
		परत -EINVAL;

	menu_वस्तुसंच_iter(m, menu_item_destroy, रिक्त);

	जर (m->title)
		मुक्त(m->title);

	मुक्त(m);

	परत 1;
}

स्थिर पूर्णांक bootmenu_conv_shortcut_किल्ली(संरचना bootmenu_data *menu, पूर्णांक ichar)
{
	पूर्णांक shortcut_किल्ली;

	ichar = छोटे(ichar);
	निवड (ichar) {
	/* a-z for bootmenu entry > 9 */
	स्थिती 'a' ... 'z':
		shortcut_किल्ली = ichar - 'a' + 9;
		थांबा;
	/* 1-9 for bootmenu entry <= 9 */
	स्थिती '1' ... '9':
		shortcut_किल्ली = ichar - '1';
		थांबा;
	/* Reserve 0 for last option (aka Exit) */
	स्थिती '0':
	पूर्वनिर्धारित:
		परत -1;
	}

	परत shortcut_किल्ली;
}

गणना bootmenu_किल्ली bootmenu_autoboot_loop(संरचना bootmenu_data *menu,
					 संरचना cli_ch_state *cch)
{
	गणना bootmenu_किल्ली किल्ली = BKEY_NONE;
	पूर्णांक i, c;

	पर्यंत (menu->delay > 0) {
		पूर्णांक ichar;

		जर (ansi)
			म_लिखो(ANSI_CURSOR_POSITION, menu->मोजणी + 5, 3);
		म_लिखो("Hit any key to stop autoboot: %d ", menu->delay);
		साठी (i = 0; i < 100; ++i) {
			जर (!tstc()) {
				schedule();
				mdelay(10);
				सुरूठेवा;
			}

			menu->delay = -1;
			c = अक्षर_लो();

			ichar = cli_ch_process(cch, c);

			निवड (ichar) {
			स्थिती '\0':
				किल्ली = BKEY_NONE;
				थांबा;
			स्थिती '\n':
				किल्ली = BKEY_SELECT;
				थांबा;
			स्थिती 0x3: /* ^C */
				किल्ली = BKEY_QUIT;
				थांबा;
			स्थिती 'A' ... 'Z':
			स्थिती 'a' ... 'z':
			स्थिती '0' ... '9':
				किल्ली = BKEY_SHORTCUT;
				थांबा;
			पूर्वनिर्धारित:
				किल्ली = BKEY_NONE;
				थांबा;
			}
			थांबा;
		}

		जर (किल्ली == BKEY_SHORTCUT)
			cch->shortcut_किल्ली = bootmenu_conv_shortcut_किल्ली(menu, ichar);

		जर (menu->delay < 0)
			थांबा;

		--menu->delay;
	}

	जर (ansi)
		म_लिखो(ANSI_CURSOR_POSITION ANSI_CLEAR_ओळ, menu->मोजणी + 5, 1);

	जर (menu->delay == 0)
		किल्ली = BKEY_SELECT;

	परत किल्ली;
}

गणना bootmenu_किल्ली bootmenu_conv_किल्ली(पूर्णांक ichar)
{
	गणना bootmenu_किल्ली किल्ली;

	निवड (ichar) {
	स्थिती '\n':
		/* enter key was pressed */
		किल्ली = BKEY_SELECT;
		थांबा;
	स्थिती CTL_CH('c'):
	स्थिती '\e':
		/* ^C was pressed */
		किल्ली = BKEY_QUIT;
		थांबा;
	स्थिती CTL_CH('p'):
		किल्ली = BKEY_UP;
		थांबा;
	स्थिती CTL_CH('n'):
		किल्ली = BKEY_DOWN;
		थांबा;
	स्थिती CTL_CH('s'):
		किल्ली = BKEY_SAVE;
		थांबा;
	स्थिती '+':
		किल्ली = BKEY_PLUS;
		थांबा;
	स्थिती '-':
		किल्ली = BKEY_MINUS;
		थांबा;
	स्थिती ' ':
		किल्ली = BKEY_SPACE;
		थांबा;
	पूर्वनिर्धारित:
		किल्ली = BKEY_NONE;
		थांबा;
	}

	परत किल्ली;
}

गणना bootmenu_किल्ली bootmenu_loop(संरचना bootmenu_data *menu,
				संरचना cli_ch_state *cch)
{
	गणना bootmenu_किल्ली किल्ली;
	पूर्णांक c, errchar = 0;

	c = cli_ch_process(cch, 0);
	जर (!c) {
		पर्यंत (!c && !tstc()) {
			schedule();
			mdelay(10);
			c = cli_ch_process(cch, errchar);
			errchar = -ETIMEDOUT;
		}
		जर (!c) {
			c = अक्षर_लो();
			c = cli_ch_process(cch, c);
		}
	}

	किल्ली = bootmenu_conv_किल्ली(c);

	जर (किल्ली == BKEY_NONE && है_अक्षर_अंक(c)) {
		किल्ली = BKEY_SHORTCUT;
		cch->shortcut_किल्ली = bootmenu_conv_shortcut_किल्ली(menu, c);
	}

	परत किल्ली;
}
