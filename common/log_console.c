// SPDX-License-Identifier: GPL-2.0+
/*
 * Logging support
 *
 * Copyright (c) 2017 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#ଅନ୍ତର୍ଭାଵ <log.h>
#ଅନ୍ତର୍ଭାଵ <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା log_console_emit(ସଂରଚନା log_device *ldev, ସଂରଚନା log_rec *rec)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା fmt = gd->log_fmt;
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
	ଯଦି (!(rec->flags & LOGRECF_CONT) && fmt != BIT(LOGF_ସନ୍ଦେଶ)) {
		add_space = true;
		ଯଦି (fmt & BIT(LOGF_LEVEL))
			printf("%s.", log_get_level_name(rec->level));
		ଯଦି (fmt & BIT(LOGF_CAT))
			printf("%s,", log_get_cat_name(rec->cat));
		ଯଦି (fmt & BIT(LOGF_FILE))
			printf("%s:", rec->file);
		ଯଦି (fmt & BIT(LOGF_ଧାଡ଼ି))
			printf("%d-", rec->ଧାଡ଼ି);
		ଯଦି (fmt & BIT(LOGF_FUNC)) {
			ଯଦି (CONFIG_IS_ENABLED(USE_TINY_PRINTF)) {
				printf("%s()", rec->func ?: "?");
			} ଅନ୍ୟଥା {
				printf("%*s()", CONFIG_LOGF_FUNC_PAD,
				       rec->func ?: "?");
			}
		}
	}
	ଯଦି (fmt & BIT(LOGF_ସନ୍ଦେଶ))
		printf("%s%s", add_space ? " " : "", rec->ସନ୍ଦେଶ);

	ଫେରନ୍ତୁ 0;
}

LOG_DRIVER(console) = {
	.name	= "console",
	.emit	= log_console_emit,
	.flags	= LOGDF_ENABLE,
};
