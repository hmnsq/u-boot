// SPDX-License-Identifier: GPL-2.0+
/*
 * Logging support
 *
 * Copyright (c) 2017 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#ضمن <log.h>
#ضمن <asm/global_data.h>

DECLARE_GLOBAL_داده_PTR;

ساكن صحيح log_console_emit(بنية log_device *ldev, بنية log_rec *rec)
{
	صحيح fmt = gd->log_fmt;
	bool جمع_کن_space = false;

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
	إذا (!(rec->flags & LOGRECF_CONT) && fmt != بیت(LOGF_پیام)) {
		جمع_کن_space = true;
		إذا (fmt & بیت(LOGF_LEVEL))
			چاپ_کن("%s.", log_get_level_name(rec->level));
		إذا (fmt & بیت(LOGF_CAT))
			چاپ_کن("%s,", log_get_cat_name(rec->cat));
		إذا (fmt & بیت(LOGF_FILE))
			چاپ_کن("%s:", rec->file);
		إذا (fmt & بیت(LOGF_سطر))
			چاپ_کن("%d-", rec->سطر);
		إذا (fmt & بیت(LOGF_FUNC)) {
			إذا (CONFIG_IS_ENABLED(USE_TINY_PRINTF)) {
				چاپ_کن("%s()", rec->func ?: "?");
			} وإلا {
				چاپ_کن("%*s()", CONFIG_LOGF_FUNC_PAD,
				       rec->func ?: "?");
			}
		}
	}
	إذا (fmt & بیت(LOGF_پیام))
		چاپ_کن("%s%s", جمع_کن_space ? " " : "", rec->پیام);

	أرجع 0;
}

LOG_DRIVER(console) = {
	.name	= "console",
	.emit	= log_console_emit,
	.flags	= LOGDF_ENABLE,
};
