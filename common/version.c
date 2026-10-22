// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2000-2009
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#ضمن <timestamp.h>
#ضمن <version.h>
#ضمن <version_string.h>

#تعریف_کړه U_BOOT_نسخه_STRING U_BOOT_نسخه " (" U_BOOT_DATE " - " \
	U_BOOT_وخت " " U_BOOT_TZ ")" CONFIG_IDENT_STRING

ثابت حرف نسخه_string[] = U_BOOT_نسخه_STRING;
ثابت بینښه لنډ نسخه_عدد = U_BOOT_نسخه_عدد;
ثابت بینښه حرف نسخه_عدد_patch = U_BOOT_نسخه_عدد_PATCH;
