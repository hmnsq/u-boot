// SPDX-License-Identifier: GPL-2.0+
/*
 * Passing basic information from SPL to U-Boot proper
 *
 * Copyright 2018 Google, Inc
 */

#அந்தர்பாவ <handoff.h>
#அந்தர்பாவ <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

வெறுமை handoff_save_dram(கட்டமைப்பு spl_handoff *ho)
{
	முழு i;

	ho->ram_size = gd->ram_size;

	க்கு (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		ho->ram_bank[i].start = gd->dram[i].start;
		ho->ram_bank[i].size = gd->dram[i].size;
	}
}

வெறுமை handoff_load_dram_size(கட்டமைப்பு spl_handoff *ho)
{
	gd->ram_size = ho->ram_size;
}

வெறுமை handoff_load_dram_banks(கட்டமைப்பு spl_handoff *ho)
{
	முழு i;

	க்கு (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		gd->dram[i].start = ho->ram_bank[i].start;
		gd->dram[i].size = ho->ram_bank[i].size;
	}
}
