// SPDX-License-Identifier: GPL-2.0+
/*
 * Passing basic information from SPL to U-Boot proper
 *
 * Copyright 2018 Google, Inc
 */

#ಅನ್ತರ್ಭಾವ <handoff.h>
#ಅನ್ತರ್ಭಾವ <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

ಶೂನ್ಯ handoff_save_dram(ಸಂರಚನಾ spl_handoff *ho)
{
	ಪೂರ್ಣಾಙ್ಕ i;

	ho->ram_size = gd->ram_size;

	ಪ್ರತಿ (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		ho->ram_bank[i].start = gd->dram[i].start;
		ho->ram_bank[i].size = gd->dram[i].size;
	}
}

ಶೂನ್ಯ handoff_load_dram_size(ಸಂರಚನಾ spl_handoff *ho)
{
	gd->ram_size = ho->ram_size;
}

ಶೂನ್ಯ handoff_load_dram_banks(ಸಂರಚನಾ spl_handoff *ho)
{
	ಪೂರ್ಣಾಙ್ಕ i;

	ಪ್ರತಿ (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		gd->dram[i].start = ho->ram_bank[i].start;
		gd->dram[i].size = ho->ram_bank[i].size;
	}
}
