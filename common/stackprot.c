// SPDX-License-Identifier: GPL-2.0+
/*
 *  Copyright 2021 Broadcom
 */

#अन्तर्भाव <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

अचिन्हित दीर्घ __stack_chk_guard = (अचिन्हित दीर्घ)(0xfeedf00ddeadbeef & ~0UL);

शून्य __stack_chk_fail(शून्य)
{
	शून्य *ra;

	ra = __builtin_extract_return_addr(__builtin_return_address(0));
	panic("Stack smashing detected in function:\n%p relocated from %p",
	      ra, ra - gd->reloc_off);
}

शून्य __stack_chk_fail_local(शून्य)
{
	__stack_chk_fail();
}
