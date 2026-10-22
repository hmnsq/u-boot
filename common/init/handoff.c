// SPDX-License-Identifier: GPL-2.0+
/*
 * Passing basic information from SPL to U-Boot proper
 *
 * Copyright 2018 Google, Inc
 */

#അന്തര്ഭാവ <handoff.h>
#അന്തര്ഭാവ <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

ശൂന്യം handoff_save_dram(ഘടന spl_handoff *ho)
{
	പൂർണ്ണസംഖ്യ i;

	ho->ram_size = gd->ram_size;

	വേണ്ടി (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		ho->ram_bank[i].start = gd->dram[i].start;
		ho->ram_bank[i].size = gd->dram[i].size;
	}
}

ശൂന്യം handoff_load_dram_size(ഘടന spl_handoff *ho)
{
	gd->ram_size = ho->ram_size;
}

ശൂന്യം handoff_load_dram_banks(ഘടന spl_handoff *ho)
{
	പൂർണ്ണസംഖ്യ i;

	വേണ്ടി (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		gd->dram[i].start = ho->ram_bank[i].start;
		gd->dram[i].size = ho->ram_bank[i].size;
	}
}
