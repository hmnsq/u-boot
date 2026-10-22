// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2008-2014 Freescale Semiconductor, Inc.
 */

#অন্তর্ভাব <ddr_spd.h>
#অন্তর্ভাব <stdio.h>

/* used for ddr1 and ddr2 spd */
স্থির পূর্ণাঙ্ক
spd_check(ধ্রুব u8 *buf, u8 spd_rev, u8 spd_cksum)
{
	unsigned পূর্ণাঙ্ক cksum = 0;
	unsigned পূর্ণাঙ্ক i;

	/*
	 * Check SPD revision supported
	 * Rev 1.X or less supported by this code
	 */
	যদি (spd_rev >= 0x20) {
		printf("SPD revision %02X not supported by this code\n",
		       spd_rev);
		প্রত্যাগম 1;
	}
	যদি (spd_rev > 0x13) {
		printf("SPD revision %02X not verified by this code\n",
		       spd_rev);
	}

	/*
	 * Calculate checksum
	 */
	প্রতি (i = 0; i < 63; i++) {
		cksum += *buf++;
	}
	cksum &= 0xFF;

	যদি (cksum != spd_cksum) {
		printf("SPD checksum unexpected. "
			"Checksum in SPD = %02X, computed SPD = %02X\n",
			spd_cksum, cksum);
		প্রত্যাগম 1;
	}

	প্রত্যাগম 0;
}

unsigned পূর্ণাঙ্ক
ddr1_spd_check(ধ্রুব ddr1_spd_eeprom_t *spd)
{
	ধ্রুব u8 *p = (ধ্রুব u8 *)spd;

	প্রত্যাগম spd_check(p, spd->spd_rev, spd->cksum);
}

unsigned পূর্ণাঙ্ক
ddr2_spd_check(ধ্রুব ddr2_spd_eeprom_t *spd)
{
	ধ্রুব u8 *p = (ধ্রুব u8 *)spd;

	প্রত্যাগম spd_check(p, spd->spd_rev, spd->cksum);
}

/*
 * CRC16 compute for DDR3 SPD
 * Copied from DDR3 SPD spec.
 */
স্থির পূর্ণাঙ্ক
crc16(বর্ণ *ptr, পূর্ণাঙ্ক গুনতি)
{
	পূর্ণাঙ্ক crc, i;

	crc = 0;
	যাবত্ (--গুনতি >= 0) {
		crc = crc ^ (পূর্ণাঙ্ক)*ptr++ << 8;
		প্রতি (i = 0; i < 8; ++i)
			যদি (crc & 0x8000)
				crc = crc << 1 ^ 0x1021;
			অন্যথা
				crc = crc << 1;
	}
	প্রত্যাগম crc & 0xffff;
}

unsigned পূর্ণাঙ্ক
ddr3_spd_check(ধ্রুব ddr3_spd_eeprom_t *spd)
{
	বর্ণ *p = (বর্ণ *)spd;
	পূর্ণাঙ্ক csum16;
	পূর্ণাঙ্ক len;
	বর্ণ crc_lsb;	/* byte 126 */
	বর্ণ crc_msb;	/* byte 127 */

	/*
	 * SPD byte0[7] - CRC coverage
	 * 0 = CRC covers bytes 0~125
	 * 1 = CRC covers bytes 0~116
	 */

	len = !(spd->info_size_crc & 0x80) ? 126 : 117;
	csum16 = crc16(p, len);

	crc_lsb = (বর্ণ) (csum16 & 0xff);
	crc_msb = (বর্ণ) (csum16 >> 8);

	যদি (spd->crc[0] == crc_lsb && spd->crc[1] == crc_msb) {
		প্রত্যাগম 0;
	} অন্যথা {
		printf("SPD checksum unexpected.\n"
			"Checksum lsb in SPD = %02X, computed SPD = %02X\n"
			"Checksum msb in SPD = %02X, computed SPD = %02X\n",
			spd->crc[0], crc_lsb, spd->crc[1], crc_msb);
		প্রত্যাগম 1;
	}
}

unsigned পূর্ণাঙ্ক ddr4_spd_check(ধ্রুব সংরচনা ddr4_spd_eeprom_s *spd)
{
	বর্ণ *p = (বর্ণ *)spd;
	পূর্ণাঙ্ক csum16;
	পূর্ণাঙ্ক len;
	বর্ণ crc_lsb;	/* byte 126 */
	বর্ণ crc_msb;	/* byte 127 */

	len = 126;
	csum16 = crc16(p, len);

	crc_lsb = (বর্ণ) (csum16 & 0xff);
	crc_msb = (বর্ণ) (csum16 >> 8);

	যদি (spd->crc[0] != crc_lsb || spd->crc[1] != crc_msb) {
		printf("SPD checksum unexpected.\n"
			"Checksum lsb in SPD = %02X, computed SPD = %02X\n"
			"Checksum msb in SPD = %02X, computed SPD = %02X\n",
			spd->crc[0], crc_lsb, spd->crc[1], crc_msb);
		প্রত্যাগম 1;
	}

	p = (বর্ণ *)((ulong)spd + 128);
	len = 126;
	csum16 = crc16(p, len);

	crc_lsb = (বর্ণ) (csum16 & 0xff);
	crc_msb = (বর্ণ) (csum16 >> 8);

	যদি (spd->mod_section.uc[126] != crc_lsb ||
	    spd->mod_section.uc[127] != crc_msb) {
		printf("SPD checksum unexpected.\n"
			"Checksum lsb in SPD = %02X, computed SPD = %02X\n"
			"Checksum msb in SPD = %02X, computed SPD = %02X\n",
			spd->mod_section.uc[126],
			crc_lsb, spd->mod_section.uc[127],
			crc_msb);
		প্রত্যাগম 1;
	}

	প্রত্যাগম 0;
}
