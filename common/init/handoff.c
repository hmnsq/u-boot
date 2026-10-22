// SPDX-License-Identifier: GPL-2.0+
/*
 * Passing basic information from SPL to U-Boot proper
 *
 * Copyright 2018 Google, Inc
 */

#ضمن <handoff.h>
#ضمن <asm/global_data.h>

DECLARE_GLOBAL_داده_PTR;

فراغ handoff_save_dram(بنية spl_handoff *ho)
{
	صحيح i;

	ho->ram_اندازه = gd->ram_اندازه;

	لكل (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		ho->ram_bank[i].شروع = gd->dram[i].شروع;
		ho->ram_bank[i].اندازه = gd->dram[i].اندازه;
	}
}

فراغ handoff_بارگیری_dram_اندازه(بنية spl_handoff *ho)
{
	gd->ram_اندازه = ho->ram_اندازه;
}

فراغ handoff_بارگیری_dram_banks(بنية spl_handoff *ho)
{
	صحيح i;

	لكل (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		gd->dram[i].شروع = ho->ram_bank[i].شروع;
		gd->dram[i].اندازه = ho->ram_bank[i].اندازه;
	}
}
