// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2000-2009
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#অন্তর্ভাৱ <timestamp.h>
#অন্তর্ভাৱ <version.h>
#অন্তর্ভাৱ <version_string.h>

#define U_BOOT_VERSION_STRING U_BOOT_VERSION " (" U_BOOT_DATE " - " \
	U_BOOT_TIME " " U_BOOT_TZ ")" CONFIG_IDENT_STRING

ধ্রুৱ ৱর্ণ version_string[] = U_BOOT_VERSION_STRING;
ধ্রুৱ unsigned short version_অঙ্ক = U_BOOT_VERSION_অঙ্ক;
ধ্রুৱ unsigned ৱর্ণ version_অঙ্ক_patch = U_BOOT_VERSION_অঙ্ক_PATCH;
