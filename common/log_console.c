// SPDX-License-Identifier: GPL-2.0+
/*
 * Logging support
 *
 * Copyright (c) 2017 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#ضمن <log.h>
#ضمن <asm/global_data.h>

DECLARE_GLOBAL_معلومات_PTR;

ساكن صحيح log_console_emit(بنية log_device *ldev, بنية log_rec *rec)
{
	صحيح fmt = gd->log_fmt;
	bool جمع_کړه_space = false;

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
	إذا (!(rec->flags & LOGRECF_CONT) && fmt != بټ(LOGF_پیغام)) {
		جمع_کړه_space = true;
		إذا (fmt & بټ(LOGF_LEVEL))
			چاپ_کړه("%s.", log_get_level_name(rec->level));
		إذا (fmt & بټ(LOGF_CAT))
			چاپ_کړه("%s,", log_get_cat_name(rec->cat));
		إذا (fmt & بټ(LOGF_FILE))
			چاپ_کړه("%s:", rec->file);
		إذا (fmt & بټ(LOGF_کرښه))
			چاپ_کړه("%d-", rec->کرښه);
		إذا (fmt & بټ(LOGF_FUNC)) {
			إذا (CONFIG_IS_ENABLED(USE_TINY_PRINTF)) {
				چاپ_کړه("%s()", rec->func ?: "?");
			} وإلا {
				چاپ_کړه("%*s()", CONFIG_LOGF_FUNC_PAD,
				       rec->func ?: "?");
			}
		}
	}
	إذا (fmt & بټ(LOGF_پیغام))
		چاپ_کړه("%s%s", جمع_کړه_space ? " " : "", rec->پیغام);

	أرجع 0;
}

LOG_DRIVER(console) = {
	.name	= "console",
	.emit	= log_console_emit,
	.flags	= LOGDF_ENABLE,
};
