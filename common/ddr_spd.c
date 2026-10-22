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
	بنانشانی صحيح cksum = 0;
	بنانشانی صحيح i;

	/*
	 * Check SPD revision supported
	 * Rev 1.X or less supported by this code
	 */
	إذا (spd_rev >= 0x20) {
		لکھو("SPD revision %02X not supported by this code\n",
		       spd_rev);
		أرجع 1;
	}
	إذا (spd_rev > 0x13) {
		لکھو("SPD revision %02X not verified by this code\n",
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
		لکھو("SPD checksum unexpected. "
			"Checksum in SPD = %02X, computed SPD = %02X\n",
			spd_cksum, cksum);
		أرجع 1;
	}

	أرجع 0;
}

بنانشانی صحيح
ddr1_spd_check(ثابت ddr1_spd_eeprom_t *spd)
{
	ثابت u8 *p = (ثابت u8 *)spd;

	أرجع spd_check(p, spd->spd_rev, spd->cksum);
}

بنانشانی صحيح
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
crc16(حرف *ptr, صحيح گنتی)
{
	صحيح چکری_جانچ, i;

	چکری_جانچ = 0;
	طالما (--گنتی >= 0) {
		چکری_جانچ = چکری_جانچ ^ (صحيح)*ptr++ << 8;
		لكل (i = 0; i < 8; ++i)
			إذا (چکری_جانچ & 0x8000)
				چکری_جانچ = چکری_جانچ << 1 ^ 0x1021;
			وإلا
				چکری_جانچ = چکری_جانچ << 1;
	}
	أرجع چکری_جانچ & 0xffff;
}

بنانشانی صحيح
ddr3_spd_check(ثابت ddr3_spd_eeprom_t *spd)
{
	حرف *p = (حرف *)spd;
	صحيح csum16;
	صحيح len;
	حرف چکری_جانچ_lsb;	/* byte 126 */
	حرف چکری_جانچ_msb;	/* byte 127 */

	/*
	 * SPD byte0[7] - CRC coverage
	 * 0 = CRC covers bytes 0~125
	 * 1 = CRC covers bytes 0~116
	 */

	len = !(spd->info_ناپ_چکری_جانچ & 0x80) ? 126 : 117;
	csum16 = crc16(p, len);

	چکری_جانچ_lsb = (حرف) (csum16 & 0xff);
	چکری_جانچ_msb = (حرف) (csum16 >> 8);

	إذا (spd->چکری_جانچ[0] == چکری_جانچ_lsb && spd->چکری_جانچ[1] == چکری_جانچ_msb) {
		أرجع 0;
	} وإلا {
		لکھو("SPD checksum unexpected.\n"
			"Checksum lsb in SPD = %02X, computed SPD = %02X\n"
			"Checksum msb in SPD = %02X, computed SPD = %02X\n",
			spd->چکری_جانچ[0], چکری_جانچ_lsb, spd->چکری_جانچ[1], چکری_جانچ_msb);
		أرجع 1;
	}
}

بنانشانی صحيح ddr4_spd_check(ثابت بنية ddr4_spd_eeprom_s *spd)
{
	حرف *p = (حرف *)spd;
	صحيح csum16;
	صحيح len;
	حرف چکری_جانچ_lsb;	/* byte 126 */
	حرف چکری_جانچ_msb;	/* byte 127 */

	len = 126;
	csum16 = crc16(p, len);

	چکری_جانچ_lsb = (حرف) (csum16 & 0xff);
	چکری_جانچ_msb = (حرف) (csum16 >> 8);

	إذا (spd->چکری_جانچ[0] != چکری_جانچ_lsb || spd->چکری_جانچ[1] != چکری_جانچ_msb) {
		لکھو("SPD checksum unexpected.\n"
			"Checksum lsb in SPD = %02X, computed SPD = %02X\n"
			"Checksum msb in SPD = %02X, computed SPD = %02X\n",
			spd->چکری_جانچ[0], چکری_جانچ_lsb, spd->چکری_جانچ[1], چکری_جانچ_msb);
		أرجع 1;
	}

	p = (حرف *)((ulong)spd + 128);
	len = 126;
	csum16 = crc16(p, len);

	چکری_جانچ_lsb = (حرف) (csum16 & 0xff);
	چکری_جانچ_msb = (حرف) (csum16 >> 8);

	إذا (spd->mod_section.uc[126] != چکری_جانچ_lsb ||
	    spd->mod_section.uc[127] != چکری_جانچ_msb) {
		لکھو("SPD checksum unexpected.\n"
			"Checksum lsb in SPD = %02X, computed SPD = %02X\n"
			"Checksum msb in SPD = %02X, computed SPD = %02X\n",
			spd->mod_section.uc[126],
			چکری_جانچ_lsb, spd->mod_section.uc[127],
			چکری_جانچ_msb);
		أرجع 1;
	}

	أرجع 0;
}
