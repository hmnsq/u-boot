// SPDX-License-Identifier: GPL-2.0+
/*
 * Logging support
 *
 * Copyright (c) 2017 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#অন্তর্ভাব <log.h>
#অন্তর্ভাব <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

স্থির পূর্ণাঙ্ক log_console_emit(সংরচনা log_device *ldev, সংরচনা log_rec *rec)
{
	পূর্ণাঙ্ক fmt = gd->log_fmt;
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
	যদি (!(rec->flags & LOGRECF_CONT) && fmt != BIT(LOGF_বার্তা)) {
		add_space = true;
		যদি (fmt & BIT(LOGF_LEVEL))
			printf("%s.", log_get_level_name(rec->level));
		যদি (fmt & BIT(LOGF_CAT))
			printf("%s,", log_get_cat_name(rec->cat));
		যদি (fmt & BIT(LOGF_FILE))
			printf("%s:", rec->file);
		যদি (fmt & BIT(LOGF_সারি))
			printf("%d-", rec->সারি);
		যদি (fmt & BIT(LOGF_FUNC)) {
			যদি (CONFIG_IS_ENABLED(USE_TINY_PRINTF)) {
				printf("%s()", rec->func ?: "?");
			} অন্যথা {
				printf("%*s()", CONFIG_LOGF_FUNC_PAD,
				       rec->func ?: "?");
			}
		}
	}
	যদি (fmt & BIT(LOGF_বার্তা))
		printf("%s%s", add_space ? " " : "", rec->বার্তা);

	প্রত্যাগম 0;
}

LOG_DRIVER(console) = {
	.name	= "console",
	.emit	= log_console_emit,
	.flags	= LOGDF_ENABLE,
};
