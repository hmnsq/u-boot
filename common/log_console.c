// SPDX-License-Identifier: GPL-2.0+
/*
 * Logging support
 *
 * Copyright (c) 2017 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#ਅਨ੍ਤਰ੍ਭਾਵ <log.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <asm/global_data.h>

DECLARE_GLOBAL_ਅੰਕੜੇ_PTR;

ਸਥਿਰ ਪੂਰਨ log_console_emit(ਬਣਤਰ log_device *ldev, ਬਣਤਰ log_rec *rec)
{
	ਪੂਰਨ fmt = gd->log_fmt;
	bool ਜੋੜੋ_space = false;

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
	ਜੇ (!(rec->flags & LOGRECF_CONT) && fmt != ਦੋਆਧਾਰੀ_ਅੰਕ(LOGF_ਸੁਨੇਹਾ)) {
		ਜੋੜੋ_space = true;
		ਜੇ (fmt & ਦੋਆਧਾਰੀ_ਅੰਕ(LOGF_LEVEL))
			ਲਿਖੋ("%s.", log_get_level_name(rec->level));
		ਜੇ (fmt & ਦੋਆਧਾਰੀ_ਅੰਕ(LOGF_CAT))
			ਲਿਖੋ("%s,", log_get_cat_name(rec->cat));
		ਜੇ (fmt & ਦੋਆਧਾਰੀ_ਅੰਕ(LOGF_FILE))
			ਲਿਖੋ("%s:", rec->file);
		ਜੇ (fmt & ਦੋਆਧਾਰੀ_ਅੰਕ(LOGF_ਸਤਰ))
			ਲਿਖੋ("%d-", rec->ਸਤਰ);
		ਜੇ (fmt & ਦੋਆਧਾਰੀ_ਅੰਕ(LOGF_FUNC)) {
			ਜੇ (CONFIG_IS_ENABLED(USE_TINY_PRINTF)) {
				ਲਿਖੋ("%s()", rec->func ?: "?");
			} ਨਹੀਂਤਾਂ {
				ਲਿਖੋ("%*s()", CONFIG_LOGF_FUNC_PAD,
				       rec->func ?: "?");
			}
		}
	}
	ਜੇ (fmt & ਦੋਆਧਾਰੀ_ਅੰਕ(LOGF_ਸੁਨੇਹਾ))
		ਲਿਖੋ("%s%s", ਜੋੜੋ_space ? " " : "", rec->ਸੁਨੇਹਾ);

	ਵਾਪਸ 0;
}

LOG_DRIVER(console) = {
	.name	= "console",
	.emit	= log_console_emit,
	.flags	= LOGDF_ENABLE,
};
