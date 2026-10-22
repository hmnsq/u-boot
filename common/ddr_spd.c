// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2008-2014 Freescale Semiconductor, Inc.
 */

#ਅਨ੍ਤਰ੍ਭਾਵ <ddr_spd.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>

/* used for ddr1 and ddr2 spd */
ਸਥਿਰ ਪੂਰਨ
spd_check(ਅਚਲ u8 *buf, u8 spd_rev, u8 spd_cksum)
{
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ cksum = 0;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ i;

	/*
	 * Check SPD revision supported
	 * Rev 1.X or less supported by this code
	 */
	ਜੇ (spd_rev >= 0x20) {
		ਲਿਖੋ("SPD revision %02X not supported by this code\n",
		       spd_rev);
		ਵਾਪਸ 1;
	}
	ਜੇ (spd_rev > 0x13) {
		ਲਿਖੋ("SPD revision %02X not verified by this code\n",
		       spd_rev);
	}

	/*
	 * Calculate checksum
	 */
	ਲਈ (i = 0; i < 63; i++) {
		cksum += *buf++;
	}
	cksum &= 0xFF;

	ਜੇ (cksum != spd_cksum) {
		ਲਿਖੋ("SPD checksum unexpected. "
			"Checksum in SPD = %02X, computed SPD = %02X\n",
			spd_cksum, cksum);
		ਵਾਪਸ 1;
	}

	ਵਾਪਸ 0;
}

ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ
ddr1_spd_check(ਅਚਲ ddr1_spd_eeprom_t *spd)
{
	ਅਚਲ u8 *p = (ਅਚਲ u8 *)spd;

	ਵਾਪਸ spd_check(p, spd->spd_rev, spd->cksum);
}

ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ
ddr2_spd_check(ਅਚਲ ddr2_spd_eeprom_t *spd)
{
	ਅਚਲ u8 *p = (ਅਚਲ u8 *)spd;

	ਵਾਪਸ spd_check(p, spd->spd_rev, spd->cksum);
}

/*
 * CRC16 compute for DDR3 SPD
 * Copied from DDR3 SPD spec.
 */
ਸਥਿਰ ਪੂਰਨ
crc16(ਅੱਖਰ *ptr, ਪੂਰਨ ਗਿਣਤੀ_2)
{
	ਪੂਰਨ ਚੱਕਰੀ_ਜਾਂਚ, i;

	ਚੱਕਰੀ_ਜਾਂਚ = 0;
	ਜਦੋਂ (--ਗਿਣਤੀ_2 >= 0) {
		ਚੱਕਰੀ_ਜਾਂਚ = ਚੱਕਰੀ_ਜਾਂਚ ^ (ਪੂਰਨ)*ptr++ << 8;
		ਲਈ (i = 0; i < 8; ++i)
			ਜੇ (ਚੱਕਰੀ_ਜਾਂਚ & 0x8000)
				ਚੱਕਰੀ_ਜਾਂਚ = ਚੱਕਰੀ_ਜਾਂਚ << 1 ^ 0x1021;
			ਨਹੀਂਤਾਂ
				ਚੱਕਰੀ_ਜਾਂਚ = ਚੱਕਰੀ_ਜਾਂਚ << 1;
	}
	ਵਾਪਸ ਚੱਕਰੀ_ਜਾਂਚ & 0xffff;
}

ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ
ddr3_spd_check(ਅਚਲ ddr3_spd_eeprom_t *spd)
{
	ਅੱਖਰ *p = (ਅੱਖਰ *)spd;
	ਪੂਰਨ csum16;
	ਪੂਰਨ len;
	ਅੱਖਰ ਚੱਕਰੀ_ਜਾਂਚ_lsb;	/* byte 126 */
	ਅੱਖਰ ਚੱਕਰੀ_ਜਾਂਚ_msb;	/* byte 127 */

	/*
	 * SPD byte0[7] - CRC coverage
	 * 0 = CRC covers bytes 0~125
	 * 1 = CRC covers bytes 0~116
	 */

	len = !(spd->info_ਮਾਪ_ਚੱਕਰੀ_ਜਾਂਚ & 0x80) ? 126 : 117;
	csum16 = crc16(p, len);

	ਚੱਕਰੀ_ਜਾਂਚ_lsb = (ਅੱਖਰ) (csum16 & 0xff);
	ਚੱਕਰੀ_ਜਾਂਚ_msb = (ਅੱਖਰ) (csum16 >> 8);

	ਜੇ (spd->ਚੱਕਰੀ_ਜਾਂਚ[0] == ਚੱਕਰੀ_ਜਾਂਚ_lsb && spd->ਚੱਕਰੀ_ਜਾਂਚ[1] == ਚੱਕਰੀ_ਜਾਂਚ_msb) {
		ਵਾਪਸ 0;
	} ਨਹੀਂਤਾਂ {
		ਲਿਖੋ("SPD checksum unexpected.\n"
			"Checksum lsb in SPD = %02X, computed SPD = %02X\n"
			"Checksum msb in SPD = %02X, computed SPD = %02X\n",
			spd->ਚੱਕਰੀ_ਜਾਂਚ[0], ਚੱਕਰੀ_ਜਾਂਚ_lsb, spd->ਚੱਕਰੀ_ਜਾਂਚ[1], ਚੱਕਰੀ_ਜਾਂਚ_msb);
		ਵਾਪਸ 1;
	}
}

ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ddr4_spd_check(ਅਚਲ ਬਣਤਰ ddr4_spd_eeprom_s *spd)
{
	ਅੱਖਰ *p = (ਅੱਖਰ *)spd;
	ਪੂਰਨ csum16;
	ਪੂਰਨ len;
	ਅੱਖਰ ਚੱਕਰੀ_ਜਾਂਚ_lsb;	/* byte 126 */
	ਅੱਖਰ ਚੱਕਰੀ_ਜਾਂਚ_msb;	/* byte 127 */

	len = 126;
	csum16 = crc16(p, len);

	ਚੱਕਰੀ_ਜਾਂਚ_lsb = (ਅੱਖਰ) (csum16 & 0xff);
	ਚੱਕਰੀ_ਜਾਂਚ_msb = (ਅੱਖਰ) (csum16 >> 8);

	ਜੇ (spd->ਚੱਕਰੀ_ਜਾਂਚ[0] != ਚੱਕਰੀ_ਜਾਂਚ_lsb || spd->ਚੱਕਰੀ_ਜਾਂਚ[1] != ਚੱਕਰੀ_ਜਾਂਚ_msb) {
		ਲਿਖੋ("SPD checksum unexpected.\n"
			"Checksum lsb in SPD = %02X, computed SPD = %02X\n"
			"Checksum msb in SPD = %02X, computed SPD = %02X\n",
			spd->ਚੱਕਰੀ_ਜਾਂਚ[0], ਚੱਕਰੀ_ਜਾਂਚ_lsb, spd->ਚੱਕਰੀ_ਜਾਂਚ[1], ਚੱਕਰੀ_ਜਾਂਚ_msb);
		ਵਾਪਸ 1;
	}

	p = (ਅੱਖਰ *)((ulong)spd + 128);
	len = 126;
	csum16 = crc16(p, len);

	ਚੱਕਰੀ_ਜਾਂਚ_lsb = (ਅੱਖਰ) (csum16 & 0xff);
	ਚੱਕਰੀ_ਜਾਂਚ_msb = (ਅੱਖਰ) (csum16 >> 8);

	ਜੇ (spd->mod_section.uc[126] != ਚੱਕਰੀ_ਜਾਂਚ_lsb ||
	    spd->mod_section.uc[127] != ਚੱਕਰੀ_ਜਾਂਚ_msb) {
		ਲਿਖੋ("SPD checksum unexpected.\n"
			"Checksum lsb in SPD = %02X, computed SPD = %02X\n"
			"Checksum msb in SPD = %02X, computed SPD = %02X\n",
			spd->mod_section.uc[126],
			ਚੱਕਰੀ_ਜਾਂਚ_lsb, spd->mod_section.uc[127],
			ਚੱਕਰੀ_ਜਾਂਚ_msb);
		ਵਾਪਸ 1;
	}

	ਵਾਪਸ 0;
}
