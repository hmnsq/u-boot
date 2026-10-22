// SPDX-License-Identifier: GPL-2.0+
/*
 * Passing basic information from SPL to U-Boot proper
 *
 * Copyright 2018 Google, Inc
 */

#అన్తర్భావ <handoff.h>
#అన్తర్భావ <asm/global_data.h>

DECLARE_GLOBAL_దత్తాంశం_PTR;

శూన్య handoff_save_dram(నిర్మాణం spl_handoff *ho)
{
	పూర్ణాంకం i;

	ho->ram_కొలత = gd->ram_కొలత;

	కొరకు (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		ho->ram_bank[i].ప్రారంభం = gd->dram[i].ప్రారంభం;
		ho->ram_bank[i].కొలత = gd->dram[i].కొలత;
	}
}

శూన్య handoff_ఎక్కించు_dram_కొలత(నిర్మాణం spl_handoff *ho)
{
	gd->ram_కొలత = ho->ram_కొలత;
}

శూన్య handoff_ఎక్కించు_dram_banks(నిర్మాణం spl_handoff *ho)
{
	పూర్ణాంకం i;

	కొరకు (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		gd->dram[i].ప్రారంభం = ho->ram_bank[i].ప్రారంభం;
		gd->dram[i].కొలత = ho->ram_bank[i].కొలత;
	}
}
