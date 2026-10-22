// SPDX-License-Identifier: GPL-2.0+
/*
 * Passing basic information from SPL to U-Boot proper
 *
 * Copyright 2018 Google, Inc
 */

#समावेश <handoff.h>
#समावेश <asm/global_data.h>

DECLARE_GLOBAL_तथ्य_PTR;

व्योम handoff_save_dram(काष्ठा spl_handoff *ho)
{
	पूर्णांक i;

	ho->ram_आकार = gd->ram_आकार;

	क्रम (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		ho->ram_bank[i].start = gd->dram[i].start;
		ho->ram_bank[i].आकार = gd->dram[i].आकार;
	}
}

व्योम handoff_load_dram_आकार(काष्ठा spl_handoff *ho)
{
	gd->ram_आकार = ho->ram_आकार;
}

व्योम handoff_load_dram_banks(काष्ठा spl_handoff *ho)
{
	पूर्णांक i;

	क्रम (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		gd->dram[i].start = ho->ram_bank[i].start;
		gd->dram[i].आकार = ho->ram_bank[i].आकार;
	}
}
