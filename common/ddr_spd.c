// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2008-2014 Freescale Semiconductor, Inc.
 */

#ضمن <ddr_spd.h>
#ضمن <stdio.h>

/* used for ddr1 and ddr2 spd */
ساكن صحيح
spd_check(ثابت u8 *buf, u8 spd_rev, u8 spd_cksum)
{
	بیعلامت صحيح cksum = 0;
	بیعلامت صحيح i;

	/*
	 * Check SPD revision supported
	 * Rev 1.X or less supported by this code
	 */
	إذا (spd_rev >= 0x20) {
		چاپ_کن("SPD revision %02X not supported by this code\n",
		       spd_rev);
		أرجع 1;
	}
	إذا (spd_rev > 0x13) {
		چاپ_کن("SPD revision %02X not verified by this code\n",
		       spd_rev);
	}

	/*
	 * Calculate checksum
	 */
	لكل (i = 0; i < 63; i++) {
		cksum += *buf++;
	}
	cksum &= 0xFF;

	إذا (cksum != spd_cksum) {
		چاپ_کن("SPD checksum unexpected. "
			"Checksum in SPD = %02X, computed SPD = %02X\n",
			spd_cksum, cksum);
		أرجع 1;
	}

	أرجع 0;
}

بیعلامت صحيح
ddr1_spd_check(ثابت ddr1_spd_eeprom_t *spd)
{
	ثابت u8 *p = (ثابت u8 *)spd;

	أرجع spd_check(p, spd->spd_rev, spd->cksum);
}

بیعلامت صحيح
ddr2_spd_check(ثابت ddr2_spd_eeprom_t *spd)
{
	ثابت u8 *p = (ثابت u8 *)spd;

	أرجع spd_check(p, spd->spd_rev, spd->cksum);
}

/*
 * CRC16 compute for DDR3 SPD
 * Copied from DDR3 SPD spec.
 */
ساكن صحيح
crc16(حرف *ptr, صحيح شمار)
{
	صحيح بررسی_چرخهای, i;

	بررسی_چرخهای = 0;
	طالما (--شمار >= 0) {
		بررسی_چرخهای = بررسی_چرخهای ^ (صحيح)*ptr++ << 8;
		لكل (i = 0; i < 8; ++i)
			إذا (بررسی_چرخهای & 0x8000)
				بررسی_چرخهای = بررسی_چرخهای << 1 ^ 0x1021;
			وإلا
				بررسی_چرخهای = بررسی_چرخهای << 1;
	}
	أرجع بررسی_چرخهای & 0xffff;
}

بیعلامت صحيح
ddr3_spd_check(ثابت ddr3_spd_eeprom_t *spd)
{
	حرف *p = (حرف *)spd;
	صحيح csum16;
	صحيح len;
	حرف بررسی_چرخهای_lsb;	/* byte 126 */
	حرف بررسی_چرخهای_msb;	/* byte 127 */

	/*
	 * SPD byte0[7] - CRC coverage
	 * 0 = CRC covers bytes 0~125
	 * 1 = CRC covers bytes 0~116
	 */

	len = !(spd->info_اندازه_بررسی_چرخهای & 0x80) ? 126 : 117;
	csum16 = crc16(p, len);

	بررسی_چرخهای_lsb = (حرف) (csum16 & 0xff);
	بررسی_چرخهای_msb = (حرف) (csum16 >> 8);

	إذا (spd->بررسی_چرخهای[0] == بررسی_چرخهای_lsb && spd->بررسی_چرخهای[1] == بررسی_چرخهای_msb) {
		أرجع 0;
	} وإلا {
		چاپ_کن("SPD checksum unexpected.\n"
			"Checksum lsb in SPD = %02X, computed SPD = %02X\n"
			"Checksum msb in SPD = %02X, computed SPD = %02X\n",
			spd->بررسی_چرخهای[0], بررسی_چرخهای_lsb, spd->بررسی_چرخهای[1], بررسی_چرخهای_msb);
		أرجع 1;
	}
}

بیعلامت صحيح ddr4_spd_check(ثابت بنية ddr4_spd_eeprom_s *spd)
{
	حرف *p = (حرف *)spd;
	صحيح csum16;
	صحيح len;
	حرف بررسی_چرخهای_lsb;	/* byte 126 */
	حرف بررسی_چرخهای_msb;	/* byte 127 */

	len = 126;
	csum16 = crc16(p, len);

	بررسی_چرخهای_lsb = (حرف) (csum16 & 0xff);
	بررسی_چرخهای_msb = (حرف) (csum16 >> 8);

	إذا (spd->بررسی_چرخهای[0] != بررسی_چرخهای_lsb || spd->بررسی_چرخهای[1] != بررسی_چرخهای_msb) {
		چاپ_کن("SPD checksum unexpected.\n"
			"Checksum lsb in SPD = %02X, computed SPD = %02X\n"
			"Checksum msb in SPD = %02X, computed SPD = %02X\n",
			spd->بررسی_چرخهای[0], بررسی_چرخهای_lsb, spd->بررسی_چرخهای[1], بررسی_چرخهای_msb);
		أرجع 1;
	}

	p = (حرف *)((ulong)spd + 128);
	len = 126;
	csum16 = crc16(p, len);

	بررسی_چرخهای_lsb = (حرف) (csum16 & 0xff);
	بررسی_چرخهای_msb = (حرف) (csum16 >> 8);

	إذا (spd->mod_section.uc[126] != بررسی_چرخهای_lsb ||
	    spd->mod_section.uc[127] != بررسی_چرخهای_msb) {
		چاپ_کن("SPD checksum unexpected.\n"
			"Checksum lsb in SPD = %02X, computed SPD = %02X\n"
			"Checksum msb in SPD = %02X, computed SPD = %02X\n",
			spd->mod_section.uc[126],
			بررسی_چرخهای_lsb, spd->mod_section.uc[127],
			بررسی_چرخهای_msb);
		أرجع 1;
	}

	أرجع 0;
}
