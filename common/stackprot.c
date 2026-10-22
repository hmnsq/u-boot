// SPDX-License-Identifier: GPL-2.0+
/*
 *  Copyright 2021 Broadcom
 */

#ضمن <asm/global_data.h>

DECLARE_GLOBAL_معلومات_PTR;

بنانشان ڊگهو __stack_chk_guard = (بنانشان ڊگهو)(0xfeedf00ddeadbeef & ~0UL);

فراغ __stack_chk_fail(فراغ)
{
	فراغ *ra;

	ra = __builtin_extract_return_addr(__builtin_return_address(0));
	panic("Stack smashing detected in function:\n%p relocated from %p",
	      ra, ra - gd->reloc_off);
}

فراغ __stack_chk_fail_local(فراغ)
{
	__stack_chk_fail();
}
