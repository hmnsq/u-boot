// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2008-2014 Freescale Semiconductor, Inc.
 */

#అన్తర్భావ <ddr_spd.h>
#అన్తర్భావ <stdio.h>

/* used for ddr1 and ddr2 spd */
స్థిర పూర్ణాంకం
spd_check(స్థిరరాశి u8 *buf, u8 spd_rev, u8 spd_cksum)
{
	గుర్తులేని పూర్ణాంకం cksum = 0;
	గుర్తులేని పూర్ణాంకం i;

	/*
	 * Check SPD revision supported
	 * Rev 1.X or less supported by this code
	 */
	ఒకవేళ (spd_rev >= 0x20) {
		రాయి("SPD revision %02X not supported by this code\n",
		       spd_rev);
		తిరిగి 1;
	}
	ఒకవేళ (spd_rev > 0x13) {
		రాయి("SPD revision %02X not verified by this code\n",
		       spd_rev);
	}

	/*
	 * Calculate checksum
	 */
	కొరకు (i = 0; i < 63; i++) {
		cksum += *buf++;
	}
	cksum &= 0xFF;

	ఒకవేళ (cksum != spd_cksum) {
		రాయి("SPD checksum unexpected. "
			"Checksum in SPD = %02X, computed SPD = %02X\n",
			spd_cksum, cksum);
		తిరిగి 1;
	}

	తిరిగి 0;
}

గుర్తులేని పూర్ణాంకం
ddr1_spd_check(స్థిరరాశి ddr1_spd_eeprom_t *spd)
{
	స్థిరరాశి u8 *p = (స్థిరరాశి u8 *)spd;

	తిరిగి spd_check(p, spd->spd_rev, spd->cksum);
}

గుర్తులేని పూర్ణాంకం
ddr2_spd_check(స్థిరరాశి ddr2_spd_eeprom_t *spd)
{
	స్థిరరాశి u8 *p = (స్థిరరాశి u8 *)spd;

	తిరిగి spd_check(p, spd->spd_rev, spd->cksum);
}

/*
 * CRC16 compute for DDR3 SPD
 * Copied from DDR3 SPD spec.
 */
స్థిర పూర్ణాంకం
crc16(అక్షరం *ptr, పూర్ణాంకం లెక్క)
{
	పూర్ణాంకం చక్రీయ_తనిఖీ, i;

	చక్రీయ_తనిఖీ = 0;
	అయితే (--లెక్క >= 0) {
		చక్రీయ_తనిఖీ = చక్రీయ_తనిఖీ ^ (పూర్ణాంకం)*ptr++ << 8;
		కొరకు (i = 0; i < 8; ++i)
			ఒకవేళ (చక్రీయ_తనిఖీ & 0x8000)
				చక్రీయ_తనిఖీ = చక్రీయ_తనిఖీ << 1 ^ 0x1021;
			లేకపోతే
				చక్రీయ_తనిఖీ = చక్రీయ_తనిఖీ << 1;
	}
	తిరిగి చక్రీయ_తనిఖీ & 0xffff;
}

గుర్తులేని పూర్ణాంకం
ddr3_spd_check(స్థిరరాశి ddr3_spd_eeprom_t *spd)
{
	అక్షరం *p = (అక్షరం *)spd;
	పూర్ణాంకం csum16;
	పూర్ణాంకం len;
	అక్షరం చక్రీయ_తనిఖీ_lsb;	/* byte 126 */
	అక్షరం చక్రీయ_తనిఖీ_msb;	/* byte 127 */

	/*
	 * SPD byte0[7] - CRC coverage
	 * 0 = CRC covers bytes 0~125
	 * 1 = CRC covers bytes 0~116
	 */

	len = !(spd->info_కొలత_చక్రీయ_తనిఖీ & 0x80) ? 126 : 117;
	csum16 = crc16(p, len);

	చక్రీయ_తనిఖీ_lsb = (అక్షరం) (csum16 & 0xff);
	చక్రీయ_తనిఖీ_msb = (అక్షరం) (csum16 >> 8);

	ఒకవేళ (spd->చక్రీయ_తనిఖీ[0] == చక్రీయ_తనిఖీ_lsb && spd->చక్రీయ_తనిఖీ[1] == చక్రీయ_తనిఖీ_msb) {
		తిరిగి 0;
	} లేకపోతే {
		రాయి("SPD checksum unexpected.\n"
			"Checksum lsb in SPD = %02X, computed SPD = %02X\n"
			"Checksum msb in SPD = %02X, computed SPD = %02X\n",
			spd->చక్రీయ_తనిఖీ[0], చక్రీయ_తనిఖీ_lsb, spd->చక్రీయ_తనిఖీ[1], చక్రీయ_తనిఖీ_msb);
		తిరిగి 1;
	}
}

గుర్తులేని పూర్ణాంకం ddr4_spd_check(స్థిరరాశి నిర్మాణం ddr4_spd_eeprom_s *spd)
{
	అక్షరం *p = (అక్షరం *)spd;
	పూర్ణాంకం csum16;
	పూర్ణాంకం len;
	అక్షరం చక్రీయ_తనిఖీ_lsb;	/* byte 126 */
	అక్షరం చక్రీయ_తనిఖీ_msb;	/* byte 127 */

	len = 126;
	csum16 = crc16(p, len);

	చక్రీయ_తనిఖీ_lsb = (అక్షరం) (csum16 & 0xff);
	చక్రీయ_తనిఖీ_msb = (అక్షరం) (csum16 >> 8);

	ఒకవేళ (spd->చక్రీయ_తనిఖీ[0] != చక్రీయ_తనిఖీ_lsb || spd->చక్రీయ_తనిఖీ[1] != చక్రీయ_తనిఖీ_msb) {
		రాయి("SPD checksum unexpected.\n"
			"Checksum lsb in SPD = %02X, computed SPD = %02X\n"
			"Checksum msb in SPD = %02X, computed SPD = %02X\n",
			spd->చక్రీయ_తనిఖీ[0], చక్రీయ_తనిఖీ_lsb, spd->చక్రీయ_తనిఖీ[1], చక్రీయ_తనిఖీ_msb);
		తిరిగి 1;
	}

	p = (అక్షరం *)((ulong)spd + 128);
	len = 126;
	csum16 = crc16(p, len);

	చక్రీయ_తనిఖీ_lsb = (అక్షరం) (csum16 & 0xff);
	చక్రీయ_తనిఖీ_msb = (అక్షరం) (csum16 >> 8);

	ఒకవేళ (spd->mod_section.uc[126] != చక్రీయ_తనిఖీ_lsb ||
	    spd->mod_section.uc[127] != చక్రీయ_తనిఖీ_msb) {
		రాయి("SPD checksum unexpected.\n"
			"Checksum lsb in SPD = %02X, computed SPD = %02X\n"
			"Checksum msb in SPD = %02X, computed SPD = %02X\n",
			spd->mod_section.uc[126],
			చక్రీయ_తనిఖీ_lsb, spd->mod_section.uc[127],
			చక్రీయ_తనిఖీ_msb);
		తిరిగి 1;
	}

	తిరిగి 0;
}
