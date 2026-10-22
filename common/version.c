// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2000-2009
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#ضمن <timestamp.h>
#ضمن <version.h>
#ضمن <version_string.h>

#define U_BOOT_VERSION_STRING U_BOOT_VERSION " (" U_BOOT_DATE " - " \
	U_BOOT_TIME " " U_BOOT_TZ ")" CONFIG_IDENT_STRING

ثابت حرف version_string[] = U_BOOT_VERSION_STRING;
ثابت unsigned short version_اڠک = U_BOOT_VERSION_اڠک;
ثابت unsigned حرف version_اڠک_patch = U_BOOT_VERSION_اڠک_PATCH;
