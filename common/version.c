// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2000-2009
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#شامل <timestamp.h>
#شامل <version.h>
#شامل <version_string.h>

#define U_BOOT_VERSION_STRING U_BOOT_VERSION " (" U_BOOT_DATE " - " \
	U_BOOT_TIME " " U_BOOT_TZ ")" CONFIG_IDENT_STRING

ثابت نویسه version_string[] = U_BOOT_VERSION_STRING;
ثابت unsigned short version_رقم = U_BOOT_VERSION_رقم;
ثابت unsigned نویسه version_رقم_patch = U_BOOT_VERSION_رقم_PATCH;
