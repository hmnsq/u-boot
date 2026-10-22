// SPDX-License-Identifier: GPL-2.0+
/*
 * Passing basic information from SPL to U-Boot proper
 *
 * Copyright 2018 Google, Inc
 */

#ضمن <handoff.h>
#ضمن <asm/global_data.h>

DECLARE_GLOBAL_انکڑے_PTR;

فراغ handoff_save_dram(بنية spl_handoff *ho)
{
	صحيح i;

	ho->ram_ناپ = gd->ram_ناپ;

	لكل (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		ho->ram_bank[i].شروع = gd->dram[i].شروع;
		ho->ram_bank[i].ناپ = gd->dram[i].ناپ;
	}
}

فراغ handoff_لدو_dram_ناپ(بنية spl_handoff *ho)
{
	gd->ram_ناپ = ho->ram_ناپ;
}

فراغ handoff_لدو_dram_banks(بنية spl_handoff *ho)
{
	صحيح i;

	لكل (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		gd->dram[i].شروع = ho->ram_bank[i].شروع;
		gd->dram[i].ناپ = ho->ram_bank[i].ناپ;
	}
}
