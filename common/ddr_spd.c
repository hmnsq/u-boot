// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2008-2014 Freescale Semiconductor, Inc.
 */

#ಅನ್ತರ್ಭಾವ <ddr_spd.h>
#ಅನ್ತರ್ಭಾವ <stdio.h>

/* used for ddr1 and ddr2 spd */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
spd_check(ಧ್ರುವ u8 *buf, u8 spd_rev, u8 spd_cksum)
{
	unsigned ಪೂರ್ಣಾಙ್ಕ cksum = 0;
	unsigned ಪೂರ್ಣಾಙ್ಕ i;

	/*
	 * Check SPD revision supported
	 * Rev 1.X or less supported by this code
	 */
	ಯದಿ (spd_rev >= 0x20) {
		printf("SPD revision %02X not supported by this code\n",
		       spd_rev);
		ಪ್ರತ್ಯಾಗಮ 1;
	}
	ಯದಿ (spd_rev > 0x13) {
		printf("SPD revision %02X not verified by this code\n",
		       spd_rev);
	}

	/*
	 * Calculate checksum
	 */
	ಪ್ರತಿ (i = 0; i < 63; i++) {
		cksum += *buf++;
	}
	cksum &= 0xFF;

	ಯದಿ (cksum != spd_cksum) {
		printf("SPD checksum unexpected. "
			"Checksum in SPD = %02X, computed SPD = %02X\n",
			spd_cksum, cksum);
		ಪ್ರತ್ಯಾಗಮ 1;
	}

	ಪ್ರತ್ಯಾಗಮ 0;
}

unsigned ಪೂರ್ಣಾಙ್ಕ
ddr1_spd_check(ಧ್ರುವ ddr1_spd_eeprom_t *spd)
{
	ಧ್ರುವ u8 *p = (ಧ್ರುವ u8 *)spd;

	ಪ್ರತ್ಯಾಗಮ spd_check(p, spd->spd_rev, spd->cksum);
}

unsigned ಪೂರ್ಣಾಙ್ಕ
ddr2_spd_check(ಧ್ರುವ ddr2_spd_eeprom_t *spd)
{
	ಧ್ರುವ u8 *p = (ಧ್ರುವ u8 *)spd;

	ಪ್ರತ್ಯಾಗಮ spd_check(p, spd->spd_rev, spd->cksum);
}

/*
 * CRC16 compute for DDR3 SPD
 * Copied from DDR3 SPD spec.
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
crc16(ವರ್ಣ *ptr, ಪೂರ್ಣಾಙ್ಕ ಎಣಿಕೆ)
{
	ಪೂರ್ಣಾಙ್ಕ crc, i;

	crc = 0;
	ಯಾವತ್ (--ಎಣಿಕೆ >= 0) {
		crc = crc ^ (ಪೂರ್ಣಾಙ್ಕ)*ptr++ << 8;
		ಪ್ರತಿ (i = 0; i < 8; ++i)
			ಯದಿ (crc & 0x8000)
				crc = crc << 1 ^ 0x1021;
			ಅನ್ಯಥಾ
				crc = crc << 1;
	}
	ಪ್ರತ್ಯಾಗಮ crc & 0xffff;
}

unsigned ಪೂರ್ಣಾಙ್ಕ
ddr3_spd_check(ಧ್ರುವ ddr3_spd_eeprom_t *spd)
{
	ವರ್ಣ *p = (ವರ್ಣ *)spd;
	ಪೂರ್ಣಾಙ್ಕ csum16;
	ಪೂರ್ಣಾಙ್ಕ len;
	ವರ್ಣ crc_lsb;	/* byte 126 */
	ವರ್ಣ crc_msb;	/* byte 127 */

	/*
	 * SPD byte0[7] - CRC coverage
	 * 0 = CRC covers bytes 0~125
	 * 1 = CRC covers bytes 0~116
	 */

	len = !(spd->info_size_crc & 0x80) ? 126 : 117;
	csum16 = crc16(p, len);

	crc_lsb = (ವರ್ಣ) (csum16 & 0xff);
	crc_msb = (ವರ್ಣ) (csum16 >> 8);

	ಯದಿ (spd->crc[0] == crc_lsb && spd->crc[1] == crc_msb) {
		ಪ್ರತ್ಯಾಗಮ 0;
	} ಅನ್ಯಥಾ {
		printf("SPD checksum unexpected.\n"
			"Checksum lsb in SPD = %02X, computed SPD = %02X\n"
			"Checksum msb in SPD = %02X, computed SPD = %02X\n",
			spd->crc[0], crc_lsb, spd->crc[1], crc_msb);
		ಪ್ರತ್ಯಾಗಮ 1;
	}
}

unsigned ಪೂರ್ಣಾಙ್ಕ ddr4_spd_check(ಧ್ರುವ ಸಂರಚನಾ ddr4_spd_eeprom_s *spd)
{
	ವರ್ಣ *p = (ವರ್ಣ *)spd;
	ಪೂರ್ಣಾಙ್ಕ csum16;
	ಪೂರ್ಣಾಙ್ಕ len;
	ವರ್ಣ crc_lsb;	/* byte 126 */
	ವರ್ಣ crc_msb;	/* byte 127 */

	len = 126;
	csum16 = crc16(p, len);

	crc_lsb = (ವರ್ಣ) (csum16 & 0xff);
	crc_msb = (ವರ್ಣ) (csum16 >> 8);

	ಯದಿ (spd->crc[0] != crc_lsb || spd->crc[1] != crc_msb) {
		printf("SPD checksum unexpected.\n"
			"Checksum lsb in SPD = %02X, computed SPD = %02X\n"
			"Checksum msb in SPD = %02X, computed SPD = %02X\n",
			spd->crc[0], crc_lsb, spd->crc[1], crc_msb);
		ಪ್ರತ್ಯಾಗಮ 1;
	}

	p = (ವರ್ಣ *)((ulong)spd + 128);
	len = 126;
	csum16 = crc16(p, len);

	crc_lsb = (ವರ್ಣ) (csum16 & 0xff);
	crc_msb = (ವರ್ಣ) (csum16 >> 8);

	ಯದಿ (spd->mod_section.uc[126] != crc_lsb ||
	    spd->mod_section.uc[127] != crc_msb) {
		printf("SPD checksum unexpected.\n"
			"Checksum lsb in SPD = %02X, computed SPD = %02X\n"
			"Checksum msb in SPD = %02X, computed SPD = %02X\n",
			spd->mod_section.uc[126],
			crc_lsb, spd->mod_section.uc[127],
			crc_msb);
		ಪ್ರತ್ಯಾಗಮ 1;
	}

	ಪ್ರತ್ಯಾಗಮ 0;
}
