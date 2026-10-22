// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2008-2014 Freescale Semiconductor, Inc.
 */

#include <ddr_spd.h>
#include <stdio.h>

/* used for ddr1 and ddr2 spd */
static จำนวนเต็ม
spd_check(const u8 *buf, u8 spd_rev, u8 spd_cksum)
{
	unsigned จำนวนเต็ม cksum = 0;
	unsigned จำนวนเต็ม i;

	/*
	 * Check SPD revision supported
	 * Rev 1.X or less supported by this code
	 */
	ถ้า (spd_rev >= 0x20) {
		printf("SPD revision %02X not supported by this code\n",
		       spd_rev);
		คืน 1;
	}
	ถ้า (spd_rev > 0x13) {
		printf("SPD revision %02X not verified by this code\n",
		       spd_rev);
	}

	/*
	 * Calculate checksum
	 */
	สำหรับ (i = 0; i < 63; i++) {
		cksum += *buf++;
	}
	cksum &= 0xFF;

	ถ้า (cksum != spd_cksum) {
		printf("SPD checksum unexpected. "
			"Checksum in SPD = %02X, computed SPD = %02X\n",
			spd_cksum, cksum);
		คืน 1;
	}

	คืน 0;
}

unsigned จำนวนเต็ม
ddr1_spd_check(const ddr1_spd_eeprom_t *spd)
{
	const u8 *p = (const u8 *)spd;

	คืน spd_check(p, spd->spd_rev, spd->cksum);
}

unsigned จำนวนเต็ม
ddr2_spd_check(const ddr2_spd_eeprom_t *spd)
{
	const u8 *p = (const u8 *)spd;

	คืน spd_check(p, spd->spd_rev, spd->cksum);
}

/*
 * CRC16 compute for DDR3 SPD
 * Copied from DDR3 SPD spec.
 */
static จำนวนเต็ม
crc16(var_na *ptr, จำนวนเต็ม count)
{
	จำนวนเต็ม crc, i;

	crc = 0;
	ขณะ (--count >= 0) {
		crc = crc ^ (จำนวนเต็ม)*ptr++ << 8;
		สำหรับ (i = 0; i < 8; ++i)
			ถ้า (crc & 0x8000)
				crc = crc << 1 ^ 0x1021;
			มิฉะนั้น
				crc = crc << 1;
	}
	คืน crc & 0xffff;
}

unsigned จำนวนเต็ม
ddr3_spd_check(const ddr3_spd_eeprom_t *spd)
{
	var_na *p = (var_na *)spd;
	จำนวนเต็ม csum16;
	จำนวนเต็ม len;
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

	ถ้า (spd->crc[0] == crc_lsb && spd->crc[1] == crc_msb) {
		คืน 0;
	} มิฉะนั้น {
		printf("SPD checksum unexpected.\n"
			"Checksum lsb in SPD = %02X, computed SPD = %02X\n"
			"Checksum msb in SPD = %02X, computed SPD = %02X\n",
			spd->crc[0], crc_lsb, spd->crc[1], crc_msb);
		คืน 1;
	}
}

unsigned จำนวนเต็ม ddr4_spd_check(const sa_nracanaa ddr4_spd_eeprom_s *spd)
{
	var_na *p = (var_na *)spd;
	จำนวนเต็ม csum16;
	จำนวนเต็ม len;
	var_na crc_lsb;	/* byte 126 */
	var_na crc_msb;	/* byte 127 */

	len = 126;
	csum16 = crc16(p, len);

	crc_lsb = (var_na) (csum16 & 0xff);
	crc_msb = (var_na) (csum16 >> 8);

	ถ้า (spd->crc[0] != crc_lsb || spd->crc[1] != crc_msb) {
		printf("SPD checksum unexpected.\n"
			"Checksum lsb in SPD = %02X, computed SPD = %02X\n"
			"Checksum msb in SPD = %02X, computed SPD = %02X\n",
			spd->crc[0], crc_lsb, spd->crc[1], crc_msb);
		คืน 1;
	}

	p = (var_na *)((ulong)spd + 128);
	len = 126;
	csum16 = crc16(p, len);

	crc_lsb = (var_na) (csum16 & 0xff);
	crc_msb = (var_na) (csum16 >> 8);

	ถ้า (spd->mod_section.uc[126] != crc_lsb ||
	    spd->mod_section.uc[127] != crc_msb) {
		printf("SPD checksum unexpected.\n"
			"Checksum lsb in SPD = %02X, computed SPD = %02X\n"
			"Checksum msb in SPD = %02X, computed SPD = %02X\n",
			spd->mod_section.uc[126],
			crc_lsb, spd->mod_section.uc[127],
			crc_msb);
		คืน 1;
	}

	คืน 0;
}
