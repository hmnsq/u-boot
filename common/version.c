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

अचर वर्ण version_string[] = U_BOOT_VERSION_STRING;
अचर अचिन्हित लघु version_अङ्क = U_BOOT_VERSION_अङ्क;
अचर अचिन्हित वर्ण version_अङ्क_patch = U_BOOT_VERSION_अङ्क_PATCH;
