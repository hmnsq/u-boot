// SPDX-License-Identifier: GPL-2.0+
/*
 * Passing basic information from SPL to U-Boot proper
 *
 * Copyright 2018 Google, Inc
 */

#અન્તર્ભાવ <handoff.h>
#અન્તર્ભાવ <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

શૂન્ય handoff_save_dram(સંરચના spl_handoff *ho)
{
	પૂર્ણાંક i;

	ho->ram_size = gd->ram_size;

	માટે (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		ho->ram_bank[i].start = gd->dram[i].start;
		ho->ram_bank[i].size = gd->dram[i].size;
	}
}

શૂન્ય handoff_load_dram_size(સંરચના spl_handoff *ho)
{
	gd->ram_size = ho->ram_size;
}

શૂન્ય handoff_load_dram_banks(સંરચના spl_handoff *ho)
{
	પૂર્ણાંક i;

	માટે (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		gd->dram[i].start = ho->ram_bank[i].start;
		gd->dram[i].size = ho->ram_bank[i].size;
	}
}
