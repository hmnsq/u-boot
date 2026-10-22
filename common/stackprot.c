// SPDX-License-Identifier: GPL-2.0+
/*
 *  Copyright 2021 Broadcom
 */

#ଅନ୍ତର୍ଭାଵ <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

unsigned long __stack_chk_guard = (unsigned long)(0xfeedf00ddeadbeef & ~0UL);

ଶୂନ୍ୟ __stack_chk_fail(ଶୂନ୍ୟ)
{
	ଶୂନ୍ୟ *ra;

	ra = __builtin_extract_return_addr(__builtin_return_address(0));
	panic("Stack smashing detected in function:\n%p relocated from %p",
	      ra, ra - gd->reloc_off);
}

ଶୂନ୍ୟ __stack_chk_fail_local(ଶୂନ୍ୟ)
{
	__stack_chk_fail();
}
