// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2000-2009
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#अन्तर्भाव <timestamp.h>
#अन्तर्भाव <version.h>
#अन्तर्भाव <version_string.h>

#define U_BOOT_VERSION_STRING U_BOOT_VERSION " (" U_BOOT_DATE " - " \
	U_BOOT_TIME " " U_BOOT_TZ ")" CONFIG_IDENT_STRING

ध्रुव वर्ण version_string[] = U_BOOT_VERSION_STRING;
ध्रुव unsigned short version_अङ्कः = U_BOOT_VERSION_अङ्कः;
ध्रुव unsigned वर्ण version_अङ्कः_patch = U_BOOT_VERSION_अङ्कः_PATCH;
