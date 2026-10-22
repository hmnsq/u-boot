// SPDX-License-Identifier: GPL-2.0+
/*
 * Logging support
 *
 * Copyright (c) 2017 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#شامل <log.h>
#شامل <asm/global_data.h>

DECLARE_GLOBAL_ڈیٹا_PTR;

ساکن صحیح log_console_emit(ساخت log_device *ldev, ساخت log_rec *rec)
{
	صحیح fmt = gd->log_fmt;
	bool جوڑو_space = false;

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
	اگر (!(rec->flags & LOGRECF_CONT) && fmt != ثنائی_ہندسہ(LOGF_پیغام)) {
		جوڑو_space = true;
		اگر (fmt & ثنائی_ہندسہ(LOGF_LEVEL))
			لکھو("%s.", log_get_level_name(rec->level));
		اگر (fmt & ثنائی_ہندسہ(LOGF_CAT))
			لکھو("%s,", log_get_cat_name(rec->cat));
		اگر (fmt & ثنائی_ہندسہ(LOGF_FILE))
			لکھو("%s:", rec->file);
		اگر (fmt & ثنائی_ہندسہ(LOGF_سطر))
			لکھو("%d-", rec->سطر);
		اگر (fmt & ثنائی_ہندسہ(LOGF_FUNC)) {
			اگر (CONFIG_IS_ENABLED(USE_TINY_PRINTF)) {
				لکھو("%s()", rec->func ?: "?");
			} ورنہ {
				لکھو("%*s()", CONFIG_LOGF_FUNC_PAD,
				       rec->func ?: "?");
			}
		}
	}
	اگر (fmt & ثنائی_ہندسہ(LOGF_پیغام))
		لکھو("%s%s", جوڑو_space ? " " : "", rec->پیغام);

	واپس 0;
}

LOG_DRIVER(console) = {
	.name	= "console",
	.emit	= log_console_emit,
	.flags	= LOGDF_ENABLE,
};
