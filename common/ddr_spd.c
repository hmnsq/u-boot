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
	अचिन्हित पूर्णाङ्क cksum = 0;
	अचिन्हित पूर्णाङ्क i;

	/*
	 * Check SPD revision supported
	 * Rev 1.X or less supported by this code
	 */
	यदि (spd_rev >= 0x20) {
		म_लिखो("SPD revision %02X not supported by this code\n",
		       spd_rev);
		प्रत्यागम 1;
	}
	यदि (spd_rev > 0x13) {
		म_लिखो("SPD revision %02X not verified by this code\n",
		       spd_rev);
	}

	/*
	 * Calculate checksum
	 */
	प्रति (i = 0; i < 63; i++) {
		cksum += *buf++;
	}
	cksum &= 0xFF;

	यदि (cksum != spd_cksum) {
		म_लिखो("SPD checksum unexpected. "
			"Checksum in SPD = %02X, computed SPD = %02X\n",
			spd_cksum, cksum);
		प्रत्यागम 1;
	}

	प्रत्यागम 0;
}

अचिन्हित पूर्णाङ्क
ddr1_spd_check(ध्रुव ddr1_spd_eeprom_t *spd)
{
	ध्रुव u8 *p = (ध्रुव u8 *)spd;

	प्रत्यागम spd_check(p, spd->spd_rev, spd->cksum);
}

अचिन्हित पूर्णाङ्क
ddr2_spd_check(ध्रुव ddr2_spd_eeprom_t *spd)
{
	ध्रुव u8 *p = (ध्रुव u8 *)spd;

	प्रत्यागम spd_check(p, spd->spd_rev, spd->cksum);
}

/*
 * CRC16 compute for DDR3 SPD
 * Copied from DDR3 SPD spec.
 */
स्थिर पूर्णाङ्क
crc16(वर्ण *ptr, पूर्णाङ्क गिनती)
{
	पूर्णाङ्क crc, i;

	crc = 0;
	यावत् (--गिनती >= 0) {
		crc = crc ^ (पूर्णाङ्क)*ptr++ << 8;
		प्रति (i = 0; i < 8; ++i)
			यदि (crc & 0x8000)
				crc = crc << 1 ^ 0x1021;
			अन्यथा
				crc = crc << 1;
	}
	प्रत्यागम crc & 0xffff;
}

अचिन्हित पूर्णाङ्क
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

	यदि (spd->crc[0] == crc_lsb && spd->crc[1] == crc_msb) {
		प्रत्यागम 0;
	} अन्यथा {
		म_लिखो("SPD checksum unexpected.\n"
			"Checksum lsb in SPD = %02X, computed SPD = %02X\n"
			"Checksum msb in SPD = %02X, computed SPD = %02X\n",
			spd->crc[0], crc_lsb, spd->crc[1], crc_msb);
		प्रत्यागम 1;
	}
}

अचिन्हित पूर्णाङ्क ddr4_spd_check(ध्रुव संरचना ddr4_spd_eeprom_s *spd)
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

	यदि (spd->crc[0] != crc_lsb || spd->crc[1] != crc_msb) {
		म_लिखो("SPD checksum unexpected.\n"
			"Checksum lsb in SPD = %02X, computed SPD = %02X\n"
			"Checksum msb in SPD = %02X, computed SPD = %02X\n",
			spd->crc[0], crc_lsb, spd->crc[1], crc_msb);
		प्रत्यागम 1;
	}

	p = (वर्ण *)((ulong)spd + 128);
	len = 126;
	csum16 = crc16(p, len);

	crc_lsb = (वर्ण) (csum16 & 0xff);
	crc_msb = (वर्ण) (csum16 >> 8);

	यदि (spd->mod_section.uc[126] != crc_lsb ||
	    spd->mod_section.uc[127] != crc_msb) {
		म_लिखो("SPD checksum unexpected.\n"
			"Checksum lsb in SPD = %02X, computed SPD = %02X\n"
			"Checksum msb in SPD = %02X, computed SPD = %02X\n",
			spd->mod_section.uc[126],
			crc_lsb, spd->mod_section.uc[127],
			crc_msb);
		प्रत्यागम 1;
	}

	प्रत्यागम 0;
}
