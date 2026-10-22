// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2000-2009
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#समावेश <timestamp.h>
#समावेश <version.h>
#समावेश <version_string.h>

#घोषणा U_BOOT_VERSION_STRING U_BOOT_VERSION " (" U_BOOT_DATE " - " \
	U_BOOT_TIME " " U_BOOT_TZ ")" CONFIG_IDENT_STRING

स्थिर अक्षर version_string[] = U_BOOT_VERSION_STRING;
स्थिर अचिन्हित लघु version_अंक = U_BOOT_VERSION_अंक;
स्थिर अचिन्हित अक्षर version_अंक_patch = U_BOOT_VERSION_अंक_PATCH;
