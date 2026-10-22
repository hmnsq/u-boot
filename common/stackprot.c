// SPDX-License-Identifier: GPL-2.0+
/*
 *  Copyright 2021 Broadcom
 */

#அந்தர்பாவ <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

unsigned long __stack_chk_guard = (unsigned long)(0xfeedf00ddeadbeef & ~0UL);

வெறுமை __stack_chk_fail(வெறுமை)
{
	வெறுமை *ra;

	ra = __builtin_extract_return_addr(__builtin_return_address(0));
	panic("Stack smashing detected in function:\n%p relocated from %p",
	      ra, ra - gd->reloc_off);
}

வெறுமை __stack_chk_fail_local(வெறுமை)
{
	__stack_chk_fail();
}
