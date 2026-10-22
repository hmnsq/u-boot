// SPDX-License-Identifier: GPL-2.0+
/*
 * Passing basic information from SPL to U-Boot proper
 *
 * Copyright 2018 Google, Inc
 */

#अन्तर्भाव <handoff.h>
#अन्तर्भाव <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

शून्य handoff_save_dram(संरचना spl_handoff *ho)
{
	पूर्णाङ्क i;

	ho->ram_size = gd->ram_size;

	अत्थाय (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		ho->ram_bank[i].start = gd->dram[i].start;
		ho->ram_bank[i].size = gd->dram[i].size;
	}
}

शून्य handoff_load_dram_size(संरचना spl_handoff *ho)
{
	gd->ram_size = ho->ram_size;
}

शून्य handoff_load_dram_banks(संरचना spl_handoff *ho)
{
	पूर्णाङ्क i;

	अत्थाय (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		gd->dram[i].start = ho->ram_bank[i].start;
		gd->dram[i].size = ho->ram_bank[i].size;
	}
}
