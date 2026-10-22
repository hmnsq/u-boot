// SPDX-License-Identifier: GPL-2.0+
/*
 * Passing basic information from SPL to U-Boot proper
 *
 * Copyright 2018 Google, Inc
 */

#ضمن <handoff.h>
#ضمن <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

فراغ handoff_save_dram(بنية spl_handoff *ho)
{
	صحيح i;

	ho->ram_size = gd->ram_size;

	لكل (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		ho->ram_bank[i].start = gd->dram[i].start;
		ho->ram_bank[i].size = gd->dram[i].size;
	}
}

فراغ handoff_load_dram_size(بنية spl_handoff *ho)
{
	gd->ram_size = ho->ram_size;
}

فراغ handoff_load_dram_banks(بنية spl_handoff *ho)
{
	صحيح i;

	لكل (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		gd->dram[i].start = ho->ram_bank[i].start;
		gd->dram[i].size = ho->ram_bank[i].size;
	}
}
