// SPDX-License-Identifier: GPL-2.0+
/*
 * Passing basic information from SPL to U-Boot proper
 *
 * Copyright 2018 Google, Inc
 */

#ضمن <handoff.h>
#ضمن <asm/global_data.h>

DECLARE_GLOBAL_معلومات_PTR;

فراغ handoff_save_dram(بنية spl_handoff *ho)
{
	صحيح i;

	ho->ram_سائيز = gd->ram_سائيز;

	لكل (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		ho->ram_bank[i].شروعات = gd->dram[i].شروعات;
		ho->ram_bank[i].سائيز = gd->dram[i].سائيز;
	}
}

فراغ handoff_ڀريو_dram_سائيز(بنية spl_handoff *ho)
{
	gd->ram_سائيز = ho->ram_سائيز;
}

فراغ handoff_ڀريو_dram_banks(بنية spl_handoff *ho)
{
	صحيح i;

	لكل (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		gd->dram[i].شروعات = ho->ram_bank[i].شروعات;
		gd->dram[i].سائيز = ho->ram_bank[i].سائيز;
	}
}
