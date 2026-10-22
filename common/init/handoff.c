// SPDX-License-Identifier: GPL-2.0+
/*
 * Passing basic information from SPL to U-Boot proper
 *
 * Copyright 2018 Google, Inc
 */

#অন্তর্ভাৱ <handoff.h>
#অন্তর্ভাৱ <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

শূন্য handoff_save_dram(সংরচনা spl_handoff *ho)
{
	পূর্ণাঙ্ক i;

	ho->ram_size = gd->ram_size;

	বাবে (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		ho->ram_bank[i].start = gd->dram[i].start;
		ho->ram_bank[i].size = gd->dram[i].size;
	}
}

শূন্য handoff_load_dram_size(সংরচনা spl_handoff *ho)
{
	gd->ram_size = ho->ram_size;
}

শূন্য handoff_load_dram_banks(সংরচনা spl_handoff *ho)
{
	পূর্ণাঙ্ক i;

	বাবে (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		gd->dram[i].start = ho->ram_bank[i].start;
		gd->dram[i].size = ho->ram_bank[i].size;
	}
}
