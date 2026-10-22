// SPDX-License-Identifier: GPL-2.0+
/*
 *  Copyright 2021 Broadcom
 */

#ਅਨ੍ਤਰ੍ਭਾਵ <asm/global_data.h>

DECLARE_GLOBAL_ਅੰਕੜੇ_PTR;

ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ __stack_chk_guard = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)(0xfeedf00ddeadbeef & ~0UL);

ਖਾਲੀ __stack_chk_fail(ਖਾਲੀ)
{
	ਖਾਲੀ *ra;

	ra = __builtin_extract_return_addr(__builtin_return_address(0));
	panic("Stack smashing detected in function:\n%p relocated from %p",
	      ra, ra - gd->reloc_off);
}

ਖਾਲੀ __stack_chk_fail_local(ਖਾਲੀ)
{
	__stack_chk_fail();
}
