// SPDX-License-Identifier: GPL-2.0+
/*
 * Logging support
 *
 * Copyright (c) 2017 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#அந்தர்பாவ <log.h>
#அந்தர்பாவ <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

நிலையான முழு log_console_emit(கட்டமைப்பு log_device *ldev, கட்டமைப்பு log_rec *rec)
{
	முழு fmt = gd->log_fmt;
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
	எனில் (!(rec->flags & LOGRECF_CONT) && fmt != BIT(LOGF_செய்தி)) {
		add_space = true;
		எனில் (fmt & BIT(LOGF_LEVEL))
			printf("%s.", log_get_level_name(rec->level));
		எனில் (fmt & BIT(LOGF_CAT))
			printf("%s,", log_get_cat_name(rec->cat));
		எனில் (fmt & BIT(LOGF_FILE))
			printf("%s:", rec->file);
		எனில் (fmt & BIT(LOGF_வரி))
			printf("%d-", rec->வரி);
		எனில் (fmt & BIT(LOGF_FUNC)) {
			எனில் (CONFIG_IS_ENABLED(USE_TINY_PRINTF)) {
				printf("%s()", rec->func ?: "?");
			} இல்லையெனில் {
				printf("%*s()", CONFIG_LOGF_FUNC_PAD,
				       rec->func ?: "?");
			}
		}
	}
	எனில் (fmt & BIT(LOGF_செய்தி))
		printf("%s%s", add_space ? " " : "", rec->செய்தி);

	திரும்பு 0;
}

LOG_DRIVER(console) = {
	.name	= "console",
	.emit	= log_console_emit,
	.flags	= LOGDF_ENABLE,
};
