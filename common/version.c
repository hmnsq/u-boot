// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2000-2009
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#شامل <timestamp.h>
#شامل <version.h>
#شامل <version_string.h>

#تعریف_کرو U_BOOT_نسخہ_STRING U_BOOT_نسخہ " (" U_BOOT_DATE " - " \
	U_BOOT_وقت " " U_BOOT_TZ ")" CONFIG_IDENT_STRING

ثابت حرف نسخہ_string[] = U_BOOT_نسخہ_STRING;
ثابت غیرعلامتی مختصر نسخہ_عدد = U_BOOT_نسخہ_عدد;
ثابت غیرعلامتی حرف نسخہ_عدد_patch = U_BOOT_نسخہ_عدد_PATCH;
