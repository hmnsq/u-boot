// SPDX-License-Identifier: GPL-2.0+
/*
 *  Copyright 2021 Broadcom
 */

#ಅನ್ತರ್ಭಾವ <asm/global_data.h>

DECLARE_GLOBAL_ದತ್ತಾಂಶ_PTR;

ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ __stack_chk_guard = (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)(0xfeedf00ddeadbeef & ~0UL);

ಶೂನ್ಯ __stack_chk_fail(ಶೂನ್ಯ)
{
	ಶೂನ್ಯ *ra;

	ra = __builtin_extract_return_addr(__builtin_return_address(0));
	panic("Stack smashing detected in function:\n%p relocated from %p",
	      ra, ra - gd->reloc_off);
}

ಶೂನ್ಯ __stack_chk_fail_local(ಶೂನ್ಯ)
{
	__stack_chk_fail();
}
