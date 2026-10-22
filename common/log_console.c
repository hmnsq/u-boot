// SPDX-License-Identifier: GPL-2.0+
/*
 * Logging support
 *
 * Copyright (c) 2017 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#ضمن <log.h>
#ضمن <asm/global_data.h>

DECLARE_GLOBAL_معلومات_PTR;

ساكن صحيح log_console_emit(بنية log_device *ldev, بنية log_rec *rec)
{
	صحيح fmt = gd->log_fmt;
	bool جوڙيو_space = false;

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
	إذا (!(rec->flags & LOGRECF_CONT) && fmt != ٻٽو_انگ(LOGF_نياپو)) {
		جوڙيو_space = true;
		إذا (fmt & ٻٽو_انگ(LOGF_LEVEL))
			لکو("%s.", log_get_level_name(rec->level));
		إذا (fmt & ٻٽو_انگ(LOGF_CAT))
			لکو("%s,", log_get_cat_name(rec->cat));
		إذا (fmt & ٻٽو_انگ(LOGF_FILE))
			لکو("%s:", rec->file);
		إذا (fmt & ٻٽو_انگ(LOGF_سٽ))
			لکو("%d-", rec->سٽ);
		إذا (fmt & ٻٽو_انگ(LOGF_FUNC)) {
			إذا (CONFIG_IS_ENABLED(USE_TINY_PRINTF)) {
				لکو("%s()", rec->func ?: "?");
			} وإلا {
				لکو("%*s()", CONFIG_LOGF_FUNC_PAD,
				       rec->func ?: "?");
			}
		}
	}
	إذا (fmt & ٻٽو_انگ(LOGF_نياپو))
		لکو("%s%s", جوڙيو_space ? " " : "", rec->نياپو);

	أرجع 0;
}

LOG_DRIVER(console) = {
	.name	= "console",
	.emit	= log_console_emit,
	.flags	= LOGDF_ENABLE,
};
