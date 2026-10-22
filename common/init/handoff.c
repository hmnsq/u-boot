// SPDX-License-Identifier: GPL-2.0+
/*
 * Passing basic information from SPL to U-Boot proper
 *
 * Copyright 2018 Google, Inc
 */

#अन्तर्भाव <handoff.h>
#अन्तर्भाव <asm/global_data.h>

DECLARE_GLOBAL_दत्त_PTR;

शून्य handoff_save_dram(संरचना spl_handoff *ho)
{
	पूर्णाङ्क i;

	ho->ram_आकार = gd->ram_आकार;

	प्रति (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		ho->ram_bank[i].start = gd->dram[i].start;
		ho->ram_bank[i].आकार = gd->dram[i].आकार;
	}
}

शून्य handoff_आरोपय_dram_आकार(संरचना spl_handoff *ho)
{
	gd->ram_आकार = ho->ram_आकार;
}

शून्य handoff_आरोपय_dram_banks(संरचना spl_handoff *ho)
{
	पूर्णाङ्क i;

	प्रति (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		gd->dram[i].start = ho->ram_bank[i].start;
		gd->dram[i].आकार = ho->ram_bank[i].आकार;
	}
}
