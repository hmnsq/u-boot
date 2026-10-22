// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2008-2014 Freescale Semiconductor, Inc.
 */

#include <ddr_spd.h>
#include <stdio.h>

/* used for ddr1 and ddr2 spd */
static 정수
spd_check(const u8 *buf, u8 spd_rev, u8 spd_cksum)
{
	unsigned 정수 cksum = 0;
	unsigned 정수 i;

	/*
	 * Check SPD revision supported
	 * Rev 1.X or less supported by this code
	 */
	만약 (spd_rev >= 0x20) {
		printf("SPD revision %02X not supported by this code\n",
		       spd_rev);
		반환 1;
	}
	만약 (spd_rev > 0x13) {
		printf("SPD revision %02X not verified by this code\n",
		       spd_rev);
	}

	/*
	 * Calculate checksum
	 */
	위해 (i = 0; i < 63; i++) {
		cksum += *buf++;
	}
	cksum &= 0xFF;

	만약 (cksum != spd_cksum) {
		printf("SPD checksum unexpected. "
			"Checksum in SPD = %02X, computed SPD = %02X\n",
			spd_cksum, cksum);
		반환 1;
	}

	반환 0;
}

unsigned 정수
ddr1_spd_check(const ddr1_spd_eeprom_t *spd)
{
	const u8 *p = (const u8 *)spd;

	반환 spd_check(p, spd->spd_rev, spd->cksum);
}

unsigned 정수
ddr2_spd_check(const ddr2_spd_eeprom_t *spd)
{
	const u8 *p = (const u8 *)spd;

	반환 spd_check(p, spd->spd_rev, spd->cksum);
}

/*
 * CRC16 compute for DDR3 SPD
 * Copied from DDR3 SPD spec.
 */
static 정수
crc16(char *ptr, 정수 count)
{
	정수 crc, i;

	crc = 0;
	동안 (--count >= 0) {
		crc = crc ^ (정수)*ptr++ << 8;
		위해 (i = 0; i < 8; ++i)
			만약 (crc & 0x8000)
				crc = crc << 1 ^ 0x1021;
			아니면
				crc = crc << 1;
	}
	반환 crc & 0xffff;
}

unsigned 정수
ddr3_spd_check(const ddr3_spd_eeprom_t *spd)
{
	char *p = (char *)spd;
	정수 csum16;
	정수 len;
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

	만약 (spd->crc[0] == crc_lsb && spd->crc[1] == crc_msb) {
		반환 0;
	} 아니면 {
		printf("SPD checksum unexpected.\n"
			"Checksum lsb in SPD = %02X, computed SPD = %02X\n"
			"Checksum msb in SPD = %02X, computed SPD = %02X\n",
			spd->crc[0], crc_lsb, spd->crc[1], crc_msb);
		반환 1;
	}
}

unsigned 정수 ddr4_spd_check(const struct ddr4_spd_eeprom_s *spd)
{
	char *p = (char *)spd;
	정수 csum16;
	정수 len;
	char crc_lsb;	/* byte 126 */
	char crc_msb;	/* byte 127 */

	len = 126;
	csum16 = crc16(p, len);

	crc_lsb = (char) (csum16 & 0xff);
	crc_msb = (char) (csum16 >> 8);

	만약 (spd->crc[0] != crc_lsb || spd->crc[1] != crc_msb) {
		printf("SPD checksum unexpected.\n"
			"Checksum lsb in SPD = %02X, computed SPD = %02X\n"
			"Checksum msb in SPD = %02X, computed SPD = %02X\n",
			spd->crc[0], crc_lsb, spd->crc[1], crc_msb);
		반환 1;
	}

	p = (char *)((ulong)spd + 128);
	len = 126;
	csum16 = crc16(p, len);

	crc_lsb = (char) (csum16 & 0xff);
	crc_msb = (char) (csum16 >> 8);

	만약 (spd->mod_section.uc[126] != crc_lsb ||
	    spd->mod_section.uc[127] != crc_msb) {
		printf("SPD checksum unexpected.\n"
			"Checksum lsb in SPD = %02X, computed SPD = %02X\n"
			"Checksum msb in SPD = %02X, computed SPD = %02X\n",
			spd->mod_section.uc[126],
			crc_lsb, spd->mod_section.uc[127],
			crc_msb);
		반환 1;
	}

	반환 0;
}
