// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2008-2014 Freescale Semiconductor, Inc.
 */

#ضمن <ddr_spd.h>
#ضمن <stdio.h>

/* used for ddr1 and ddr2 spd */
ساكن صحيح
spd_check(ثابت u8 *buf, u8 spd_rev, u8 spd_cksum)
{
	unsigned صحيح cksum = 0;
	unsigned صحيح i;

	/*
	 * Check SPD revision supported
	 * Rev 1.X or less supported by this code
	 */
	אם (spd_rev >= 0x20) {
		printf("SPD revision %02X not supported by this code\n",
		       spd_rev);
		החזר 1;
	}
	אם (spd_rev > 0x13) {
		printf("SPD revision %02X not verified by this code\n",
		       spd_rev);
	}

	/*
	 * Calculate checksum
	 */
	עבור (i = 0; i < 63; i++) {
		cksum += *buf++;
	}
	cksum &= 0xFF;

	אם (cksum != spd_cksum) {
		printf("SPD checksum unexpected. "
			"Checksum in SPD = %02X, computed SPD = %02X\n",
			spd_cksum, cksum);
		החזר 1;
	}

	החזר 0;
}

unsigned صحيح
ddr1_spd_check(ثابت ddr1_spd_eeprom_t *spd)
{
	ثابت u8 *p = (ثابت u8 *)spd;

	החזר spd_check(p, spd->spd_rev, spd->cksum);
}

unsigned صحيح
ddr2_spd_check(ثابت ddr2_spd_eeprom_t *spd)
{
	ثابت u8 *p = (ثابت u8 *)spd;

	החזר spd_check(p, spd->spd_rev, spd->cksum);
}

/*
 * CRC16 compute for DDR3 SPD
 * Copied from DDR3 SPD spec.
 */
ساكن صحيح
crc16(حرف *ptr, صحيح count)
{
	صحيح crc, i;

	crc = 0;
	כלעוד (--count >= 0) {
		crc = crc ^ (صحيح)*ptr++ << 8;
		עבור (i = 0; i < 8; ++i)
			אם (crc & 0x8000)
				crc = crc << 1 ^ 0x1021;
			אחרת
				crc = crc << 1;
	}
	החזר crc & 0xffff;
}

unsigned صحيح
ddr3_spd_check(ثابت ddr3_spd_eeprom_t *spd)
{
	حرف *p = (حرف *)spd;
	صحيح csum16;
	صحيح len;
	حرف crc_lsb;	/* byte 126 */
	حرف crc_msb;	/* byte 127 */

	/*
	 * SPD byte0[7] - CRC coverage
	 * 0 = CRC covers bytes 0~125
	 * 1 = CRC covers bytes 0~116
	 */

	len = !(spd->info_size_crc & 0x80) ? 126 : 117;
	csum16 = crc16(p, len);

	crc_lsb = (حرف) (csum16 & 0xff);
	crc_msb = (حرف) (csum16 >> 8);

	אם (spd->crc[0] == crc_lsb && spd->crc[1] == crc_msb) {
		החזר 0;
	} אחרת {
		printf("SPD checksum unexpected.\n"
			"Checksum lsb in SPD = %02X, computed SPD = %02X\n"
			"Checksum msb in SPD = %02X, computed SPD = %02X\n",
			spd->crc[0], crc_lsb, spd->crc[1], crc_msb);
		החזר 1;
	}
}

unsigned صحيح ddr4_spd_check(ثابت بنية ddr4_spd_eeprom_s *spd)
{
	حرف *p = (حرف *)spd;
	صحيح csum16;
	صحيح len;
	حرف crc_lsb;	/* byte 126 */
	حرف crc_msb;	/* byte 127 */

	len = 126;
	csum16 = crc16(p, len);

	crc_lsb = (حرف) (csum16 & 0xff);
	crc_msb = (حرف) (csum16 >> 8);

	אם (spd->crc[0] != crc_lsb || spd->crc[1] != crc_msb) {
		printf("SPD checksum unexpected.\n"
			"Checksum lsb in SPD = %02X, computed SPD = %02X\n"
			"Checksum msb in SPD = %02X, computed SPD = %02X\n",
			spd->crc[0], crc_lsb, spd->crc[1], crc_msb);
		החזר 1;
	}

	p = (حرف *)((ulong)spd + 128);
	len = 126;
	csum16 = crc16(p, len);

	crc_lsb = (حرف) (csum16 & 0xff);
	crc_msb = (حرف) (csum16 >> 8);

	אם (spd->mod_section.uc[126] != crc_lsb ||
	    spd->mod_section.uc[127] != crc_msb) {
		printf("SPD checksum unexpected.\n"
			"Checksum lsb in SPD = %02X, computed SPD = %02X\n"
			"Checksum msb in SPD = %02X, computed SPD = %02X\n",
			spd->mod_section.uc[126],
			crc_lsb, spd->mod_section.uc[127],
			crc_msb);
		החזר 1;
	}

	החזר 0;
}
