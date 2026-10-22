// SPDX-License-Identifier: GPL-2.0+
/*
 * Logging support
 *
 * Copyright (c) 2017 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <log.h>
#include <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

static puur_naa_nka log_console_emit(sa_nracanaa log_device *ldev, sa_nracanaa log_rec *rec)
{
	puur_naa_nka fmt = gd->log_fmt;
	bool add_space = false;

	/*
	 * The output format is designed to give someone a fighting chance of
	 * figuring out which field is which:
	 *    - level is in CAPS
	 *    - cat is lower case and ends with comma
	 *    - file normally has a .c extension and ends with a colon
	 *    - line is integer and ends with a -
	 *    - function is an identifier and ends with ()
	 *    - message has a space before it unless it is on its own
	 */
	ᱡᱩᱫᱤ (!(rec->flags & LOGRECF_CONT) && fmt != BIT(LOGF_MSG)) {
		add_space = true;
		ᱡᱩᱫᱤ (fmt & BIT(LOGF_LEVEL))
			printf("%s.", log_get_level_name(rec->level));
		ᱡᱩᱫᱤ (fmt & BIT(LOGF_CAT))
			printf("%s,", log_get_cat_name(rec->cat));
		ᱡᱩᱫᱤ (fmt & BIT(LOGF_FILE))
			printf("%s:", rec->file);
		ᱡᱩᱫᱤ (fmt & BIT(LOGF_LINE))
			printf("%d-", rec->line);
		ᱡᱩᱫᱤ (fmt & BIT(LOGF_FUNC)) {
			ᱡᱩᱫᱤ (CONFIG_IS_ENABLED(USE_TINY_PRINTF)) {
				printf("%s()", rec->func ?: "?");
			} ᱵᱟᱝ {
				printf("%*s()", CONFIG_LOGF_FUNC_PAD,
				       rec->func ?: "?");
			}
		}
	}
	ᱡᱩᱫᱤ (fmt & BIT(LOGF_MSG))
		printf("%s%s", add_space ? " " : "", rec->msg);

	ᱨᱩᱣᱟᱹᱲ 0;
}

LOG_DRIVER(console) = {
	.name	= "console",
	.emit	= log_console_emit,
	.flags	= LOGDF_ENABLE,
};
