// SPDX-License-Identifier: GPL-2.0+
/*
 * Passing basic information from SPL to U-Boot proper
 *
 * Copyright 2018 Google, Inc
 */

#ଅନ୍ତର୍ଭାଵ <handoff.h>
#ଅନ୍ତର୍ଭାଵ <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

ଶୂନ୍ୟ handoff_save_dram(ସଂରଚନା spl_handoff *ho)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

	ho->ram_size = gd->ram_size;

	ପାଇଁ (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		ho->ram_bank[i].start = gd->dram[i].start;
		ho->ram_bank[i].size = gd->dram[i].size;
	}
}

ଶୂନ୍ୟ handoff_load_dram_size(ସଂରଚନା spl_handoff *ho)
{
	gd->ram_size = ho->ram_size;
}

ଶୂନ୍ୟ handoff_load_dram_banks(ସଂରଚନା spl_handoff *ho)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

	ପାଇଁ (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		gd->dram[i].start = ho->ram_bank[i].start;
		gd->dram[i].size = ho->ram_bank[i].size;
	}
}
