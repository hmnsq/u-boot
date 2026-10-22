// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2010-2011 Calxeda, Inc.
 * Copyright (c) 2019, NVIDIA CORPORATION. All rights reserved.
 */

#समावेश <ansi.h>
#समावेश <cli.h>
#समावेश <malloc.h>
#समावेश <errno.h>
#समावेश <linux/ctype.h>
#समावेश <linux/delay.h>
#समावेश <linux/list.h>
#समावेश <watchdog.h>

#समावेश "menu.h"

#घोषणा ansi 1

/*
 * Internally, each item in a menu is represented by a struct menu_item.
 *
 * These items will be alloc'd and initialized by menu_item_add and destroyed
 * by menu_item_destroy, and the consumer of the interface never sees that
 * this struct is used at all.
 */
काष्ठा menu_item {
	अक्षर *कुंजी;
	व्योम *तथ्य;
	काष्ठा list_शीर्ष list;
};

/*
 * The menu is composed of a list of items along with settings and callbacks
 * provided by the user. An incomplete definition of this struct is available
 * in menu.h, but the full definition is here to prevent consumers from
 * relying on its contents.
 */
काष्ठा menu {
	काष्ठा menu_item *default_item;
	पूर्णांक timeout;
	अक्षर *title;
	पूर्णांक prompt;
	व्योम (*display_statusline)(काष्ठा menu *);
	व्योम (*item_तथ्य_print)(व्योम *);
	अक्षर *(*item_choice)(व्योम *);
	bool (*need_reprint)(व्योम *);
	व्योम *item_choice_तथ्य;
	काष्ठा list_शीर्ष वस्तुएँ;
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
अटल अंतरभूत व्योम *menu_वस्तुएँ_iter(काष्ठा menu *m,
		व्योम *(*callback)(काष्ठा menu *, काष्ठा menu_item *, व्योम *),
		व्योम *extra)
{
	काष्ठा list_शीर्ष *pos, *n;
	काष्ठा menu_item *item;
	व्योम *ret;

	list_for_each_safe(pos, n, &m->वस्तुएँ) {
		item = list_entry(pos, काष्ठा menu_item, list);

		ret = callback(m, item, extra);

		अगर (ret)
			वापस ret;
	}

	वापस शून्य;
}

/*
 * Print a menu_item. If the consumer provided an item_data_print function
 * when creating the menu, call it with a pointer to the item's private data.
 * Otherwise, print the key of the item.
 */
अटल अंतरभूत व्योम *menu_item_print(काष्ठा menu *m,
				काष्ठा menu_item *item,
				व्योम *extra)
{
	अगर (!m->item_तथ्य_print) {
		माला_दो(item->कुंजी);
		अ_दो('\n');
	} अन्यथा {
		m->item_तथ्य_print(item->तथ्य);
	}

	वापस शून्य;
}

/*
 * Free the memory used by a menu item. This includes the memory used by its
 * key.
 */
अटल अंतरभूत व्योम *menu_item_destroy(काष्ठा menu *m,
				काष्ठा menu_item *item,
				व्योम *extra)
{
	अगर (item->कुंजी)
		मुक्त(item->कुंजी);

	मुक्त(item);

	वापस शून्य;
}

/*
 * Display a menu so the user can make a choice of an item. First display its
 * title, if any, and then each item in the menu.
 */
अटल अंतरभूत व्योम menu_display(काष्ठा menu *m)
{
	अगर (m->need_reprint) {
		अगर (!m->need_reprint(m->item_choice_तथ्य))
			वापस;
	}

	अगर (m->title) {
		माला_दो(m->title);
		अ_दो('\n');
	}
	अगर (m->display_statusline)
		m->display_statusline(m);

	menu_वस्तुएँ_iter(m, menu_item_print, शून्य);
}

/*
 * Check if an item's key matches a provided string, pointed to by extra. If
 * extra is NULL, an item with a NULL key will match. Otherwise, the item's
 * key has to match according to strcmp.
 *
 * This is called via menu_items_iter, so it returns a pointer to the item if
 * the key matches, and returns NULL otherwise.
 */
अटल अंतरभूत व्योम *menu_item_कुंजी_match(काष्ठा menu *m,
			काष्ठा menu_item *item, व्योम *extra)
{
	अक्षर *item_कुंजी = extra;

	अगर (!item_कुंजी || !item->कुंजी) {
		अगर (item_कुंजी == item->कुंजी)
			वापस item;

		वापस शून्य;
	}

	अगर (म_भेद(item->कुंजी, item_कुंजी) == 0)
		वापस item;

	वापस शून्य;
}

/*
 * Find the first item with a key matching item_key, if any exists.
 */
अटल अंतरभूत काष्ठा menu_item *menu_item_by_कुंजी(काष्ठा menu *m,
							अक्षर *item_कुंजी)
{
	वापस menu_वस्तुएँ_iter(m, menu_item_कुंजी_match, item_कुंजी);
}

/*
 * Set *choice to point to the default item's data, if any default item was
 * set, and returns 1. If no default item was set, returns -ENOENT.
 */
पूर्णांक menu_default_choice(काष्ठा menu *m, व्योम **choice)
{
	अगर (m->default_item) {
		*choice = m->default_item->तथ्य;
		वापस 1;
	}

	वापस -ENOENT;
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
अटल अंतरभूत पूर्णांक menu_interactive_choice(काष्ठा menu *m, व्योम **choice)
{
	अक्षर cbuf[CONFIG_SYS_CBSIZE];
	काष्ठा menu_item *choice_item = शून्य;
	पूर्णांक readret;

	जबतक (!choice_item) {
		cbuf[0] = '\0';

		menu_display(m);

		अगर (!m->item_choice) {
			readret = cli_readline_into_बफर("Enter choice: ",
							   cbuf, m->timeout);

			अगर (readret >= 0) {
				choice_item = menu_item_by_कुंजी(m, cbuf);
				अगर (!choice_item)
					म_लिखो("%s not found\n", cbuf);
			} अन्यथा अगर (readret == -1)  {
				म_लिखो("<INTERRUPT>\n");
				वापस -EINTR;
			} अन्यथा {
				वापस menu_default_choice(m, choice);
			}
		} अन्यथा {
			अक्षर *कुंजी = m->item_choice(m->item_choice_तथ्य);

			अगर (कुंजी)
				choice_item = menu_item_by_कुंजी(m, कुंजी);
		}

		अगर (!choice_item)
			m->timeout = 0;
	}

	*choice = choice_item->तथ्य;

	वापस 1;
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
पूर्णांक menu_default_set(काष्ठा menu *m, अक्षर *item_कुंजी)
{
	काष्ठा menu_item *item;

	अगर (!m)
		वापस -EINVAL;

	item = menu_item_by_कुंजी(m, item_कुंजी);

	अगर (!item)
		वापस -ENOENT;

	m->default_item = item;

	वापस 1;
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
पूर्णांक menu_get_choice(काष्ठा menu *m, व्योम **choice)
{
	अगर (!m || !choice)
		वापस -EINVAL;

	अगर (!m->item_cnt)
		वापस -ENOENT;

	अगर (!m->prompt)
		वापस menu_default_choice(m, choice);

	वापस menu_interactive_choice(m, choice);
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
पूर्णांक menu_item_add(काष्ठा menu *m, अक्षर *item_कुंजी, व्योम *item_तथ्य)
{
	काष्ठा menu_item *item;

	अगर (!m)
		वापस -EINVAL;

	item = menu_item_by_कुंजी(m, item_कुंजी);

	अगर (item) {
		item->तथ्य = item_तथ्य;
		वापस 1;
	}

	item = दो_स्मृति(माप *item);
	अगर (!item)
		वापस -ENOMEM;

	item->कुंजी = strdup(item_कुंजी);

	अगर (!item->कुंजी) {
		मुक्त(item);
		वापस -ENOMEM;
	}

	item->तथ्य = item_तथ्य;

	list_add_पुच्छ(&item->list, &m->वस्तुएँ);
	m->item_cnt++;

	वापस 1;
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
काष्ठा menu *menu_create(अक्षर *title, पूर्णांक timeout, पूर्णांक prompt,
				व्योम (*display_statusline)(काष्ठा menu *),
				व्योम (*item_तथ्य_print)(व्योम *),
				अक्षर *(*item_choice)(व्योम *),
				bool (*need_reprint)(व्योम *),
				व्योम *item_choice_तथ्य)
{
	काष्ठा menu *m;

	m = दो_स्मृति(माप *m);

	अगर (!m)
		वापस शून्य;

	m->default_item = शून्य;
	m->prompt = prompt;
	m->timeout = timeout;
	m->display_statusline = display_statusline;
	m->item_तथ्य_print = item_तथ्य_print;
	m->item_choice = item_choice;
	m->need_reprint = need_reprint;
	m->item_choice_तथ्य = item_choice_तथ्य;
	m->item_cnt = 0;

	अगर (title) {
		m->title = strdup(title);
		अगर (!m->title) {
			मुक्त(m);
			वापस शून्य;
		}
	} अन्यथा
		m->title = शून्य;

	INIT_LIST_शीर्ष(&m->वस्तुएँ);

	वापस m;
}

/*
 * menu_destroy() - frees the memory used by a menu and its items.
 *
 * m - Points to a menu created by menu_create().
 *
 * Returns 1 if successful, or -EINVAL if m is NULL.
 */
पूर्णांक menu_destroy(काष्ठा menu *m)
{
	अगर (!m)
		वापस -EINVAL;

	menu_वस्तुएँ_iter(m, menu_item_destroy, शून्य);

	अगर (m->title)
		मुक्त(m->title);

	मुक्त(m);

	वापस 1;
}

अटल पूर्णांक bootmenu_conv_shortcut_कुंजी(काष्ठा bootmenu_तथ्य *menu, पूर्णांक ichar)
{
	पूर्णांक shortcut_कुंजी;

	ichar = छोटे(ichar);
	चयन (ichar) {
	/* a-z for bootmenu entry > 9 */
	हाल 'a' ... 'z':
		shortcut_कुंजी = ichar - 'a' + 9;
		अवरोध;
	/* 1-9 for bootmenu entry <= 9 */
	हाल '1' ... '9':
		shortcut_कुंजी = ichar - '1';
		अवरोध;
	/* Reserve 0 for last option (aka Exit) */
	हाल '0':
	शेष:
		वापस -1;
	}

	वापस shortcut_कुंजी;
}

क्रमागत bootmenu_कुंजी bootmenu_autoboot_loop(काष्ठा bootmenu_तथ्य *menu,
					 काष्ठा cli_ch_state *cch)
{
	क्रमागत bootmenu_कुंजी कुंजी = BKEY_NONE;
	पूर्णांक i, c;

	जबतक (menu->delay > 0) {
		पूर्णांक ichar;

		अगर (ansi)
			म_लिखो(ANSI_CURSOR_POSITION, menu->गिनती + 5, 3);
		म_लिखो("Hit any key to stop autoboot: %d ", menu->delay);
		क्रम (i = 0; i < 100; ++i) {
			अगर (!tstc()) {
				schedule();
				mdelay(10);
				जारी;
			}

			menu->delay = -1;
			c = अक्षर_लो();

			ichar = cli_ch_process(cch, c);

			चयन (ichar) {
			हाल '\0':
				कुंजी = BKEY_NONE;
				अवरोध;
			हाल '\n':
				कुंजी = BKEY_SELECT;
				अवरोध;
			हाल 0x3: /* ^C */
				कुंजी = BKEY_QUIT;
				अवरोध;
			हाल 'A' ... 'Z':
			हाल 'a' ... 'z':
			हाल '0' ... '9':
				कुंजी = BKEY_SHORTCUT;
				अवरोध;
			शेष:
				कुंजी = BKEY_NONE;
				अवरोध;
			}
			अवरोध;
		}

		अगर (कुंजी == BKEY_SHORTCUT)
			cch->shortcut_कुंजी = bootmenu_conv_shortcut_कुंजी(menu, ichar);

		अगर (menu->delay < 0)
			अवरोध;

		--menu->delay;
	}

	अगर (ansi)
		म_लिखो(ANSI_CURSOR_POSITION ANSI_CLEAR_पंक्ति, menu->गिनती + 5, 1);

	अगर (menu->delay == 0)
		कुंजी = BKEY_SELECT;

	वापस कुंजी;
}

क्रमागत bootmenu_कुंजी bootmenu_conv_कुंजी(पूर्णांक ichar)
{
	क्रमागत bootmenu_कुंजी कुंजी;

	चयन (ichar) {
	हाल '\n':
		/* enter key was pressed */
		कुंजी = BKEY_SELECT;
		अवरोध;
	हाल CTL_CH('c'):
	हाल '\e':
		/* ^C was pressed */
		कुंजी = BKEY_QUIT;
		अवरोध;
	हाल CTL_CH('p'):
		कुंजी = BKEY_UP;
		अवरोध;
	हाल CTL_CH('n'):
		कुंजी = BKEY_DOWN;
		अवरोध;
	हाल CTL_CH('s'):
		कुंजी = BKEY_SAVE;
		अवरोध;
	हाल '+':
		कुंजी = BKEY_PLUS;
		अवरोध;
	हाल '-':
		कुंजी = BKEY_MINUS;
		अवरोध;
	हाल ' ':
		कुंजी = BKEY_SPACE;
		अवरोध;
	शेष:
		कुंजी = BKEY_NONE;
		अवरोध;
	}

	वापस कुंजी;
}

क्रमागत bootmenu_कुंजी bootmenu_loop(काष्ठा bootmenu_तथ्य *menu,
				काष्ठा cli_ch_state *cch)
{
	क्रमागत bootmenu_कुंजी कुंजी;
	पूर्णांक c, errchar = 0;

	c = cli_ch_process(cch, 0);
	अगर (!c) {
		जबतक (!c && !tstc()) {
			schedule();
			mdelay(10);
			c = cli_ch_process(cch, errchar);
			errchar = -ETIMEDOUT;
		}
		अगर (!c) {
			c = अक्षर_लो();
			c = cli_ch_process(cch, c);
		}
	}

	कुंजी = bootmenu_conv_कुंजी(c);

	अगर (कुंजी == BKEY_NONE && है_अक्षर_अंक(c)) {
		कुंजी = BKEY_SHORTCUT;
		cch->shortcut_कुंजी = bootmenu_conv_shortcut_कुंजी(menu, c);
	}

	वापस कुंजी;
}
