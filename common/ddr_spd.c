// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2008-2014 Freescale Semiconductor, Inc.
 */

#ଅନ୍ତର୍ଭାଵ <ddr_spd.h>
#ଅନ୍ତର୍ଭାଵ <stdio.h>

/* used for ddr1 and ddr2 spd */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
spd_check(ଅଚଳ u8 *buf, u8 spd_rev, u8 spd_cksum)
{
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା cksum = 0;
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

	/*
	 * Check SPD revision supported
	 * Rev 1.X or less supported by this code
	 */
	ଯଦି (spd_rev >= 0x20) {
		printf("SPD revision %02X not supported by this code\n",
		       spd_rev);
		ଫେରନ୍ତୁ 1;
	}
	ଯଦି (spd_rev > 0x13) {
		printf("SPD revision %02X not verified by this code\n",
		       spd_rev);
	}

	/*
	 * Calculate checksum
	 */
	ପାଇଁ (i = 0; i < 63; i++) {
		cksum += *buf++;
	}
	cksum &= 0xFF;

	ଯଦି (cksum != spd_cksum) {
		printf("SPD checksum unexpected. "
			"Checksum in SPD = %02X, computed SPD = %02X\n",
			spd_cksum, cksum);
		ଫେରନ୍ତୁ 1;
	}

	ଫେରନ୍ତୁ 0;
}

unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା
ddr1_spd_check(ଅଚଳ ddr1_spd_eeprom_t *spd)
{
	ଅଚଳ u8 *p = (ଅଚଳ u8 *)spd;

	ଫେରନ୍ତୁ spd_check(p, spd->spd_rev, spd->cksum);
}

unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା
ddr2_spd_check(ଅଚଳ ddr2_spd_eeprom_t *spd)
{
	ଅଚଳ u8 *p = (ଅଚଳ u8 *)spd;

	ଫେରନ୍ତୁ spd_check(p, spd->spd_rev, spd->cksum);
}

/*
 * CRC16 compute for DDR3 SPD
 * Copied from DDR3 SPD spec.
 */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
crc16(ଅକ୍ଷର *ptr, ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଗଣନା_2)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା crc, i;

	crc = 0;
	ଯେତେବେଳେ (--ଗଣନା_2 >= 0) {
		crc = crc ^ (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)*ptr++ << 8;
		ପାଇଁ (i = 0; i < 8; ++i)
			ଯଦି (crc & 0x8000)
				crc = crc << 1 ^ 0x1021;
			ଅନ୍ୟଥା
				crc = crc << 1;
	}
	ଫେରନ୍ତୁ crc & 0xffff;
}

unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା
ddr3_spd_check(ଅଚଳ ddr3_spd_eeprom_t *spd)
{
	ଅକ୍ଷର *p = (ଅକ୍ଷର *)spd;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା csum16;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା len;
	ଅକ୍ଷର crc_lsb;	/* byte 126 */
	ଅକ୍ଷର crc_msb;	/* byte 127 */

	/*
	 * SPD byte0[7] - CRC coverage
	 * 0 = CRC covers bytes 0~125
	 * 1 = CRC covers bytes 0~116
	 */

	len = !(spd->info_size_crc & 0x80) ? 126 : 117;
	csum16 = crc16(p, len);

	crc_lsb = (ଅକ୍ଷର) (csum16 & 0xff);
	crc_msb = (ଅକ୍ଷର) (csum16 >> 8);

	ଯଦି (spd->crc[0] == crc_lsb && spd->crc[1] == crc_msb) {
		ଫେରନ୍ତୁ 0;
	} ଅନ୍ୟଥା {
		printf("SPD checksum unexpected.\n"
			"Checksum lsb in SPD = %02X, computed SPD = %02X\n"
			"Checksum msb in SPD = %02X, computed SPD = %02X\n",
			spd->crc[0], crc_lsb, spd->crc[1], crc_msb);
		ଫେରନ୍ତୁ 1;
	}
}

unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା ddr4_spd_check(ଅଚଳ ସଂରଚନା ddr4_spd_eeprom_s *spd)
{
	ଅକ୍ଷର *p = (ଅକ୍ଷର *)spd;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା csum16;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା len;
	ଅକ୍ଷର crc_lsb;	/* byte 126 */
	ଅକ୍ଷର crc_msb;	/* byte 127 */

	len = 126;
	csum16 = crc16(p, len);

	crc_lsb = (ଅକ୍ଷର) (csum16 & 0xff);
	crc_msb = (ଅକ୍ଷର) (csum16 >> 8);

	ଯଦି (spd->crc[0] != crc_lsb || spd->crc[1] != crc_msb) {
		printf("SPD checksum unexpected.\n"
			"Checksum lsb in SPD = %02X, computed SPD = %02X\n"
			"Checksum msb in SPD = %02X, computed SPD = %02X\n",
			spd->crc[0], crc_lsb, spd->crc[1], crc_msb);
		ଫେରନ୍ତୁ 1;
	}

	p = (ଅକ୍ଷର *)((ulong)spd + 128);
	len = 126;
	csum16 = crc16(p, len);

	crc_lsb = (ଅକ୍ଷର) (csum16 & 0xff);
	crc_msb = (ଅକ୍ଷର) (csum16 >> 8);

	ଯଦି (spd->mod_section.uc[126] != crc_lsb ||
	    spd->mod_section.uc[127] != crc_msb) {
		printf("SPD checksum unexpected.\n"
			"Checksum lsb in SPD = %02X, computed SPD = %02X\n"
			"Checksum msb in SPD = %02X, computed SPD = %02X\n",
			spd->mod_section.uc[126],
			crc_lsb, spd->mod_section.uc[127],
			crc_msb);
		ଫେରନ୍ତୁ 1;
	}

	ଫେରନ୍ତୁ 0;
}
