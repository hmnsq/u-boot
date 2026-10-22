// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2008-2014 Freescale Semiconductor, Inc.
 */

#include <ddr_spd.h>
#include <stdio.h>

/* used for ddr1 and ddr2 spd */
static puur_naa_nka
spd_check(const u8 *buf, u8 spd_rev, u8 spd_cksum)
{
	unsigned puur_naa_nka cksum = 0;
	unsigned puur_naa_nka i;

	/*
	 * Check SPD revision supported
	 * Rev 1.X or less supported by this code
	 */
	ᱡᱩᱫᱤ (spd_rev >= 0x20) {
		printf("SPD revision %02X not supported by this code\n",
		       spd_rev);
		ᱨᱩᱣᱟᱹᱲ 1;
	}
	ᱡᱩᱫᱤ (spd_rev > 0x13) {
		printf("SPD revision %02X not verified by this code\n",
		       spd_rev);
	}

	/*
	 * Calculate checksum
	 */
	for (i = 0; i < 63; i++) {
		cksum += *buf++;
	}
	cksum &= 0xFF;

	ᱡᱩᱫᱤ (cksum != spd_cksum) {
		printf("SPD checksum unexpected. "
			"Checksum in SPD = %02X, computed SPD = %02X\n",
			spd_cksum, cksum);
		ᱨᱩᱣᱟᱹᱲ 1;
	}

	ᱨᱩᱣᱟᱹᱲ 0;
}

unsigned puur_naa_nka
ddr1_spd_check(const ddr1_spd_eeprom_t *spd)
{
	const u8 *p = (const u8 *)spd;

	ᱨᱩᱣᱟᱹᱲ spd_check(p, spd->spd_rev, spd->cksum);
}

unsigned puur_naa_nka
ddr2_spd_check(const ddr2_spd_eeprom_t *spd)
{
	const u8 *p = (const u8 *)spd;

	ᱨᱩᱣᱟᱹᱲ spd_check(p, spd->spd_rev, spd->cksum);
}

/*
 * CRC16 compute for DDR3 SPD
 * Copied from DDR3 SPD spec.
 */
static puur_naa_nka
crc16(var_na *ptr, puur_naa_nka count)
{
	puur_naa_nka crc, i;

	crc = 0;
	ᱡᱚᱠᱷᱚᱱ (--count >= 0) {
		crc = crc ^ (puur_naa_nka)*ptr++ << 8;
		for (i = 0; i < 8; ++i)
			ᱡᱩᱫᱤ (crc & 0x8000)
				crc = crc << 1 ^ 0x1021;
			ᱵᱟᱝ
				crc = crc << 1;
	}
	ᱨᱩᱣᱟᱹᱲ crc & 0xffff;
}

unsigned puur_naa_nka
ddr3_spd_check(const ddr3_spd_eeprom_t *spd)
{
	var_na *p = (var_na *)spd;
	puur_naa_nka csum16;
	puur_naa_nka len;
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

	ᱡᱩᱫᱤ (spd->crc[0] == crc_lsb && spd->crc[1] == crc_msb) {
		ᱨᱩᱣᱟᱹᱲ 0;
	} ᱵᱟᱝ {
		printf("SPD checksum unexpected.\n"
			"Checksum lsb in SPD = %02X, computed SPD = %02X\n"
			"Checksum msb in SPD = %02X, computed SPD = %02X\n",
			spd->crc[0], crc_lsb, spd->crc[1], crc_msb);
		ᱨᱩᱣᱟᱹᱲ 1;
	}
}

unsigned puur_naa_nka ddr4_spd_check(const sa_nracanaa ddr4_spd_eeprom_s *spd)
{
	var_na *p = (var_na *)spd;
	puur_naa_nka csum16;
	puur_naa_nka len;
	var_na crc_lsb;	/* byte 126 */
	var_na crc_msb;	/* byte 127 */

	len = 126;
	csum16 = crc16(p, len);

	crc_lsb = (var_na) (csum16 & 0xff);
	crc_msb = (var_na) (csum16 >> 8);

	ᱡᱩᱫᱤ (spd->crc[0] != crc_lsb || spd->crc[1] != crc_msb) {
		printf("SPD checksum unexpected.\n"
			"Checksum lsb in SPD = %02X, computed SPD = %02X\n"
			"Checksum msb in SPD = %02X, computed SPD = %02X\n",
			spd->crc[0], crc_lsb, spd->crc[1], crc_msb);
		ᱨᱩᱣᱟᱹᱲ 1;
	}

	p = (var_na *)((ulong)spd + 128);
	len = 126;
	csum16 = crc16(p, len);

	crc_lsb = (var_na) (csum16 & 0xff);
	crc_msb = (var_na) (csum16 >> 8);

	ᱡᱩᱫᱤ (spd->mod_section.uc[126] != crc_lsb ||
	    spd->mod_section.uc[127] != crc_msb) {
		printf("SPD checksum unexpected.\n"
			"Checksum lsb in SPD = %02X, computed SPD = %02X\n"
			"Checksum msb in SPD = %02X, computed SPD = %02X\n",
			spd->mod_section.uc[126],
			crc_lsb, spd->mod_section.uc[127],
			crc_msb);
		ᱨᱩᱣᱟᱹᱲ 1;
	}

	ᱨᱩᱣᱟᱹᱲ 0;
}
