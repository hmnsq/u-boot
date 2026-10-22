// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2008-2014 Freescale Semiconductor, Inc.
 */

#شامل <ddr_spd.h>
#شامل <stdio.h>

/* used for ddr1 and ddr2 spd */
ایستا صحیح
spd_check(ثابت u8 *buf, u8 spd_rev, u8 spd_cksum)
{
	unsigned صحیح cksum = 0;
	unsigned صحیح i;

	/*
	 * Check SPD revision supported
	 * Rev 1.X or less supported by this code
	 */
	اگر (spd_rev >= 0x20) {
		printf("SPD revision %02X not supported by this code\n",
		       spd_rev);
		بازگشت 1;
	}
	اگر (spd_rev > 0x13) {
		printf("SPD revision %02X not verified by this code\n",
		       spd_rev);
	}

	/*
	 * Calculate checksum
	 */
	برای (i = 0; i < 63; i++) {
		cksum += *buf++;
	}
	cksum &= 0xFF;

	اگر (cksum != spd_cksum) {
		printf("SPD checksum unexpected. "
			"Checksum in SPD = %02X, computed SPD = %02X\n",
			spd_cksum, cksum);
		بازگشت 1;
	}

	بازگشت 0;
}

unsigned صحیح
ddr1_spd_check(ثابت ddr1_spd_eeprom_t *spd)
{
	ثابت u8 *p = (ثابت u8 *)spd;

	بازگشت spd_check(p, spd->spd_rev, spd->cksum);
}

unsigned صحیح
ddr2_spd_check(ثابت ddr2_spd_eeprom_t *spd)
{
	ثابت u8 *p = (ثابت u8 *)spd;

	بازگشت spd_check(p, spd->spd_rev, spd->cksum);
}

/*
 * CRC16 compute for DDR3 SPD
 * Copied from DDR3 SPD spec.
 */
ایستا صحیح
crc16(نویسه *ptr, صحیح شمار)
{
	صحیح crc, i;

	crc = 0;
	وقتی (--شمار >= 0) {
		crc = crc ^ (صحیح)*ptr++ << 8;
		برای (i = 0; i < 8; ++i)
			اگر (crc & 0x8000)
				crc = crc << 1 ^ 0x1021;
			وگرنه
				crc = crc << 1;
	}
	بازگشت crc & 0xffff;
}

unsigned صحیح
ddr3_spd_check(ثابت ddr3_spd_eeprom_t *spd)
{
	نویسه *p = (نویسه *)spd;
	صحیح csum16;
	صحیح len;
	نویسه crc_lsb;	/* byte 126 */
	نویسه crc_msb;	/* byte 127 */

	/*
	 * SPD byte0[7] - CRC coverage
	 * 0 = CRC covers bytes 0~125
	 * 1 = CRC covers bytes 0~116
	 */

	len = !(spd->info_size_crc & 0x80) ? 126 : 117;
	csum16 = crc16(p, len);

	crc_lsb = (نویسه) (csum16 & 0xff);
	crc_msb = (نویسه) (csum16 >> 8);

	اگر (spd->crc[0] == crc_lsb && spd->crc[1] == crc_msb) {
		بازگشت 0;
	} وگرنه {
		printf("SPD checksum unexpected.\n"
			"Checksum lsb in SPD = %02X, computed SPD = %02X\n"
			"Checksum msb in SPD = %02X, computed SPD = %02X\n",
			spd->crc[0], crc_lsb, spd->crc[1], crc_msb);
		بازگشت 1;
	}
}

unsigned صحیح ddr4_spd_check(ثابت ساختار ddr4_spd_eeprom_s *spd)
{
	نویسه *p = (نویسه *)spd;
	صحیح csum16;
	صحیح len;
	نویسه crc_lsb;	/* byte 126 */
	نویسه crc_msb;	/* byte 127 */

	len = 126;
	csum16 = crc16(p, len);

	crc_lsb = (نویسه) (csum16 & 0xff);
	crc_msb = (نویسه) (csum16 >> 8);

	اگر (spd->crc[0] != crc_lsb || spd->crc[1] != crc_msb) {
		printf("SPD checksum unexpected.\n"
			"Checksum lsb in SPD = %02X, computed SPD = %02X\n"
			"Checksum msb in SPD = %02X, computed SPD = %02X\n",
			spd->crc[0], crc_lsb, spd->crc[1], crc_msb);
		بازگشت 1;
	}

	p = (نویسه *)((ulong)spd + 128);
	len = 126;
	csum16 = crc16(p, len);

	crc_lsb = (نویسه) (csum16 & 0xff);
	crc_msb = (نویسه) (csum16 >> 8);

	اگر (spd->mod_section.uc[126] != crc_lsb ||
	    spd->mod_section.uc[127] != crc_msb) {
		printf("SPD checksum unexpected.\n"
			"Checksum lsb in SPD = %02X, computed SPD = %02X\n"
			"Checksum msb in SPD = %02X, computed SPD = %02X\n",
			spd->mod_section.uc[126],
			crc_lsb, spd->mod_section.uc[127],
			crc_msb);
		بازگشت 1;
	}

	بازگشت 0;
}
