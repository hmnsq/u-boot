// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2000-2009
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#ଅନ୍ତର୍ଭାଵ <timestamp.h>
#ଅନ୍ତର୍ଭାଵ <version.h>
#ଅନ୍ତର୍ଭାଵ <version_string.h>

#define U_BOOT_VERSION_STRING U_BOOT_VERSION " (" U_BOOT_DATE " - " \
	U_BOOT_TIME " " U_BOOT_TZ ")" CONFIG_IDENT_STRING

ଅଚଳ ଅକ୍ଷର version_string[] = U_BOOT_VERSION_STRING;
ଅଚଳ unsigned short version_ଅଙ୍କ = U_BOOT_VERSION_ଅଙ୍କ;
ଅଚଳ unsigned ଅକ୍ଷର version_ଅଙ୍କ_patch = U_BOOT_VERSION_ଅଙ୍କ_PATCH;
