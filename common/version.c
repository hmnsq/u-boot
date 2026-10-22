// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2000-2009
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#അന്തര്ഭാവ <timestamp.h>
#അന്തര്ഭാവ <version.h>
#അന്തര്ഭാവ <version_string.h>

#define U_BOOT_VERSION_STRING U_BOOT_VERSION " (" U_BOOT_DATE " - " \
	U_BOOT_TIME " " U_BOOT_TZ ")" CONFIG_IDENT_STRING

സ്ഥിരം അക്ഷരം version_string[] = U_BOOT_VERSION_STRING;
സ്ഥിരം unsigned short version_അക്കം = U_BOOT_VERSION_അക്കം;
സ്ഥിരം unsigned അക്ഷരം version_അക്കം_patch = U_BOOT_VERSION_അക്കം_PATCH;
