// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2000-2009
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#અન્તર્ભાવ <timestamp.h>
#અન્તર્ભાવ <version.h>
#અન્તર્ભાવ <version_string.h>

#define U_BOOT_VERSION_STRING U_BOOT_VERSION " (" U_BOOT_DATE " - " \
	U_BOOT_TIME " " U_BOOT_TZ ")" CONFIG_IDENT_STRING

અચલ અક્ષર version_string[] = U_BOOT_VERSION_STRING;
અચલ unsigned short version_અંક = U_BOOT_VERSION_અંક;
અચલ unsigned અક્ષર version_અંક_patch = U_BOOT_VERSION_અંક_PATCH;
