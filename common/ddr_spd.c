// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2008-2014 Freescale Semiconductor, Inc.
 */

#समावेश <ddr_spd.h>
#समावेश <stdio.h>

/* used for ddr1 and ddr2 spd */
अटल पूर्णांक
spd_check(स्थिर u8 *buf, u8 spd_rev, u8 spd_cksum)
{
	अचिन्हित पूर्णांक cksum = 0;
	अचिन्हित पूर्णांक i;

	/*
	 * Check SPD revision supported
	 * Rev 1.X or less supported by this code
	 */
	अगर (spd_rev >= 0x20) {
		म_लिखो("SPD revision %02X not supported by this code\n",
		       spd_rev);
		वापस 1;
	}
	अगर (spd_rev > 0x13) {
		म_लिखो("SPD revision %02X not verified by this code\n",
		       spd_rev);
	}

	/*
	 * Calculate checksum
	 */
	क्रम (i = 0; i < 63; i++) {
		cksum += *buf++;
	}
	cksum &= 0xFF;

	अगर (cksum != spd_cksum) {
		म_लिखो("SPD checksum unexpected. "
			"Checksum in SPD = %02X, computed SPD = %02X\n",
			spd_cksum, cksum);
		वापस 1;
	}

	वापस 0;
}

अचिन्हित पूर्णांक
ddr1_spd_check(स्थिर ddr1_spd_eeprom_t *spd)
{
	स्थिर u8 *p = (स्थिर u8 *)spd;

	वापस spd_check(p, spd->spd_rev, spd->cksum);
}

अचिन्हित पूर्णांक
ddr2_spd_check(स्थिर ddr2_spd_eeprom_t *spd)
{
	स्थिर u8 *p = (स्थिर u8 *)spd;

	वापस spd_check(p, spd->spd_rev, spd->cksum);
}

/*
 * CRC16 compute for DDR3 SPD
 * Copied from DDR3 SPD spec.
 */
अटल पूर्णांक
crc16(अक्षर *ptr, पूर्णांक गिनती)
{
	पूर्णांक crc, i;

	crc = 0;
	जबतक (--गिनती >= 0) {
		crc = crc ^ (पूर्णांक)*ptr++ << 8;
		क्रम (i = 0; i < 8; ++i)
			अगर (crc & 0x8000)
				crc = crc << 1 ^ 0x1021;
			अन्यथा
				crc = crc << 1;
	}
	वापस crc & 0xffff;
}

अचिन्हित पूर्णांक
ddr3_spd_check(स्थिर ddr3_spd_eeprom_t *spd)
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

	len = !(spd->info_आकार_crc & 0x80) ? 126 : 117;
	csum16 = crc16(p, len);

	crc_lsb = (अक्षर) (csum16 & 0xff);
	crc_msb = (अक्षर) (csum16 >> 8);

	अगर (spd->crc[0] == crc_lsb && spd->crc[1] == crc_msb) {
		वापस 0;
	} अन्यथा {
		म_लिखो("SPD checksum unexpected.\n"
			"Checksum lsb in SPD = %02X, computed SPD = %02X\n"
			"Checksum msb in SPD = %02X, computed SPD = %02X\n",
			spd->crc[0], crc_lsb, spd->crc[1], crc_msb);
		वापस 1;
	}
}

अचिन्हित पूर्णांक ddr4_spd_check(स्थिर काष्ठा ddr4_spd_eeprom_s *spd)
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

	अगर (spd->crc[0] != crc_lsb || spd->crc[1] != crc_msb) {
		म_लिखो("SPD checksum unexpected.\n"
			"Checksum lsb in SPD = %02X, computed SPD = %02X\n"
			"Checksum msb in SPD = %02X, computed SPD = %02X\n",
			spd->crc[0], crc_lsb, spd->crc[1], crc_msb);
		वापस 1;
	}

	p = (अक्षर *)((ulong)spd + 128);
	len = 126;
	csum16 = crc16(p, len);

	crc_lsb = (अक्षर) (csum16 & 0xff);
	crc_msb = (अक्षर) (csum16 >> 8);

	अगर (spd->mod_section.uc[126] != crc_lsb ||
	    spd->mod_section.uc[127] != crc_msb) {
		म_लिखो("SPD checksum unexpected.\n"
			"Checksum lsb in SPD = %02X, computed SPD = %02X\n"
			"Checksum msb in SPD = %02X, computed SPD = %02X\n",
			spd->mod_section.uc[126],
			crc_lsb, spd->mod_section.uc[127],
			crc_msb);
		वापस 1;
	}

	वापस 0;
}
