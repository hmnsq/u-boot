// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2008-2014 Freescale Semiconductor, Inc.
 */

#അന്തര്ഭാവ <ddr_spd.h>
#അന്തര്ഭാവ <stdio.h>

/* used for ddr1 and ddr2 spd */
സ്ഥിതം പൂർണ്ണസംഖ്യ
spd_check(സ്ഥിരം u8 *buf, u8 spd_rev, u8 spd_cksum)
{
	unsigned പൂർണ്ണസംഖ്യ cksum = 0;
	unsigned പൂർണ്ണസംഖ്യ i;

	/*
	 * Check SPD revision supported
	 * Rev 1.X or less supported by this code
	 */
	എങ്കിൽ (spd_rev >= 0x20) {
		printf("SPD revision %02X not supported by this code\n",
		       spd_rev);
		തിരികെ 1;
	}
	എങ്കിൽ (spd_rev > 0x13) {
		printf("SPD revision %02X not verified by this code\n",
		       spd_rev);
	}

	/*
	 * Calculate checksum
	 */
	വേണ്ടി (i = 0; i < 63; i++) {
		cksum += *buf++;
	}
	cksum &= 0xFF;

	എങ്കിൽ (cksum != spd_cksum) {
		printf("SPD checksum unexpected. "
			"Checksum in SPD = %02X, computed SPD = %02X\n",
			spd_cksum, cksum);
		തിരികെ 1;
	}

	തിരികെ 0;
}

unsigned പൂർണ്ണസംഖ്യ
ddr1_spd_check(സ്ഥിരം ddr1_spd_eeprom_t *spd)
{
	സ്ഥിരം u8 *p = (സ്ഥിരം u8 *)spd;

	തിരികെ spd_check(p, spd->spd_rev, spd->cksum);
}

unsigned പൂർണ്ണസംഖ്യ
ddr2_spd_check(സ്ഥിരം ddr2_spd_eeprom_t *spd)
{
	സ്ഥിരം u8 *p = (സ്ഥിരം u8 *)spd;

	തിരികെ spd_check(p, spd->spd_rev, spd->cksum);
}

/*
 * CRC16 compute for DDR3 SPD
 * Copied from DDR3 SPD spec.
 */
സ്ഥിതം പൂർണ്ണസംഖ്യ
crc16(അക്ഷരം *ptr, പൂർണ്ണസംഖ്യ എണ്ണം)
{
	പൂർണ്ണസംഖ്യ crc, i;

	crc = 0;
	ആകുമ്പോൾ (--എണ്ണം >= 0) {
		crc = crc ^ (പൂർണ്ണസംഖ്യ)*ptr++ << 8;
		വേണ്ടി (i = 0; i < 8; ++i)
			എങ്കിൽ (crc & 0x8000)
				crc = crc << 1 ^ 0x1021;
			അല്ലെങ്കിൽ
				crc = crc << 1;
	}
	തിരികെ crc & 0xffff;
}

unsigned പൂർണ്ണസംഖ്യ
ddr3_spd_check(സ്ഥിരം ddr3_spd_eeprom_t *spd)
{
	അക്ഷരം *p = (അക്ഷരം *)spd;
	പൂർണ്ണസംഖ്യ csum16;
	പൂർണ്ണസംഖ്യ len;
	അക്ഷരം crc_lsb;	/* byte 126 */
	അക്ഷരം crc_msb;	/* byte 127 */

	/*
	 * SPD byte0[7] - CRC coverage
	 * 0 = CRC covers bytes 0~125
	 * 1 = CRC covers bytes 0~116
	 */

	len = !(spd->info_size_crc & 0x80) ? 126 : 117;
	csum16 = crc16(p, len);

	crc_lsb = (അക്ഷരം) (csum16 & 0xff);
	crc_msb = (അക്ഷരം) (csum16 >> 8);

	എങ്കിൽ (spd->crc[0] == crc_lsb && spd->crc[1] == crc_msb) {
		തിരികെ 0;
	} അല്ലെങ്കിൽ {
		printf("SPD checksum unexpected.\n"
			"Checksum lsb in SPD = %02X, computed SPD = %02X\n"
			"Checksum msb in SPD = %02X, computed SPD = %02X\n",
			spd->crc[0], crc_lsb, spd->crc[1], crc_msb);
		തിരികെ 1;
	}
}

unsigned പൂർണ്ണസംഖ്യ ddr4_spd_check(സ്ഥിരം ഘടന ddr4_spd_eeprom_s *spd)
{
	അക്ഷരം *p = (അക്ഷരം *)spd;
	പൂർണ്ണസംഖ്യ csum16;
	പൂർണ്ണസംഖ്യ len;
	അക്ഷരം crc_lsb;	/* byte 126 */
	അക്ഷരം crc_msb;	/* byte 127 */

	len = 126;
	csum16 = crc16(p, len);

	crc_lsb = (അക്ഷരം) (csum16 & 0xff);
	crc_msb = (അക്ഷരം) (csum16 >> 8);

	എങ്കിൽ (spd->crc[0] != crc_lsb || spd->crc[1] != crc_msb) {
		printf("SPD checksum unexpected.\n"
			"Checksum lsb in SPD = %02X, computed SPD = %02X\n"
			"Checksum msb in SPD = %02X, computed SPD = %02X\n",
			spd->crc[0], crc_lsb, spd->crc[1], crc_msb);
		തിരികെ 1;
	}

	p = (അക്ഷരം *)((ulong)spd + 128);
	len = 126;
	csum16 = crc16(p, len);

	crc_lsb = (അക്ഷരം) (csum16 & 0xff);
	crc_msb = (അക്ഷരം) (csum16 >> 8);

	എങ്കിൽ (spd->mod_section.uc[126] != crc_lsb ||
	    spd->mod_section.uc[127] != crc_msb) {
		printf("SPD checksum unexpected.\n"
			"Checksum lsb in SPD = %02X, computed SPD = %02X\n"
			"Checksum msb in SPD = %02X, computed SPD = %02X\n",
			spd->mod_section.uc[126],
			crc_lsb, spd->mod_section.uc[127],
			crc_msb);
		തിരികെ 1;
	}

	തിരികെ 0;
}
