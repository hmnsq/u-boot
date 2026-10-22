// SPDX-License-Identifier: GPL-2.0+
/*
 * Passing basic information from SPL to U-Boot proper
 *
 * Copyright 2018 Google, Inc
 */

#অন্তর্ভাব <handoff.h>
#অন্তর্ভাব <asm/global_data.h>

DECLARE_GLOBAL_তথ্য_PTR;

শূন্য handoff_save_dram(কাঠামো spl_handoff *ho)
{
	পূর্ণসংখ্যা i;

	ho->ram_আকার = gd->ram_আকার;

	জন্য (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		ho->ram_bank[i].start = gd->dram[i].start;
		ho->ram_bank[i].আকার_2 = gd->dram[i].আকার_2;
	}
}

শূন্য handoff_load_dram_আকার(কাঠামো spl_handoff *ho)
{
	gd->ram_আকার = ho->ram_আকার;
}

শূন্য handoff_load_dram_banks(কাঠামো spl_handoff *ho)
{
	পূর্ণসংখ্যা i;

	জন্য (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		gd->dram[i].start = ho->ram_bank[i].start;
		gd->dram[i].আকার_2 = ho->ram_bank[i].আকার_2;
	}
}
