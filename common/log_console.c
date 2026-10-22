// SPDX-License-Identifier: GPL-2.0+
/*
 * Logging support
 *
 * Copyright (c) 2017 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#અન્તર્ભાવ <log.h>
#અન્તર્ભાવ <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

સ્થિર પૂર્ણાંક log_console_emit(સંરચના log_device *ldev, સંરચના log_rec *rec)
{
	પૂર્ણાંક fmt = gd->log_fmt;
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
	જો (!(rec->flags & LOGRECF_CONT) && fmt != BIT(LOGF_સંદેશ)) {
		add_space = true;
		જો (fmt & BIT(LOGF_LEVEL))
			printf("%s.", log_get_level_name(rec->level));
		જો (fmt & BIT(LOGF_CAT))
			printf("%s,", log_get_cat_name(rec->cat));
		જો (fmt & BIT(LOGF_FILE))
			printf("%s:", rec->file);
		જો (fmt & BIT(LOGF_લીટી))
			printf("%d-", rec->લીટી);
		જો (fmt & BIT(LOGF_FUNC)) {
			જો (CONFIG_IS_ENABLED(USE_TINY_PRINTF)) {
				printf("%s()", rec->func ?: "?");
			} નહીંતર {
				printf("%*s()", CONFIG_LOGF_FUNC_PAD,
				       rec->func ?: "?");
			}
		}
	}
	જો (fmt & BIT(LOGF_સંદેશ))
		printf("%s%s", add_space ? " " : "", rec->સંદેશ);

	પાછા 0;
}

LOG_DRIVER(console) = {
	.name	= "console",
	.emit	= log_console_emit,
	.flags	= LOGDF_ENABLE,
};
