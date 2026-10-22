// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2000-2009
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#அந்தர்பாவ <timestamp.h>
#அந்தர்பாவ <version.h>
#அந்தர்பாவ <version_string.h>

#define U_BOOT_VERSION_STRING U_BOOT_VERSION " (" U_BOOT_DATE " - " \
	U_BOOT_TIME " " U_BOOT_TZ ")" CONFIG_IDENT_STRING

மாறிலி எழுத்து version_string[] = U_BOOT_VERSION_STRING;
மாறிலி unsigned short version_இலக்கம் = U_BOOT_VERSION_இலக்கம்;
மாறிலி unsigned எழுத்து version_இலக்கம்_patch = U_BOOT_VERSION_இலக்கம்_PATCH;
