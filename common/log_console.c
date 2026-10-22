// SPDX-License-Identifier: GPL-2.0+
/*
 * Logging support
 *
 * Copyright (c) 2017 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#ಅನ್ತರ್ಭಾವ <log.h>
#ಅನ್ತರ್ಭಾವ <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ log_console_emit(ಸಂರಚನಾ log_device *ldev, ಸಂರಚನಾ log_rec *rec)
{
	ಪೂರ್ಣಾಙ್ಕ fmt = gd->log_fmt;
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
	ಯದಿ (!(rec->flags & LOGRECF_CONT) && fmt != BIT(LOGF_ಸಂದೇಶ)) {
		add_space = true;
		ಯದಿ (fmt & BIT(LOGF_LEVEL))
			printf("%s.", log_get_level_name(rec->level));
		ಯದಿ (fmt & BIT(LOGF_CAT))
			printf("%s,", log_get_cat_name(rec->cat));
		ಯದಿ (fmt & BIT(LOGF_FILE))
			printf("%s:", rec->file);
		ಯದಿ (fmt & BIT(LOGF_ಸಾಲು))
			printf("%d-", rec->ಸಾಲು);
		ಯದಿ (fmt & BIT(LOGF_FUNC)) {
			ಯದಿ (CONFIG_IS_ENABLED(USE_TINY_PRINTF)) {
				printf("%s()", rec->func ?: "?");
			} ಅನ್ಯಥಾ {
				printf("%*s()", CONFIG_LOGF_FUNC_PAD,
				       rec->func ?: "?");
			}
		}
	}
	ಯದಿ (fmt & BIT(LOGF_ಸಂದೇಶ))
		printf("%s%s", add_space ? " " : "", rec->ಸಂದೇಶ);

	ಪ್ರತ್ಯಾಗಮ 0;
}

LOG_DRIVER(console) = {
	.name	= "console",
	.emit	= log_console_emit,
	.flags	= LOGDF_ENABLE,
};
