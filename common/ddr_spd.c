// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2008-2014 Freescale Semiconductor, Inc.
 */

#अन्तर्भाव <ddr_spd.h>
#अन्तर्भाव <stdio.h>

/* used for ddr1 and ddr2 spd */
स्थिर पूर्णांक
spd_check(स्थिरांक u8 *buf, u8 spd_rev, u8 spd_cksum)
{
	अचिन्हित पूर्णांक cksum = 0;
	अचिन्हित पूर्णांक i;

	/*
	 * Check SPD revision supported
	 * Rev 1.X or less supported by this code
	 */
	जर (spd_rev >= 0x20) {
		म_लिखो("SPD revision %02X not supported by this code\n",
		       spd_rev);
		परत 1;
	}
	जर (spd_rev > 0x13) {
		म_लिखो("SPD revision %02X not verified by this code\n",
		       spd_rev);
	}

	/*
	 * Calculate checksum
	 */
	साठी (i = 0; i < 63; i++) {
		cksum += *buf++;
	}
	cksum &= 0xFF;

	जर (cksum != spd_cksum) {
		म_लिखो("SPD checksum unexpected. "
			"Checksum in SPD = %02X, computed SPD = %02X\n",
			spd_cksum, cksum);
		परत 1;
	}

	परत 0;
}

अचिन्हित पूर्णांक
ddr1_spd_check(स्थिरांक ddr1_spd_eeprom_t *spd)
{
	स्थिरांक u8 *p = (स्थिरांक u8 *)spd;

	परत spd_check(p, spd->spd_rev, spd->cksum);
}

अचिन्हित पूर्णांक
ddr2_spd_check(स्थिरांक ddr2_spd_eeprom_t *spd)
{
	स्थिरांक u8 *p = (स्थिरांक u8 *)spd;

	परत spd_check(p, spd->spd_rev, spd->cksum);
}

/*
 * CRC16 compute for DDR3 SPD
 * Copied from DDR3 SPD spec.
 */
स्थिर पूर्णांक
crc16(अक्षर *ptr, पूर्णांक मोजणी)
{
	पूर्णांक crc, i;

	crc = 0;
	पर्यंत (--मोजणी >= 0) {
		crc = crc ^ (पूर्णांक)*ptr++ << 8;
		साठी (i = 0; i < 8; ++i)
			जर (crc & 0x8000)
				crc = crc << 1 ^ 0x1021;
			अन्यथा
				crc = crc << 1;
	}
	परत crc & 0xffff;
}

अचिन्हित पूर्णांक
ddr3_spd_check(स्थिरांक ddr3_spd_eeprom_t *spd)
{
	अक्षर *p = (अक्षर *)spd;
	पूर्णांक csum16;
	पूर्णांक len;
	अक्षर crc_lsb;	/* byte 126 */
	अक्षर crc_msb;	/* byte 127 */

	/*
	 * SPD byte0[7] - CRC coverage
	 * 0 = CRC covers bytes 0~125
	 * 1 = CRC covers bytes 0~116
	 */

	len = !(spd->info_size_crc & 0x80) ? 126 : 117;
	csum16 = crc16(p, len);

	crc_lsb = (अक्षर) (csum16 & 0xff);
	crc_msb = (अक्षर) (csum16 >> 8);

	जर (spd->crc[0] == crc_lsb && spd->crc[1] == crc_msb) {
		परत 0;
	} अन्यथा {
		म_लिखो("SPD checksum unexpected.\n"
			"Checksum lsb in SPD = %02X, computed SPD = %02X\n"
			"Checksum msb in SPD = %02X, computed SPD = %02X\n",
			spd->crc[0], crc_lsb, spd->crc[1], crc_msb);
		परत 1;
	}
}

अचिन्हित पूर्णांक ddr4_spd_check(स्थिरांक संरचना ddr4_spd_eeprom_s *spd)
{
	अक्षर *p = (अक्षर *)spd;
	पूर्णांक csum16;
	पूर्णांक len;
	अक्षर crc_lsb;	/* byte 126 */
	अक्षर crc_msb;	/* byte 127 */

	len = 126;
	csum16 = crc16(p, len);

	crc_lsb = (अक्षर) (csum16 & 0xff);
	crc_msb = (अक्षर) (csum16 >> 8);

	जर (spd->crc[0] != crc_lsb || spd->crc[1] != crc_msb) {
		म_लिखो("SPD checksum unexpected.\n"
			"Checksum lsb in SPD = %02X, computed SPD = %02X\n"
			"Checksum msb in SPD = %02X, computed SPD = %02X\n",
			spd->crc[0], crc_lsb, spd->crc[1], crc_msb);
		परत 1;
	}

	p = (अक्षर *)((ulong)spd + 128);
	len = 126;
	csum16 = crc16(p, len);

	crc_lsb = (अक्षर) (csum16 & 0xff);
	crc_msb = (अक्षर) (csum16 >> 8);

	जर (spd->mod_section.uc[126] != crc_lsb ||
	    spd->mod_section.uc[127] != crc_msb) {
		म_लिखो("SPD checksum unexpected.\n"
			"Checksum lsb in SPD = %02X, computed SPD = %02X\n"
			"Checksum msb in SPD = %02X, computed SPD = %02X\n",
			spd->mod_section.uc[126],
			crc_lsb, spd->mod_section.uc[127],
			crc_msb);
		परत 1;
	}

	परत 0;
}
