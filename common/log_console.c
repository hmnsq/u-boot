// SPDX-License-Identifier: GPL-2.0+
/*
 * Logging support
 *
 * Copyright (c) 2017 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#ضمن <log.h>
#ضمن <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

ساكن صحيح log_console_emit(بنية log_device *ldev, بنية log_rec *rec)
{
	صحيح fmt = gd->log_fmt;
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
	إذا (!(rec->flags & LOGRECF_CONT) && fmt != BIT(LOGF_رسالة)) {
		add_space = true;
		إذا (fmt & BIT(LOGF_LEVEL))
			printf("%s.", log_get_level_name(rec->level));
		إذا (fmt & BIT(LOGF_CAT))
			printf("%s,", log_get_cat_name(rec->cat));
		إذا (fmt & BIT(LOGF_FILE))
			printf("%s:", rec->file);
		إذا (fmt & BIT(LOGF_سطر))
			printf("%d-", rec->سطر);
		إذا (fmt & BIT(LOGF_FUNC)) {
			إذا (CONFIG_IS_ENABLED(USE_TINY_PRINTF)) {
				printf("%s()", rec->func ?: "?");
			} وإلا {
				printf("%*s()", CONFIG_LOGF_FUNC_PAD,
				       rec->func ?: "?");
			}
		}
	}
	إذا (fmt & BIT(LOGF_رسالة))
		printf("%s%s", add_space ? " " : "", rec->رسالة);

	إرجاع 0;
}

LOG_DRIVER(console) = {
	.name	= "console",
	.emit	= log_console_emit,
	.flags	= LOGDF_ENABLE,
};
