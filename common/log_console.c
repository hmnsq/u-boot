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

स्थिर पूर्णांक log_console_emit(संरचना log_device *ldev, संरचना log_rec *rec)
{
	पूर्णांक fmt = gd->log_fmt;
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
	जर (!(rec->flags & LOGRECF_CONT) && fmt != BIT(LOGF_संदेश)) {
		add_space = true;
		जर (fmt & BIT(LOGF_LEVEL))
			म_लिखो("%s.", log_get_level_name(rec->level));
		जर (fmt & BIT(LOGF_CAT))
			म_लिखो("%s,", log_get_cat_name(rec->cat));
		जर (fmt & BIT(LOGF_FILE))
			म_लिखो("%s:", rec->file);
		जर (fmt & BIT(LOGF_ओळ))
			म_लिखो("%d-", rec->ओळ);
		जर (fmt & BIT(LOGF_FUNC)) {
			जर (CONFIG_IS_ENABLED(USE_TINY_PRINTF)) {
				म_लिखो("%s()", rec->func ?: "?");
			} अन्यथा {
				म_लिखो("%*s()", CONFIG_LOGF_FUNC_PAD,
				       rec->func ?: "?");
			}
		}
	}
	जर (fmt & BIT(LOGF_संदेश))
		म_लिखो("%s%s", add_space ? " " : "", rec->संदेश);

	परत 0;
}

LOG_DRIVER(console) = {
	.name	= "console",
	.emit	= log_console_emit,
	.flags	= LOGDF_ENABLE,
};
