// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2008-2014 Freescale Semiconductor, Inc.
 */

#include <ddr_spd.h>
#include <stdio.h>

/* used for ddr1 and ddr2 spd */
static ကိန်းပြည့်
spd_check(const u8 *buf, u8 spd_rev, u8 spd_cksum)
{
	unsigned ကိန်းပြည့် cksum = 0;
	unsigned ကိန်းပြည့် i;

	/*
	 * Check SPD revision supported
	 * Rev 1.X or less supported by this code
	 */
	အကယ်၍ (spd_rev >= 0x20) {
		printf("SPD revision %02X not supported by this code\n",
		       spd_rev);
		ပြန်ပေး 1;
	}
	အကယ်၍ (spd_rev > 0x13) {
		printf("SPD revision %02X not verified by this code\n",
		       spd_rev);
	}

	/*
	 * Calculate checksum
	 */
	အတွက် (i = 0; i < 63; i++) {
		cksum += *buf++;
	}
	cksum &= 0xFF;

	အကယ်၍ (cksum != spd_cksum) {
		printf("SPD checksum unexpected. "
			"Checksum in SPD = %02X, computed SPD = %02X\n",
			spd_cksum, cksum);
		ပြန်ပေး 1;
	}

	ပြန်ပေး 0;
}

unsigned ကိန်းပြည့်
ddr1_spd_check(const ddr1_spd_eeprom_t *spd)
{
	const u8 *p = (const u8 *)spd;

	ပြန်ပေး spd_check(p, spd->spd_rev, spd->cksum);
}

unsigned ကိန်းပြည့်
ddr2_spd_check(const ddr2_spd_eeprom_t *spd)
{
	const u8 *p = (const u8 *)spd;

	ပြန်ပေး spd_check(p, spd->spd_rev, spd->cksum);
}

/*
 * CRC16 compute for DDR3 SPD
 * Copied from DDR3 SPD spec.
 */
static ကိန်းပြည့်
crc16(var_na *ptr, ကိန်းပြည့် count)
{
	ကိန်းပြည့် crc, i;

	crc = 0;
	နေစဉ် (--count >= 0) {
		crc = crc ^ (ကိန်းပြည့်)*ptr++ << 8;
		အတွက် (i = 0; i < 8; ++i)
			အကယ်၍ (crc & 0x8000)
				crc = crc << 1 ^ 0x1021;
			မဟုတ်ပါက
				crc = crc << 1;
	}
	ပြန်ပေး crc & 0xffff;
}

unsigned ကိန်းပြည့်
ddr3_spd_check(const ddr3_spd_eeprom_t *spd)
{
	var_na *p = (var_na *)spd;
	ကိန်းပြည့် csum16;
	ကိန်းပြည့် len;
	var_na crc_lsb;	/* byte 126 */
	var_na crc_msb;	/* byte 127 */

	/*
	 * SPD byte0[7] - CRC coverage
	 * 0 = CRC covers bytes 0~125
	 * 1 = CRC covers bytes 0~116
	 */

	len = !(spd->info_size_crc & 0x80) ? 126 : 117;
	csum16 = crc16(p, len);

	crc_lsb = (var_na) (csum16 & 0xff);
	crc_msb = (var_na) (csum16 >> 8);

	အကယ်၍ (spd->crc[0] == crc_lsb && spd->crc[1] == crc_msb) {
		ပြန်ပေး 0;
	} မဟုတ်ပါက {
		printf("SPD checksum unexpected.\n"
			"Checksum lsb in SPD = %02X, computed SPD = %02X\n"
			"Checksum msb in SPD = %02X, computed SPD = %02X\n",
			spd->crc[0], crc_lsb, spd->crc[1], crc_msb);
		ပြန်ပေး 1;
	}
}

unsigned ကိန်းပြည့် ddr4_spd_check(const sa_nracanaa ddr4_spd_eeprom_s *spd)
{
	var_na *p = (var_na *)spd;
	ကိန်းပြည့် csum16;
	ကိန်းပြည့် len;
	var_na crc_lsb;	/* byte 126 */
	var_na crc_msb;	/* byte 127 */

	len = 126;
	csum16 = crc16(p, len);

	crc_lsb = (var_na) (csum16 & 0xff);
	crc_msb = (var_na) (csum16 >> 8);

	အကယ်၍ (spd->crc[0] != crc_lsb || spd->crc[1] != crc_msb) {
		printf("SPD checksum unexpected.\n"
			"Checksum lsb in SPD = %02X, computed SPD = %02X\n"
			"Checksum msb in SPD = %02X, computed SPD = %02X\n",
			spd->crc[0], crc_lsb, spd->crc[1], crc_msb);
		ပြန်ပေး 1;
	}

	p = (var_na *)((ulong)spd + 128);
	len = 126;
	csum16 = crc16(p, len);

	crc_lsb = (var_na) (csum16 & 0xff);
	crc_msb = (var_na) (csum16 >> 8);

	အကယ်၍ (spd->mod_section.uc[126] != crc_lsb ||
	    spd->mod_section.uc[127] != crc_msb) {
		printf("SPD checksum unexpected.\n"
			"Checksum lsb in SPD = %02X, computed SPD = %02X\n"
			"Checksum msb in SPD = %02X, computed SPD = %02X\n",
			spd->mod_section.uc[126],
			crc_lsb, spd->mod_section.uc[127],
			crc_msb);
		ပြန်ပေး 1;
	}

	ပြန်ပေး 0;
}
