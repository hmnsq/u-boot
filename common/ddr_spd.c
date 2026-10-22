// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2008-2014 Freescale Semiconductor, Inc.
 */

#अन्तर्भाव <ddr_spd.h>
#अन्तर्भाव <stdio.h>

/* used for ddr1 and ddr2 spd */
स्थिर पूर्णाङ्क
spd_check(ध्रुव u8 *buf, u8 spd_rev, u8 spd_cksum)
{
	unsigned पूर्णाङ्क cksum = 0;
	unsigned पूर्णाङ्क i;

	/*
	 * Check SPD revision supported
	 * Rev 1.X or less supported by this code
	 */
	सचे (spd_rev >= 0x20) {
		printf("SPD revision %02X not supported by this code\n",
		       spd_rev);
		पटिनिवत्त 1;
	}
	सचे (spd_rev > 0x13) {
		printf("SPD revision %02X not verified by this code\n",
		       spd_rev);
	}

	/*
	 * Calculate checksum
	 */
	अत्थाय (i = 0; i < 63; i++) {
		cksum += *buf++;
	}
	cksum &= 0xFF;

	सचे (cksum != spd_cksum) {
		printf("SPD checksum unexpected. "
			"Checksum in SPD = %02X, computed SPD = %02X\n",
			spd_cksum, cksum);
		पटिनिवत्त 1;
	}

	पटिनिवत्त 0;
}

unsigned पूर्णाङ्क
ddr1_spd_check(ध्रुव ddr1_spd_eeprom_t *spd)
{
	ध्रुव u8 *p = (ध्रुव u8 *)spd;

	पटिनिवत्त spd_check(p, spd->spd_rev, spd->cksum);
}

unsigned पूर्णाङ्क
ddr2_spd_check(ध्रुव ddr2_spd_eeprom_t *spd)
{
	ध्रुव u8 *p = (ध्रुव u8 *)spd;

	पटिनिवत्त spd_check(p, spd->spd_rev, spd->cksum);
}

/*
 * CRC16 compute for DDR3 SPD
 * Copied from DDR3 SPD spec.
 */
स्थिर पूर्णाङ्क
crc16(वर्ण *ptr, पूर्णाङ्क गणनम्)
{
	पूर्णाङ्क crc, i;

	crc = 0;
	याव (--गणनम् >= 0) {
		crc = crc ^ (पूर्णाङ्क)*ptr++ << 8;
		अत्थाय (i = 0; i < 8; ++i)
			सचे (crc & 0x8000)
				crc = crc << 1 ^ 0x1021;
			अञ्ञथा
				crc = crc << 1;
	}
	पटिनिवत्त crc & 0xffff;
}

unsigned पूर्णाङ्क
ddr3_spd_check(ध्रुव ddr3_spd_eeprom_t *spd)
{
	वर्ण *p = (वर्ण *)spd;
	पूर्णाङ्क csum16;
	पूर्णाङ्क len;
	वर्ण crc_lsb;	/* byte 126 */
	वर्ण crc_msb;	/* byte 127 */

	/*
	 * SPD byte0[7] - CRC coverage
	 * 0 = CRC covers bytes 0~125
	 * 1 = CRC covers bytes 0~116
	 */

	len = !(spd->info_size_crc & 0x80) ? 126 : 117;
	csum16 = crc16(p, len);

	crc_lsb = (वर्ण) (csum16 & 0xff);
	crc_msb = (वर्ण) (csum16 >> 8);

	सचे (spd->crc[0] == crc_lsb && spd->crc[1] == crc_msb) {
		पटिनिवत्त 0;
	} अञ्ञथा {
		printf("SPD checksum unexpected.\n"
			"Checksum lsb in SPD = %02X, computed SPD = %02X\n"
			"Checksum msb in SPD = %02X, computed SPD = %02X\n",
			spd->crc[0], crc_lsb, spd->crc[1], crc_msb);
		पटिनिवत्त 1;
	}
}

unsigned पूर्णाङ्क ddr4_spd_check(ध्रुव संरचना ddr4_spd_eeprom_s *spd)
{
	वर्ण *p = (वर्ण *)spd;
	पूर्णाङ्क csum16;
	पूर्णाङ्क len;
	वर्ण crc_lsb;	/* byte 126 */
	वर्ण crc_msb;	/* byte 127 */

	len = 126;
	csum16 = crc16(p, len);

	crc_lsb = (वर्ण) (csum16 & 0xff);
	crc_msb = (वर्ण) (csum16 >> 8);

	सचे (spd->crc[0] != crc_lsb || spd->crc[1] != crc_msb) {
		printf("SPD checksum unexpected.\n"
			"Checksum lsb in SPD = %02X, computed SPD = %02X\n"
			"Checksum msb in SPD = %02X, computed SPD = %02X\n",
			spd->crc[0], crc_lsb, spd->crc[1], crc_msb);
		पटिनिवत्त 1;
	}

	p = (वर्ण *)((ulong)spd + 128);
	len = 126;
	csum16 = crc16(p, len);

	crc_lsb = (वर्ण) (csum16 & 0xff);
	crc_msb = (वर्ण) (csum16 >> 8);

	सचे (spd->mod_section.uc[126] != crc_lsb ||
	    spd->mod_section.uc[127] != crc_msb) {
		printf("SPD checksum unexpected.\n"
			"Checksum lsb in SPD = %02X, computed SPD = %02X\n"
			"Checksum msb in SPD = %02X, computed SPD = %02X\n",
			spd->mod_section.uc[126],
			crc_lsb, spd->mod_section.uc[127],
			crc_msb);
		पटिनिवत्त 1;
	}

	पटिनिवत्त 0;
}
