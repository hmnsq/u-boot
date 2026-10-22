// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2008-2014 Freescale Semiconductor, Inc.
 */

#شامل <ddr_spd.h>
#شامل <stdio.h>

/* used for ddr1 and ddr2 spd */
ساکن صحیح
spd_check(ثابت u8 *buf, u8 spd_rev, u8 spd_cksum)
{
	غیرعلامتی صحیح cksum = 0;
	غیرعلامتی صحیح i;

	/*
	 * Check SPD revision supported
	 * Rev 1.X or less supported by this code
	 */
	اگر (spd_rev >= 0x20) {
		لکھو("SPD revision %02X not supported by this code\n",
		       spd_rev);
		واپس 1;
	}
	اگر (spd_rev > 0x13) {
		لکھو("SPD revision %02X not verified by this code\n",
		       spd_rev);
	}

	/*
	 * Calculate checksum
	 */
	برائے (i = 0; i < 63; i++) {
		cksum += *buf++;
	}
	cksum &= 0xFF;

	اگر (cksum != spd_cksum) {
		لکھو("SPD checksum unexpected. "
			"Checksum in SPD = %02X, computed SPD = %02X\n",
			spd_cksum, cksum);
		واپس 1;
	}

	واپس 0;
}

غیرعلامتی صحیح
ddr1_spd_check(ثابت ddr1_spd_eeprom_t *spd)
{
	ثابت u8 *p = (ثابت u8 *)spd;

	واپس spd_check(p, spd->spd_rev, spd->cksum);
}

غیرعلامتی صحیح
ddr2_spd_check(ثابت ddr2_spd_eeprom_t *spd)
{
	ثابت u8 *p = (ثابت u8 *)spd;

	واپس spd_check(p, spd->spd_rev, spd->cksum);
}

/*
 * CRC16 compute for DDR3 SPD
 * Copied from DDR3 SPD spec.
 */
ساکن صحیح
crc16(حرف *ptr, صحیح گنتی)
{
	صحیح چکری_جانچ, i;

	چکری_جانچ = 0;
	جب (--گنتی >= 0) {
		چکری_جانچ = چکری_جانچ ^ (صحیح)*ptr++ << 8;
		برائے (i = 0; i < 8; ++i)
			اگر (چکری_جانچ & 0x8000)
				چکری_جانچ = چکری_جانچ << 1 ^ 0x1021;
			ورنہ
				چکری_جانچ = چکری_جانچ << 1;
	}
	واپس چکری_جانچ & 0xffff;
}

غیرعلامتی صحیح
ddr3_spd_check(ثابت ddr3_spd_eeprom_t *spd)
{
	حرف *p = (حرف *)spd;
	صحیح csum16;
	صحیح len;
	حرف چکری_جانچ_lsb;	/* byte 126 */
	حرف چکری_جانچ_msb;	/* byte 127 */

	/*
	 * SPD byte0[7] - CRC coverage
	 * 0 = CRC covers bytes 0~125
	 * 1 = CRC covers bytes 0~116
	 */

	len = !(spd->info_سائز_چکری_جانچ & 0x80) ? 126 : 117;
	csum16 = crc16(p, len);

	چکری_جانچ_lsb = (حرف) (csum16 & 0xff);
	چکری_جانچ_msb = (حرف) (csum16 >> 8);

	اگر (spd->چکری_جانچ[0] == چکری_جانچ_lsb && spd->چکری_جانچ[1] == چکری_جانچ_msb) {
		واپس 0;
	} ورنہ {
		لکھو("SPD checksum unexpected.\n"
			"Checksum lsb in SPD = %02X, computed SPD = %02X\n"
			"Checksum msb in SPD = %02X, computed SPD = %02X\n",
			spd->چکری_جانچ[0], چکری_جانچ_lsb, spd->چکری_جانچ[1], چکری_جانچ_msb);
		واپس 1;
	}
}

غیرعلامتی صحیح ddr4_spd_check(ثابت ساخت ddr4_spd_eeprom_s *spd)
{
	حرف *p = (حرف *)spd;
	صحیح csum16;
	صحیح len;
	حرف چکری_جانچ_lsb;	/* byte 126 */
	حرف چکری_جانچ_msb;	/* byte 127 */

	len = 126;
	csum16 = crc16(p, len);

	چکری_جانچ_lsb = (حرف) (csum16 & 0xff);
	چکری_جانچ_msb = (حرف) (csum16 >> 8);

	اگر (spd->چکری_جانچ[0] != چکری_جانچ_lsb || spd->چکری_جانچ[1] != چکری_جانچ_msb) {
		لکھو("SPD checksum unexpected.\n"
			"Checksum lsb in SPD = %02X, computed SPD = %02X\n"
			"Checksum msb in SPD = %02X, computed SPD = %02X\n",
			spd->چکری_جانچ[0], چکری_جانچ_lsb, spd->چکری_جانچ[1], چکری_جانچ_msb);
		واپس 1;
	}

	p = (حرف *)((ulong)spd + 128);
	len = 126;
	csum16 = crc16(p, len);

	چکری_جانچ_lsb = (حرف) (csum16 & 0xff);
	چکری_جانچ_msb = (حرف) (csum16 >> 8);

	اگر (spd->mod_section.uc[126] != چکری_جانچ_lsb ||
	    spd->mod_section.uc[127] != چکری_جانچ_msb) {
		لکھو("SPD checksum unexpected.\n"
			"Checksum lsb in SPD = %02X, computed SPD = %02X\n"
			"Checksum msb in SPD = %02X, computed SPD = %02X\n",
			spd->mod_section.uc[126],
			چکری_جانچ_lsb, spd->mod_section.uc[127],
			چکری_جانچ_msb);
		واپس 1;
	}

	واپس 0;
}
