// SPDX-License-Identifier: GPL-2.0+
/*
 * Passing basic information from SPL to U-Boot proper
 *
 * Copyright 2018 Google, Inc
 */

#شامل <handoff.h>
#شامل <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

تهی handoff_save_dram(ساختار spl_handoff *ho)
{
	صحیح i;

	ho->ram_size = gd->ram_size;

	برای (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		ho->ram_bank[i].start = gd->dram[i].start;
		ho->ram_bank[i].size = gd->dram[i].size;
	}
}

تهی handoff_load_dram_size(ساختار spl_handoff *ho)
{
	gd->ram_size = ho->ram_size;
}

تهی handoff_load_dram_banks(ساختار spl_handoff *ho)
{
	صحیح i;

	برای (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		gd->dram[i].start = ho->ram_bank[i].start;
		gd->dram[i].size = ho->ram_bank[i].size;
	}
}
