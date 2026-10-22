// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2008-2014 Freescale Semiconductor, Inc.
 */

#அந்தர்பாவ <ddr_spd.h>
#அந்தர்பாவ <stdio.h>

/* used for ddr1 and ddr2 spd */
நிலையான முழு
spd_check(மாறிலி u8 *buf, u8 spd_rev, u8 spd_cksum)
{
	unsigned முழு cksum = 0;
	unsigned முழு i;

	/*
	 * Check SPD revision supported
	 * Rev 1.X or less supported by this code
	 */
	எனில் (spd_rev >= 0x20) {
		printf("SPD revision %02X not supported by this code\n",
		       spd_rev);
		திரும்பு 1;
	}
	எனில் (spd_rev > 0x13) {
		printf("SPD revision %02X not verified by this code\n",
		       spd_rev);
	}

	/*
	 * Calculate checksum
	 */
	க்கு (i = 0; i < 63; i++) {
		cksum += *buf++;
	}
	cksum &= 0xFF;

	எனில் (cksum != spd_cksum) {
		printf("SPD checksum unexpected. "
			"Checksum in SPD = %02X, computed SPD = %02X\n",
			spd_cksum, cksum);
		திரும்பு 1;
	}

	திரும்பு 0;
}

unsigned முழு
ddr1_spd_check(மாறிலி ddr1_spd_eeprom_t *spd)
{
	மாறிலி u8 *p = (மாறிலி u8 *)spd;

	திரும்பு spd_check(p, spd->spd_rev, spd->cksum);
}

unsigned முழு
ddr2_spd_check(மாறிலி ddr2_spd_eeprom_t *spd)
{
	மாறிலி u8 *p = (மாறிலி u8 *)spd;

	திரும்பு spd_check(p, spd->spd_rev, spd->cksum);
}

/*
 * CRC16 compute for DDR3 SPD
 * Copied from DDR3 SPD spec.
 */
நிலையான முழு
crc16(எழுத்து *ptr, முழு மொத்தம்)
{
	முழு crc, i;

	crc = 0;
	போது (--மொத்தம் >= 0) {
		crc = crc ^ (முழு)*ptr++ << 8;
		க்கு (i = 0; i < 8; ++i)
			எனில் (crc & 0x8000)
				crc = crc << 1 ^ 0x1021;
			இல்லையெனில்
				crc = crc << 1;
	}
	திரும்பு crc & 0xffff;
}

unsigned முழு
ddr3_spd_check(மாறிலி ddr3_spd_eeprom_t *spd)
{
	எழுத்து *p = (எழுத்து *)spd;
	முழு csum16;
	முழு len;
	எழுத்து crc_lsb;	/* byte 126 */
	எழுத்து crc_msb;	/* byte 127 */

	/*
	 * SPD byte0[7] - CRC coverage
	 * 0 = CRC covers bytes 0~125
	 * 1 = CRC covers bytes 0~116
	 */

	len = !(spd->info_size_crc & 0x80) ? 126 : 117;
	csum16 = crc16(p, len);

	crc_lsb = (எழுத்து) (csum16 & 0xff);
	crc_msb = (எழுத்து) (csum16 >> 8);

	எனில் (spd->crc[0] == crc_lsb && spd->crc[1] == crc_msb) {
		திரும்பு 0;
	} இல்லையெனில் {
		printf("SPD checksum unexpected.\n"
			"Checksum lsb in SPD = %02X, computed SPD = %02X\n"
			"Checksum msb in SPD = %02X, computed SPD = %02X\n",
			spd->crc[0], crc_lsb, spd->crc[1], crc_msb);
		திரும்பு 1;
	}
}

unsigned முழு ddr4_spd_check(மாறிலி கட்டமைப்பு ddr4_spd_eeprom_s *spd)
{
	எழுத்து *p = (எழுத்து *)spd;
	முழு csum16;
	முழு len;
	எழுத்து crc_lsb;	/* byte 126 */
	எழுத்து crc_msb;	/* byte 127 */

	len = 126;
	csum16 = crc16(p, len);

	crc_lsb = (எழுத்து) (csum16 & 0xff);
	crc_msb = (எழுத்து) (csum16 >> 8);

	எனில் (spd->crc[0] != crc_lsb || spd->crc[1] != crc_msb) {
		printf("SPD checksum unexpected.\n"
			"Checksum lsb in SPD = %02X, computed SPD = %02X\n"
			"Checksum msb in SPD = %02X, computed SPD = %02X\n",
			spd->crc[0], crc_lsb, spd->crc[1], crc_msb);
		திரும்பு 1;
	}

	p = (எழுத்து *)((ulong)spd + 128);
	len = 126;
	csum16 = crc16(p, len);

	crc_lsb = (எழுத்து) (csum16 & 0xff);
	crc_msb = (எழுத்து) (csum16 >> 8);

	எனில் (spd->mod_section.uc[126] != crc_lsb ||
	    spd->mod_section.uc[127] != crc_msb) {
		printf("SPD checksum unexpected.\n"
			"Checksum lsb in SPD = %02X, computed SPD = %02X\n"
			"Checksum msb in SPD = %02X, computed SPD = %02X\n",
			spd->mod_section.uc[126],
			crc_lsb, spd->mod_section.uc[127],
			crc_msb);
		திரும்பு 1;
	}

	திரும்பு 0;
}
