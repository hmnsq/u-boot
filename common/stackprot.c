// SPDX-License-Identifier: GPL-2.0+
/*
 *  Copyright 2021 Broadcom
 */

#समावेश <asm/global_data.h>

DECLARE_GLOBAL_तथ्य_PTR;

अचिन्हित दीर्घ __stack_chk_guard = (अचिन्हित दीर्घ)(0xfeedf00ddeadbeef & ~0UL);

व्योम __stack_chk_fail(व्योम)
{
	व्योम *ra;

	ra = __builtin_extract_return_addr(__builtin_return_address(0));
	panic("Stack smashing detected in function:\n%p relocated from %p",
	      ra, ra - gd->reloc_off);
}

व्योम __stack_chk_fail_local(व्योम)
{
	__stack_chk_fail();
}
