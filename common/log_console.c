// SPDX-License-Identifier: GPL-2.0+
/*
 * Logging support
 *
 * Copyright (c) 2017 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#अन्तर्भाव <log.h>
#अन्तर्भाव <asm/global_data.h>

DECLARE_GLOBAL_दत्त_PTR;

स्थिर पूर्णाङ्क log_console_emit(संरचना log_device *ldev, संरचना log_rec *rec)
{
	पूर्णाङ्क fmt = gd->log_fmt;
	bool सङ्कलय_space = false;

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
	यदि (!(rec->flags & LOGRECF_CONT) && fmt != BIT(LOGF_सन्देशः)) {
		सङ्कलय_space = true;
		यदि (fmt & BIT(LOGF_LEVEL))
			प्रारूप_लिख("%s.", log_get_level_name(rec->level));
		यदि (fmt & BIT(LOGF_CAT))
			प्रारूप_लिख("%s,", log_get_cat_name(rec->cat));
		यदि (fmt & BIT(LOGF_FILE))
			प्रारूप_लिख("%s:", rec->file);
		यदि (fmt & BIT(LOGF_पङ्क्तिः))
			प्रारूप_लिख("%d-", rec->पङ्क्तिः);
		यदि (fmt & BIT(LOGF_FUNC)) {
			यदि (CONFIG_IS_ENABLED(USE_TINY_PRINTF)) {
				प्रारूप_लिख("%s()", rec->func ?: "?");
			} अन्यथा {
				प्रारूप_लिख("%*s()", CONFIG_LOGF_FUNC_PAD,
				       rec->func ?: "?");
			}
		}
	}
	यदि (fmt & BIT(LOGF_सन्देशः))
		प्रारूप_लिख("%s%s", सङ्कलय_space ? " " : "", rec->सन्देशः);

	प्रत्यागम 0;
}

LOG_DRIVER(console) = {
	.name	= "console",
	.emit	= log_console_emit,
	.flags	= LOGDF_ENABLE,
};
