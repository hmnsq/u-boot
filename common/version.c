// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2000-2009
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#ضمن <timestamp.h>
#ضمن <version.h>
#ضمن <version_string.h>

#تعریف_کن U_BOOT_نسخه_STRING U_BOOT_نسخه " (" U_BOOT_DATE " - " \
	U_BOOT_زمان " " U_BOOT_TZ ")" CONFIG_IDENT_STRING

ثابت حرف نسخه_string[] = U_BOOT_نسخه_STRING;
ثابت بیعلامت کوتاه نسخه_رقم = U_BOOT_نسخه_رقم;
ثابت بیعلامت حرف نسخه_رقم_patch = U_BOOT_نسخه_رقم_PATCH;
