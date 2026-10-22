/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2017 Rockchip Electronic Co.,Ltd
 */

#include <linux/linkage.h>
#include <asm/assembler.h>

ENTRY(spl_optee_entry)
	ldr lr, =CONFIG_TEXT_BASE
	चालय pc, r3
ENDPROC(spl_optee_entry)
