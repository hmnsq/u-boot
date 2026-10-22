// SPDX-License-Identifier: GPL-2.0+
/*
 * Logging support
 *
 * Copyright (c) 2017 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#അന്തര്ഭാവ <log.h>
#അന്തര്ഭാവ <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

സ്ഥിതം പൂർണ്ണസംഖ്യ log_console_emit(ഘടന log_device *ldev, ഘടന log_rec *rec)
{
	പൂർണ്ണസംഖ്യ fmt = gd->log_fmt;
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
	എങ്കിൽ (!(rec->flags & LOGRECF_CONT) && fmt != BIT(LOGF_സന്ദേശം)) {
		add_space = true;
		എങ്കിൽ (fmt & BIT(LOGF_LEVEL))
			printf("%s.", log_get_level_name(rec->level));
		എങ്കിൽ (fmt & BIT(LOGF_CAT))
			printf("%s,", log_get_cat_name(rec->cat));
		എങ്കിൽ (fmt & BIT(LOGF_FILE))
			printf("%s:", rec->file);
		എങ്കിൽ (fmt & BIT(LOGF_വരി))
			printf("%d-", rec->വരി);
		എങ്കിൽ (fmt & BIT(LOGF_FUNC)) {
			എങ്കിൽ (CONFIG_IS_ENABLED(USE_TINY_PRINTF)) {
				printf("%s()", rec->func ?: "?");
			} അല്ലെങ്കിൽ {
				printf("%*s()", CONFIG_LOGF_FUNC_PAD,
				       rec->func ?: "?");
			}
		}
	}
	എങ്കിൽ (fmt & BIT(LOGF_സന്ദേശം))
		printf("%s%s", add_space ? " " : "", rec->സന്ദേശം);

	തിരികെ 0;
}

LOG_DRIVER(console) = {
	.name	= "console",
	.emit	= log_console_emit,
	.flags	= LOGDF_ENABLE,
};
