// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2000-2009
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#ಅನ್ತರ್ಭಾವ <timestamp.h>
#ಅನ್ತರ್ಭಾವ <version.h>
#ಅನ್ತರ್ಭಾವ <version_string.h>

#define U_BOOT_VERSION_STRING U_BOOT_VERSION " (" U_BOOT_DATE " - " \
	U_BOOT_TIME " " U_BOOT_TZ ")" CONFIG_IDENT_STRING

ಧ್ರುವ ವರ್ಣ version_string[] = U_BOOT_VERSION_STRING;
ಧ್ರುವ unsigned short version_ಅಂಕೆ = U_BOOT_VERSION_ಅಂಕೆ;
ಧ್ರುವ unsigned ವರ್ಣ version_ಅಂಕೆ_patch = U_BOOT_VERSION_ಅಂಕೆ_PATCH;
