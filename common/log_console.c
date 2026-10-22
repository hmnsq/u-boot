// SPDX-License-Identifier: GPL-2.0+
/*
 * Logging support
 *
 * Copyright (c) 2017 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#شامل <log.h>
#شامل <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

ایستا صحیح log_console_emit(ساختار log_device *ldev, ساختار log_rec *rec)
{
	صحیح fmt = gd->log_fmt;
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
	اگر (!(rec->flags & LOGRECF_CONT) && fmt != BIT(LOGF_پیام)) {
		add_space = true;
		اگر (fmt & BIT(LOGF_LEVEL))
			printf("%s.", log_get_level_name(rec->level));
		اگر (fmt & BIT(LOGF_CAT))
			printf("%s,", log_get_cat_name(rec->cat));
		اگر (fmt & BIT(LOGF_FILE))
			printf("%s:", rec->file);
		اگر (fmt & BIT(LOGF_سطر))
			printf("%d-", rec->سطر);
		اگر (fmt & BIT(LOGF_FUNC)) {
			اگر (CONFIG_IS_ENABLED(USE_TINY_PRINTF)) {
				printf("%s()", rec->func ?: "?");
			} وگرنه {
				printf("%*s()", CONFIG_LOGF_FUNC_PAD,
				       rec->func ?: "?");
			}
		}
	}
	اگر (fmt & BIT(LOGF_پیام))
		printf("%s%s", add_space ? " " : "", rec->پیام);

	بازگشت 0;
}

LOG_DRIVER(console) = {
	.name	= "console",
	.emit	= log_console_emit,
	.flags	= LOGDF_ENABLE,
};
