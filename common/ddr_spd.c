// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2008-2014 Freescale Semiconductor, Inc.
 */

#অন্তর্ভাব <ddr_spd.h>
#অন্তর্ভাব <stdio.h>

/* used for ddr1 and ddr2 spd */
স্থির পূর্ণসংখ্যা
spd_check(ধ্রুবক u8 *buf, u8 spd_rev, u8 spd_cksum)
{
	unsigned পূর্ণসংখ্যা cksum = 0;
	unsigned পূর্ণসংখ্যা i;

	/*
	 * Check SPD revision supported
	 * Rev 1.X or less supported by this code
	 */
	যদি (spd_rev >= 0x20) {
		printf("SPD revision %02X not supported by this code\n",
		       spd_rev);
		ফেরত 1;
	}
	যদি (spd_rev > 0x13) {
		printf("SPD revision %02X not verified by this code\n",
		       spd_rev);
	}

	/*
	 * Calculate checksum
	 */
	জন্য (i = 0; i < 63; i++) {
		cksum += *buf++;
	}
	cksum &= 0xFF;

	যদি (cksum != spd_cksum) {
		printf("SPD checksum unexpected. "
			"Checksum in SPD = %02X, computed SPD = %02X\n",
			spd_cksum, cksum);
		ফেরত 1;
	}

	ফেরত 0;
}

unsigned পূর্ণসংখ্যা
ddr1_spd_check(ধ্রুবক ddr1_spd_eeprom_t *spd)
{
	ধ্রুবক u8 *p = (ধ্রুবক u8 *)spd;

	ফেরত spd_check(p, spd->spd_rev, spd->cksum);
}

unsigned পূর্ণসংখ্যা
ddr2_spd_check(ধ্রুবক ddr2_spd_eeprom_t *spd)
{
	ধ্রুবক u8 *p = (ধ্রুবক u8 *)spd;

	ফেরত spd_check(p, spd->spd_rev, spd->cksum);
}

/*
 * CRC16 compute for DDR3 SPD
 * Copied from DDR3 SPD spec.
 */
স্থির পূর্ণসংখ্যা
crc16(অক্ষর *ptr, পূর্ণসংখ্যা গুনতি)
{
	পূর্ণসংখ্যা crc, i;

	crc = 0;
	যখন (--গুনতি >= 0) {
		crc = crc ^ (পূর্ণসংখ্যা)*ptr++ << 8;
		জন্য (i = 0; i < 8; ++i)
			যদি (crc & 0x8000)
				crc = crc << 1 ^ 0x1021;
			অন্যথা
				crc = crc << 1;
	}
	ফেরত crc & 0xffff;
}

unsigned পূর্ণসংখ্যা
ddr3_spd_check(ধ্রুবক ddr3_spd_eeprom_t *spd)
{
	অক্ষর *p = (অক্ষর *)spd;
	পূর্ণসংখ্যা csum16;
	পূর্ণসংখ্যা len;
	অক্ষর crc_lsb;	/* byte 126 */
	অক্ষর crc_msb;	/* byte 127 */

	/*
	 * SPD byte0[7] - CRC coverage
	 * 0 = CRC covers bytes 0~125
	 * 1 = CRC covers bytes 0~116
	 */

	len = !(spd->info_আকার_crc & 0x80) ? 126 : 117;
	csum16 = crc16(p, len);

	crc_lsb = (অক্ষর) (csum16 & 0xff);
	crc_msb = (অক্ষর) (csum16 >> 8);

	যদি (spd->crc[0] == crc_lsb && spd->crc[1] == crc_msb) {
		ফেরত 0;
	} অন্যথা {
		printf("SPD checksum unexpected.\n"
			"Checksum lsb in SPD = %02X, computed SPD = %02X\n"
			"Checksum msb in SPD = %02X, computed SPD = %02X\n",
			spd->crc[0], crc_lsb, spd->crc[1], crc_msb);
		ফেরত 1;
	}
}

unsigned পূর্ণসংখ্যা ddr4_spd_check(ধ্রুবক কাঠামো ddr4_spd_eeprom_s *spd)
{
	অক্ষর *p = (অক্ষর *)spd;
	পূর্ণসংখ্যা csum16;
	পূর্ণসংখ্যা len;
	অক্ষর crc_lsb;	/* byte 126 */
	অক্ষর crc_msb;	/* byte 127 */

	len = 126;
	csum16 = crc16(p, len);

	crc_lsb = (অক্ষর) (csum16 & 0xff);
	crc_msb = (অক্ষর) (csum16 >> 8);

	যদি (spd->crc[0] != crc_lsb || spd->crc[1] != crc_msb) {
		printf("SPD checksum unexpected.\n"
			"Checksum lsb in SPD = %02X, computed SPD = %02X\n"
			"Checksum msb in SPD = %02X, computed SPD = %02X\n",
			spd->crc[0], crc_lsb, spd->crc[1], crc_msb);
		ফেরত 1;
	}

	p = (অক্ষর *)((ulong)spd + 128);
	len = 126;
	csum16 = crc16(p, len);

	crc_lsb = (অক্ষর) (csum16 & 0xff);
	crc_msb = (অক্ষর) (csum16 >> 8);

	যদি (spd->mod_section.uc[126] != crc_lsb ||
	    spd->mod_section.uc[127] != crc_msb) {
		printf("SPD checksum unexpected.\n"
			"Checksum lsb in SPD = %02X, computed SPD = %02X\n"
			"Checksum msb in SPD = %02X, computed SPD = %02X\n",
			spd->mod_section.uc[126],
			crc_lsb, spd->mod_section.uc[127],
			crc_msb);
		ফেরত 1;
	}

	ফেরত 0;
}
