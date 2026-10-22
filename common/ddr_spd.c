// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2008-2014 Freescale Semiconductor, Inc.
 */

#include <ddr_spd.h>
#include <stdio.h>

/* used for ddr1 and ddr2 spd */
static цео
spd_check(const u8 *buf, u8 spd_rev, u8 spd_cksum)
{
	unsigned цео cksum = 0;
	unsigned цео i;

	/*
	 * Check SPD revision supported
	 * Rev 1.X or less supported by this code
	 */
	ако (spd_rev >= 0x20) {
		printf("SPD revision %02X not supported by this code\n",
		       spd_rev);
		врати 1;
	}
	ако (spd_rev > 0x13) {
		printf("SPD revision %02X not verified by this code\n",
		       spd_rev);
	}

	/*
	 * Calculate checksum
	 */
	за (i = 0; i < 63; i++) {
		cksum += *buf++;
	}
	cksum &= 0xFF;

	ако (cksum != spd_cksum) {
		printf("SPD checksum unexpected. "
			"Checksum in SPD = %02X, computed SPD = %02X\n",
			spd_cksum, cksum);
		врати 1;
	}

	врати 0;
}

unsigned цео
ddr1_spd_check(const ddr1_spd_eeprom_t *spd)
{
	const u8 *p = (const u8 *)spd;

	врати spd_check(p, spd->spd_rev, spd->cksum);
}

unsigned цео
ddr2_spd_check(const ddr2_spd_eeprom_t *spd)
{
	const u8 *p = (const u8 *)spd;

	врати spd_check(p, spd->spd_rev, spd->cksum);
}

/*
 * CRC16 compute for DDR3 SPD
 * Copied from DDR3 SPD spec.
 */
static цео
crc16(char *ptr, цео count)
{
	цео crc, i;

	crc = 0;
	док (--count >= 0) {
		crc = crc ^ (цео)*ptr++ << 8;
		за (i = 0; i < 8; ++i)
			ако (crc & 0x8000)
				crc = crc << 1 ^ 0x1021;
			инако
				crc = crc << 1;
	}
	врати crc & 0xffff;
}

unsigned цео
ddr3_spd_check(const ddr3_spd_eeprom_t *spd)
{
	char *p = (char *)spd;
	цео csum16;
	цео len;
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

	ако (spd->crc[0] == crc_lsb && spd->crc[1] == crc_msb) {
		врати 0;
	} инако {
		printf("SPD checksum unexpected.\n"
			"Checksum lsb in SPD = %02X, computed SPD = %02X\n"
			"Checksum msb in SPD = %02X, computed SPD = %02X\n",
			spd->crc[0], crc_lsb, spd->crc[1], crc_msb);
		врати 1;
	}
}

unsigned цео ddr4_spd_check(const struct ddr4_spd_eeprom_s *spd)
{
	char *p = (char *)spd;
	цео csum16;
	цео len;
	char crc_lsb;	/* byte 126 */
	char crc_msb;	/* byte 127 */

	len = 126;
	csum16 = crc16(p, len);

	crc_lsb = (char) (csum16 & 0xff);
	crc_msb = (char) (csum16 >> 8);

	ако (spd->crc[0] != crc_lsb || spd->crc[1] != crc_msb) {
		printf("SPD checksum unexpected.\n"
			"Checksum lsb in SPD = %02X, computed SPD = %02X\n"
			"Checksum msb in SPD = %02X, computed SPD = %02X\n",
			spd->crc[0], crc_lsb, spd->crc[1], crc_msb);
		врати 1;
	}

	p = (char *)((ulong)spd + 128);
	len = 126;
	csum16 = crc16(p, len);

	crc_lsb = (char) (csum16 & 0xff);
	crc_msb = (char) (csum16 >> 8);

	ако (spd->mod_section.uc[126] != crc_lsb ||
	    spd->mod_section.uc[127] != crc_msb) {
		printf("SPD checksum unexpected.\n"
			"Checksum lsb in SPD = %02X, computed SPD = %02X\n"
			"Checksum msb in SPD = %02X, computed SPD = %02X\n",
			spd->mod_section.uc[126],
			crc_lsb, spd->mod_section.uc[127],
			crc_msb);
		врати 1;
	}

	врати 0;
}
