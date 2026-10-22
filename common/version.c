// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2000-2009
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#অন্তর্ভাব <timestamp.h>
#অন্তর্ভাব <version.h>
#অন্তর্ভাব <version_string.h>

#define U_BOOT_VERSION_STRING U_BOOT_VERSION " (" U_BOOT_DATE " - " \
	U_BOOT_TIME " " U_BOOT_TZ ")" CONFIG_IDENT_STRING

ধ্রুব বর্ণ version_string[] = U_BOOT_VERSION_STRING;
ধ্রুব unsigned short version_অঙ্ক = U_BOOT_VERSION_অঙ্ক;
ধ্রুব unsigned বর্ণ version_অঙ্ক_patch = U_BOOT_VERSION_অঙ্ক_PATCH;
