// SPDX-License-Identifier: GPL-2.0+
/*
 * Passing basic information from SPL to U-Boot proper
 *
 * Copyright 2018 Google, Inc
 */

#ಅನ್ತರ್ಭಾವ <handoff.h>
#ಅನ್ತರ್ಭಾವ <asm/global_data.h>

DECLARE_GLOBAL_ದತ್ತಾಂಶ_PTR;

ಶೂನ್ಯ handoff_save_dram(ರಚನೆ spl_handoff *ho)
{
	ಪೂರ್ಣಾಂಕ i;

	ho->ram_ಗಾತ್ರ = gd->ram_ಗಾತ್ರ;

	ಗಾಗಿ (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		ho->ram_bank[i].ಆರಂಭ = gd->dram[i].ಆರಂಭ;
		ho->ram_bank[i].ಗಾತ್ರ_2 = gd->dram[i].ಗಾತ್ರ_2;
	}
}

ಶೂನ್ಯ handoff_ತುಂಬು_dram_ಗಾತ್ರ(ರಚನೆ spl_handoff *ho)
{
	gd->ram_ಗಾತ್ರ = ho->ram_ಗಾತ್ರ;
}

ಶೂನ್ಯ handoff_ತುಂಬು_dram_banks(ರಚನೆ spl_handoff *ho)
{
	ಪೂರ್ಣಾಂಕ i;

	ಗಾಗಿ (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		gd->dram[i].ಆರಂಭ = ho->ram_bank[i].ಆರಂಭ;
		gd->dram[i].ಗಾತ್ರ_2 = ho->ram_bank[i].ಗಾತ್ರ_2;
	}
}
