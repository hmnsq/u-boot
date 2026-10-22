// SPDX-License-Identifier: GPL-2.0+
/*
 *  Copyright 2021 Broadcom
 */

#অন্তর্ভাব <asm/global_data.h>

DECLARE_GLOBAL_তথ্য_PTR;

unsigned long __stack_chk_guard = (unsigned long)(0xfeedf00ddeadbeef & ~0UL);

শূন্য __stack_chk_fail(শূন্য)
{
	শূন্য *ra;

	ra = __builtin_extract_return_addr(__builtin_return_address(0));
	panic("Stack smashing detected in function:\n%p relocated from %p",
	      ra, ra - gd->reloc_off);
}

শূন্য __stack_chk_fail_local(শূন্য)
{
	__stack_chk_fail();
}
