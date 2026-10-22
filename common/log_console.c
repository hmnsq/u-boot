// SPDX-License-Identifier: GPL-2.0+
/*
 * Logging support
 *
 * Copyright (c) 2017 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#ಅನ್ತರ್ಭಾವ <log.h>
#ಅನ್ತರ್ಭಾವ <asm/global_data.h>

DECLARE_GLOBAL_ದತ್ತಾಂಶ_PTR;

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ log_console_emit(ರಚನೆ log_device *ldev, ರಚನೆ log_rec *rec)
{
	ಪೂರ್ಣಾಂಕ fmt = gd->log_fmt;
	bool ಕೂಡಿಸು_space = false;

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
	ಆದರೆ (!(rec->flags & LOGRECF_CONT) && fmt != ಬಿಟ್(LOGF_ಸಂದೇಶ)) {
		ಕೂಡಿಸು_space = true;
		ಆದರೆ (fmt & ಬಿಟ್(LOGF_LEVEL))
			ಬರೆ("%s.", log_get_level_name(rec->level));
		ಆದರೆ (fmt & ಬಿಟ್(LOGF_CAT))
			ಬರೆ("%s,", log_get_cat_name(rec->cat));
		ಆದರೆ (fmt & ಬಿಟ್(LOGF_FILE))
			ಬರೆ("%s:", rec->file);
		ಆದರೆ (fmt & ಬಿಟ್(LOGF_ಸಾಲು))
			ಬರೆ("%d-", rec->ಸಾಲು);
		ಆದರೆ (fmt & ಬಿಟ್(LOGF_FUNC)) {
			ಆದರೆ (CONFIG_IS_ENABLED(USE_TINY_PRINTF)) {
				ಬರೆ("%s()", rec->func ?: "?");
			} ಇಲ್ಲದಿದ್ದರೆ {
				ಬರೆ("%*s()", CONFIG_LOGF_FUNC_PAD,
				       rec->func ?: "?");
			}
		}
	}
	ಆದರೆ (fmt & ಬಿಟ್(LOGF_ಸಂದೇಶ))
		ಬರೆ("%s%s", ಕೂಡಿಸು_space ? " " : "", rec->ಸಂದೇಶ);

	ಹಿಂತಿರುಗಿ 0;
}

LOG_DRIVER(console) = {
	.name	= "console",
	.emit	= log_console_emit,
	.flags	= LOGDF_ENABLE,
};
