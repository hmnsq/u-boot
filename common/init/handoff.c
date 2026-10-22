// SPDX-License-Identifier: GPL-2.0+
/*
 * Passing basic information from SPL to U-Boot proper
 *
 * Copyright 2018 Google, Inc
 */

#شامل <handoff.h>
#شامل <asm/global_data.h>

DECLARE_GLOBAL_ڈیٹا_PTR;

خالی handoff_save_dram(ساخت spl_handoff *ho)
{
	صحیح i;

	ho->ram_سائز = gd->ram_سائز;

	برائے (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		ho->ram_bank[i].آغاز = gd->dram[i].آغاز;
		ho->ram_bank[i].سائز = gd->dram[i].سائز;
	}
}

خالی handoff_بھرو_dram_سائز(ساخت spl_handoff *ho)
{
	gd->ram_سائز = ho->ram_سائز;
}

خالی handoff_بھرو_dram_banks(ساخت spl_handoff *ho)
{
	صحیح i;

	برائے (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		gd->dram[i].آغاز = ho->ram_bank[i].آغاز;
		gd->dram[i].سائز = ho->ram_bank[i].سائز;
	}
}
