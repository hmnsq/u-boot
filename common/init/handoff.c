// SPDX-License-Identifier: GPL-2.0+
/*
 * Passing basic information from SPL to U-Boot proper
 *
 * Copyright 2018 Google, Inc
 */

#ਅਨ੍ਤਰ੍ਭਾਵ <handoff.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <asm/global_data.h>

DECLARE_GLOBAL_ਅੰਕੜੇ_PTR;

ਖਾਲੀ handoff_save_dram(ਬਣਤਰ spl_handoff *ho)
{
	ਪੂਰਨ i;

	ho->ram_ਮਾਪ = gd->ram_ਮਾਪ;

	ਲਈ (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		ho->ram_bank[i].ਸ਼ੁਰੂ = gd->dram[i].ਸ਼ੁਰੂ;
		ho->ram_bank[i].ਮਾਪ = gd->dram[i].ਮਾਪ;
	}
}

ਖਾਲੀ handoff_ਲੱਦੋ_dram_ਮਾਪ(ਬਣਤਰ spl_handoff *ho)
{
	gd->ram_ਮਾਪ = ho->ram_ਮਾਪ;
}

ਖਾਲੀ handoff_ਲੱਦੋ_dram_banks(ਬਣਤਰ spl_handoff *ho)
{
	ਪੂਰਨ i;

	ਲਈ (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		gd->dram[i].ਸ਼ੁਰੂ = ho->ram_bank[i].ਸ਼ੁਰੂ;
		gd->dram[i].ਮਾਪ = ho->ram_bank[i].ਮਾਪ;
	}
}
