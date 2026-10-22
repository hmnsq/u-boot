// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2008-2014 Freescale Semiconductor, Inc.
 */

#ಅನ್ತರ್ಭಾವ <ddr_spd.h>
#ಅನ್ತರ್ಭಾವ <stdio.h>

/* used for ddr1 and ddr2 spd */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
spd_check(ಸ್ಥಿರಾಂಕ u8 *buf, u8 spd_rev, u8 spd_cksum)
{
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ cksum = 0;
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ i;

	/*
	 * Check SPD revision supported
	 * Rev 1.X or less supported by this code
	 */
	ಆದರೆ (spd_rev >= 0x20) {
		ಬರೆ("SPD revision %02X not supported by this code\n",
		       spd_rev);
		ಹಿಂತಿರುಗಿ 1;
	}
	ಆದರೆ (spd_rev > 0x13) {
		ಬರೆ("SPD revision %02X not verified by this code\n",
		       spd_rev);
	}

	/*
	 * Calculate checksum
	 */
	ಗಾಗಿ (i = 0; i < 63; i++) {
		cksum += *buf++;
	}
	cksum &= 0xFF;

	ಆದರೆ (cksum != spd_cksum) {
		ಬರೆ("SPD checksum unexpected. "
			"Checksum in SPD = %02X, computed SPD = %02X\n",
			spd_cksum, cksum);
		ಹಿಂತಿರುಗಿ 1;
	}

	ಹಿಂತಿರುಗಿ 0;
}

ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ
ddr1_spd_check(ಸ್ಥಿರಾಂಕ ddr1_spd_eeprom_t *spd)
{
	ಸ್ಥಿರಾಂಕ u8 *p = (ಸ್ಥಿರಾಂಕ u8 *)spd;

	ಹಿಂತಿರುಗಿ spd_check(p, spd->spd_rev, spd->cksum);
}

ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ
ddr2_spd_check(ಸ್ಥಿರಾಂಕ ddr2_spd_eeprom_t *spd)
{
	ಸ್ಥಿರಾಂಕ u8 *p = (ಸ್ಥಿರಾಂಕ u8 *)spd;

	ಹಿಂತಿರುಗಿ spd_check(p, spd->spd_rev, spd->cksum);
}

/*
 * CRC16 compute for DDR3 SPD
 * Copied from DDR3 SPD spec.
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
crc16(ಅಕ್ಷರ *ptr, ಪೂರ್ಣಾಂಕ ಎಣಿಕೆ_2)
{
	ಪೂರ್ಣಾಂಕ ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ, i;

	ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ = 0;
	ಆಗ (--ಎಣಿಕೆ_2 >= 0) {
		ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ = ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ ^ (ಪೂರ್ಣಾಂಕ)*ptr++ << 8;
		ಗಾಗಿ (i = 0; i < 8; ++i)
			ಆದರೆ (ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ & 0x8000)
				ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ = ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ << 1 ^ 0x1021;
			ಇಲ್ಲದಿದ್ದರೆ
				ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ = ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ << 1;
	}
	ಹಿಂತಿರುಗಿ ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ & 0xffff;
}

ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ
ddr3_spd_check(ಸ್ಥಿರಾಂಕ ddr3_spd_eeprom_t *spd)
{
	ಅಕ್ಷರ *p = (ಅಕ್ಷರ *)spd;
	ಪೂರ್ಣಾಂಕ csum16;
	ಪೂರ್ಣಾಂಕ len;
	ಅಕ್ಷರ ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_lsb;	/* byte 126 */
	ಅಕ್ಷರ ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_msb;	/* byte 127 */

	/*
	 * SPD byte0[7] - CRC coverage
	 * 0 = CRC covers bytes 0~125
	 * 1 = CRC covers bytes 0~116
	 */

	len = !(spd->info_ಗಾತ್ರ_ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ & 0x80) ? 126 : 117;
	csum16 = crc16(p, len);

	ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_lsb = (ಅಕ್ಷರ) (csum16 & 0xff);
	ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_msb = (ಅಕ್ಷರ) (csum16 >> 8);

	ಆದರೆ (spd->ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ[0] == ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_lsb && spd->ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ[1] == ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_msb) {
		ಹಿಂತಿರುಗಿ 0;
	} ಇಲ್ಲದಿದ್ದರೆ {
		ಬರೆ("SPD checksum unexpected.\n"
			"Checksum lsb in SPD = %02X, computed SPD = %02X\n"
			"Checksum msb in SPD = %02X, computed SPD = %02X\n",
			spd->ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ[0], ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_lsb, spd->ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ[1], ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_msb);
		ಹಿಂತಿರುಗಿ 1;
	}
}

ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ddr4_spd_check(ಸ್ಥಿರಾಂಕ ರಚನೆ ddr4_spd_eeprom_s *spd)
{
	ಅಕ್ಷರ *p = (ಅಕ್ಷರ *)spd;
	ಪೂರ್ಣಾಂಕ csum16;
	ಪೂರ್ಣಾಂಕ len;
	ಅಕ್ಷರ ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_lsb;	/* byte 126 */
	ಅಕ್ಷರ ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_msb;	/* byte 127 */

	len = 126;
	csum16 = crc16(p, len);

	ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_lsb = (ಅಕ್ಷರ) (csum16 & 0xff);
	ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_msb = (ಅಕ್ಷರ) (csum16 >> 8);

	ಆದರೆ (spd->ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ[0] != ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_lsb || spd->ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ[1] != ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_msb) {
		ಬರೆ("SPD checksum unexpected.\n"
			"Checksum lsb in SPD = %02X, computed SPD = %02X\n"
			"Checksum msb in SPD = %02X, computed SPD = %02X\n",
			spd->ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ[0], ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_lsb, spd->ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ[1], ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_msb);
		ಹಿಂತಿರುಗಿ 1;
	}

	p = (ಅಕ್ಷರ *)((ulong)spd + 128);
	len = 126;
	csum16 = crc16(p, len);

	ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_lsb = (ಅಕ್ಷರ) (csum16 & 0xff);
	ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_msb = (ಅಕ್ಷರ) (csum16 >> 8);

	ಆದರೆ (spd->mod_section.uc[126] != ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_lsb ||
	    spd->mod_section.uc[127] != ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_msb) {
		ಬರೆ("SPD checksum unexpected.\n"
			"Checksum lsb in SPD = %02X, computed SPD = %02X\n"
			"Checksum msb in SPD = %02X, computed SPD = %02X\n",
			spd->mod_section.uc[126],
			ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_lsb, spd->mod_section.uc[127],
			ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_msb);
		ಹಿಂತಿರುಗಿ 1;
	}

	ಹಿಂತಿರುಗಿ 0;
}
