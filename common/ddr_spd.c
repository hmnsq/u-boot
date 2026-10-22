// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2008-2014 Freescale Semiconductor, Inc.
 */

#include <ddr_spd.h>
#include <stdio.h>

/* used for ddr1 and ddr2 spd */
static 整数
spd_check(const u8 *buf, u8 spd_rev, u8 spd_cksum)
{
	unsigned 整数 cksum = 0;
	unsigned 整数 i;

	/*
	 * Check SPD revision supported
	 * Rev 1.X or less supported by this code
	 */
	もし (spd_rev >= 0x20) {
		printf("SPD revision %02X not supported by this code\n",
		       spd_rev);
		戻る 1;
	}
	もし (spd_rev > 0x13) {
		printf("SPD revision %02X not verified by this code\n",
		       spd_rev);
	}

	/*
	 * Calculate checksum
	 */
	ために (i = 0; i < 63; i++) {
		cksum += *buf++;
	}
	cksum &= 0xFF;

	もし (cksum != spd_cksum) {
		printf("SPD checksum unexpected. "
			"Checksum in SPD = %02X, computed SPD = %02X\n",
			spd_cksum, cksum);
		戻る 1;
	}

	戻る 0;
}

unsigned 整数
ddr1_spd_check(const ddr1_spd_eeprom_t *spd)
{
	const u8 *p = (const u8 *)spd;

	戻る spd_check(p, spd->spd_rev, spd->cksum);
}

unsigned 整数
ddr2_spd_check(const ddr2_spd_eeprom_t *spd)
{
	const u8 *p = (const u8 *)spd;

	戻る spd_check(p, spd->spd_rev, spd->cksum);
}

/*
 * CRC16 compute for DDR3 SPD
 * Copied from DDR3 SPD spec.
 */
static 整数
crc16(char *ptr, 整数 count)
{
	整数 crc, i;

	crc = 0;
	ながら (--count >= 0) {
		crc = crc ^ (整数)*ptr++ << 8;
		ために (i = 0; i < 8; ++i)
			もし (crc & 0x8000)
				crc = crc << 1 ^ 0x1021;
			否则
				crc = crc << 1;
	}
	戻る crc & 0xffff;
}

unsigned 整数
ddr3_spd_check(const ddr3_spd_eeprom_t *spd)
{
	char *p = (char *)spd;
	整数 csum16;
	整数 len;
	char crc_lsb;	/* byte 126 */
	char crc_msb;	/* byte 127 */

	/*
	 * SPD byte0[7] - CRC coverage
	 * 0 = CRC covers bytes 0~125
	 * 1 = CRC covers bytes 0~116
	 */

	len = !(spd->info_size_crc & 0x80) ? 126 : 117;
	csum16 = crc16(p, len);

	crc_lsb = (char) (csum16 & 0xff);
	crc_msb = (char) (csum16 >> 8);

	もし (spd->crc[0] == crc_lsb && spd->crc[1] == crc_msb) {
		戻る 0;
	} 否则 {
		printf("SPD checksum unexpected.\n"
			"Checksum lsb in SPD = %02X, computed SPD = %02X\n"
			"Checksum msb in SPD = %02X, computed SPD = %02X\n",
			spd->crc[0], crc_lsb, spd->crc[1], crc_msb);
		戻る 1;
	}
}

unsigned 整数 ddr4_spd_check(const struct ddr4_spd_eeprom_s *spd)
{
	char *p = (char *)spd;
	整数 csum16;
	整数 len;
	char crc_lsb;	/* byte 126 */
	char crc_msb;	/* byte 127 */

	len = 126;
	csum16 = crc16(p, len);

	crc_lsb = (char) (csum16 & 0xff);
	crc_msb = (char) (csum16 >> 8);

	もし (spd->crc[0] != crc_lsb || spd->crc[1] != crc_msb) {
		printf("SPD checksum unexpected.\n"
			"Checksum lsb in SPD = %02X, computed SPD = %02X\n"
			"Checksum msb in SPD = %02X, computed SPD = %02X\n",
			spd->crc[0], crc_lsb, spd->crc[1], crc_msb);
		戻る 1;
	}

	p = (char *)((ulong)spd + 128);
	len = 126;
	csum16 = crc16(p, len);

	crc_lsb = (char) (csum16 & 0xff);
	crc_msb = (char) (csum16 >> 8);

	もし (spd->mod_section.uc[126] != crc_lsb ||
	    spd->mod_section.uc[127] != crc_msb) {
		printf("SPD checksum unexpected.\n"
			"Checksum lsb in SPD = %02X, computed SPD = %02X\n"
			"Checksum msb in SPD = %02X, computed SPD = %02X\n",
			spd->mod_section.uc[126],
			crc_lsb, spd->mod_section.uc[127],
			crc_msb);
		戻る 1;
	}

	戻る 0;
}
