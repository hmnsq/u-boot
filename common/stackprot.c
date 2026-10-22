// SPDX-License-Identifier: GPL-2.0+
/*
 *  Copyright 2021 Broadcom
 */

#అన్తర్భావ <asm/global_data.h>

DECLARE_GLOBAL_దత్తాంశం_PTR;

గుర్తులేని దీర్ఘ __stack_chk_guard = (గుర్తులేని దీర్ఘ)(0xfeedf00ddeadbeef & ~0UL);

శూన్య __stack_chk_fail(శూన్య)
{
	శూన్య *ra;

	ra = __builtin_extract_return_addr(__builtin_return_address(0));
	panic("Stack smashing detected in function:\n%p relocated from %p",
	      ra, ra - gd->reloc_off);
}

శూన్య __stack_chk_fail_local(శూన్య)
{
	__stack_chk_fail();
}
