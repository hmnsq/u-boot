// SPDX-License-Identifier: GPL-2.0+
/*
 * Logging support
 *
 * Copyright (c) 2017 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#अन्तर्भाव <log.h>
#अन्तर्भाव <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

स्थिर पूर्णाङ्क log_console_emit(संरचना log_device *ldev, संरचना log_rec *rec)
{
	पूर्णाङ्क fmt = gd->log_fmt;
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
	सचे (!(rec->flags & LOGRECF_CONT) && fmt != BIT(LOGF_सन्देशः)) {
		add_space = true;
		सचे (fmt & BIT(LOGF_LEVEL))
			printf("%s.", log_get_level_name(rec->level));
		सचे (fmt & BIT(LOGF_CAT))
			printf("%s,", log_get_cat_name(rec->cat));
		सचे (fmt & BIT(LOGF_FILE))
			printf("%s:", rec->file);
		सचे (fmt & BIT(LOGF_पङ्क्तिः))
			printf("%d-", rec->पङ्क्तिः);
		सचे (fmt & BIT(LOGF_FUNC)) {
			सचे (CONFIG_IS_ENABLED(USE_TINY_PRINTF)) {
				printf("%s()", rec->func ?: "?");
			} अञ्ञथा {
				printf("%*s()", CONFIG_LOGF_FUNC_PAD,
				       rec->func ?: "?");
			}
		}
	}
	सचे (fmt & BIT(LOGF_सन्देशः))
		printf("%s%s", add_space ? " " : "", rec->सन्देशः);

	पटिनिवत्त 0;
}

LOG_DRIVER(console) = {
	.name	= "console",
	.emit	= log_console_emit,
	.flags	= LOGDF_ENABLE,
};
