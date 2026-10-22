// SPDX-License-Identifier: GPL-2.0+
/*
 * Logging support
 *
 * Copyright (c) 2017 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#అన్తర్భావ <log.h>
#అన్తర్భావ <asm/global_data.h>

DECLARE_GLOBAL_దత్తాంశం_PTR;

స్థిర పూర్ణాంకం log_console_emit(నిర్మాణం log_device *ldev, నిర్మాణం log_rec *rec)
{
	పూర్ణాంకం fmt = gd->log_fmt;
	bool కూడు_space = false;

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
	ఒకవేళ (!(rec->flags & LOGRECF_CONT) && fmt != బిట్టు(LOGF_సందేశం)) {
		కూడు_space = true;
		ఒకవేళ (fmt & బిట్టు(LOGF_LEVEL))
			రాయి("%s.", log_get_level_name(rec->level));
		ఒకవేళ (fmt & బిట్టు(LOGF_CAT))
			రాయి("%s,", log_get_cat_name(rec->cat));
		ఒకవేళ (fmt & బిట్టు(LOGF_FILE))
			రాయి("%s:", rec->file);
		ఒకవేళ (fmt & బిట్టు(LOGF_పంక్తి))
			రాయి("%d-", rec->పంక్తి);
		ఒకవేళ (fmt & బిట్టు(LOGF_FUNC)) {
			ఒకవేళ (CONFIG_IS_ENABLED(USE_TINY_PRINTF)) {
				రాయి("%s()", rec->func ?: "?");
			} లేకపోతే {
				రాయి("%*s()", CONFIG_LOGF_FUNC_PAD,
				       rec->func ?: "?");
			}
		}
	}
	ఒకవేళ (fmt & బిట్టు(LOGF_సందేశం))
		రాయి("%s%s", కూడు_space ? " " : "", rec->సందేశం);

	తిరిగి 0;
}

LOG_DRIVER(console) = {
	.name	= "console",
	.emit	= log_console_emit,
	.flags	= LOGDF_ENABLE,
};
