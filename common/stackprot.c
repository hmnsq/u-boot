// SPDX-License-Identifier: GPL-2.0+
/*
 *  Copyright 2021 Broadcom
 */

#شامل <asm/global_data.h>

DECLARE_GLOBAL_ڈیٹا_PTR;

غیرعلامتی طویل __stack_chk_guard = (غیرعلامتی طویل)(0xfeedf00ddeadbeef & ~0UL);

خالی __stack_chk_fail(خالی)
{
	خالی *ra;

	ra = __builtin_extract_return_addr(__builtin_return_address(0));
	panic("Stack smashing detected in function:\n%p relocated from %p",
	      ra, ra - gd->reloc_off);
}

خالی __stack_chk_fail_local(خالی)
{
	__stack_chk_fail();
}
