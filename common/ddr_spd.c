// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2008-2014 Freescale Semiconductor, Inc.
 */

#અન્તર્ભાવ <ddr_spd.h>
#અન્તર્ભાવ <stdio.h>

/* used for ddr1 and ddr2 spd */
સ્થિર પૂર્ણાંક
spd_check(અચલ u8 *buf, u8 spd_rev, u8 spd_cksum)
{
	unsigned પૂર્ણાંક cksum = 0;
	unsigned પૂર્ણાંક i;

	/*
	 * Check SPD revision supported
	 * Rev 1.X or less supported by this code
	 */
	જો (spd_rev >= 0x20) {
		printf("SPD revision %02X not supported by this code\n",
		       spd_rev);
		પાછા 1;
	}
	જો (spd_rev > 0x13) {
		printf("SPD revision %02X not verified by this code\n",
		       spd_rev);
	}

	/*
	 * Calculate checksum
	 */
	માટે (i = 0; i < 63; i++) {
		cksum += *buf++;
	}
	cksum &= 0xFF;

	જો (cksum != spd_cksum) {
		printf("SPD checksum unexpected. "
			"Checksum in SPD = %02X, computed SPD = %02X\n",
			spd_cksum, cksum);
		પાછા 1;
	}

	પાછા 0;
}

unsigned પૂર્ણાંક
ddr1_spd_check(અચલ ddr1_spd_eeprom_t *spd)
{
	અચલ u8 *p = (અચલ u8 *)spd;

	પાછા spd_check(p, spd->spd_rev, spd->cksum);
}

unsigned પૂર્ણાંક
ddr2_spd_check(અચલ ddr2_spd_eeprom_t *spd)
{
	અચલ u8 *p = (અચલ u8 *)spd;

	પાછા spd_check(p, spd->spd_rev, spd->cksum);
}

/*
 * CRC16 compute for DDR3 SPD
 * Copied from DDR3 SPD spec.
 */
સ્થિર પૂર્ણાંક
crc16(અક્ષર *ptr, પૂર્ણાંક ગણતરી)
{
	પૂર્ણાંક crc, i;

	crc = 0;
	જ્યારે (--ગણતરી >= 0) {
		crc = crc ^ (પૂર્ણાંક)*ptr++ << 8;
		માટે (i = 0; i < 8; ++i)
			જો (crc & 0x8000)
				crc = crc << 1 ^ 0x1021;
			નહીંતર
				crc = crc << 1;
	}
	પાછા crc & 0xffff;
}

unsigned પૂર્ણાંક
ddr3_spd_check(અચલ ddr3_spd_eeprom_t *spd)
{
	અક્ષર *p = (અક્ષર *)spd;
	પૂર્ણાંક csum16;
	પૂર્ણાંક len;
	અક્ષર crc_lsb;	/* byte 126 */
	અક્ષર crc_msb;	/* byte 127 */

	/*
	 * SPD byte0[7] - CRC coverage
	 * 0 = CRC covers bytes 0~125
	 * 1 = CRC covers bytes 0~116
	 */

	len = !(spd->info_size_crc & 0x80) ? 126 : 117;
	csum16 = crc16(p, len);

	crc_lsb = (અક્ષર) (csum16 & 0xff);
	crc_msb = (અક્ષર) (csum16 >> 8);

	જો (spd->crc[0] == crc_lsb && spd->crc[1] == crc_msb) {
		પાછા 0;
	} નહીંતર {
		printf("SPD checksum unexpected.\n"
			"Checksum lsb in SPD = %02X, computed SPD = %02X\n"
			"Checksum msb in SPD = %02X, computed SPD = %02X\n",
			spd->crc[0], crc_lsb, spd->crc[1], crc_msb);
		પાછા 1;
	}
}

unsigned પૂર્ણાંક ddr4_spd_check(અચલ સંરચના ddr4_spd_eeprom_s *spd)
{
	અક્ષર *p = (અક્ષર *)spd;
	પૂર્ણાંક csum16;
	પૂર્ણાંક len;
	અક્ષર crc_lsb;	/* byte 126 */
	અક્ષર crc_msb;	/* byte 127 */

	len = 126;
	csum16 = crc16(p, len);

	crc_lsb = (અક્ષર) (csum16 & 0xff);
	crc_msb = (અક્ષર) (csum16 >> 8);

	જો (spd->crc[0] != crc_lsb || spd->crc[1] != crc_msb) {
		printf("SPD checksum unexpected.\n"
			"Checksum lsb in SPD = %02X, computed SPD = %02X\n"
			"Checksum msb in SPD = %02X, computed SPD = %02X\n",
			spd->crc[0], crc_lsb, spd->crc[1], crc_msb);
		પાછા 1;
	}

	p = (અક્ષર *)((ulong)spd + 128);
	len = 126;
	csum16 = crc16(p, len);

	crc_lsb = (અક્ષર) (csum16 & 0xff);
	crc_msb = (અક્ષર) (csum16 >> 8);

	જો (spd->mod_section.uc[126] != crc_lsb ||
	    spd->mod_section.uc[127] != crc_msb) {
		printf("SPD checksum unexpected.\n"
			"Checksum lsb in SPD = %02X, computed SPD = %02X\n"
			"Checksum msb in SPD = %02X, computed SPD = %02X\n",
			spd->mod_section.uc[126],
			crc_lsb, spd->mod_section.uc[127],
			crc_msb);
		પાછા 1;
	}

	પાછા 0;
}
