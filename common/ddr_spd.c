// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2008-2014 Freescale Semiconductor, Inc.
 */

#include <ddr_spd.h>
#include <stdio.h>

/* used for ddr1 and ddr2 spd */
static ຈຳນວນເຕັມ
spd_check(const u8 *buf, u8 spd_rev, u8 spd_cksum)
{
	unsigned ຈຳນວນເຕັມ cksum = 0;
	unsigned ຈຳນວນເຕັມ i;

	/*
	 * Check SPD revision supported
	 * Rev 1.X or less supported by this code
	 */
	ຖ້າ (spd_rev >= 0x20) {
		printf("SPD revision %02X not supported by this code\n",
		       spd_rev);
		ສົ່ງຄືນ 1;
	}
	ຖ້າ (spd_rev > 0x13) {
		printf("SPD revision %02X not verified by this code\n",
		       spd_rev);
	}

	/*
	 * Calculate checksum
	 */
	ສຳລັບ (i = 0; i < 63; i++) {
		cksum += *buf++;
	}
	cksum &= 0xFF;

	ຖ້າ (cksum != spd_cksum) {
		printf("SPD checksum unexpected. "
			"Checksum in SPD = %02X, computed SPD = %02X\n",
			spd_cksum, cksum);
		ສົ່ງຄືນ 1;
	}

	ສົ່ງຄືນ 0;
}

unsigned ຈຳນວນເຕັມ
ddr1_spd_check(const ddr1_spd_eeprom_t *spd)
{
	const u8 *p = (const u8 *)spd;

	ສົ່ງຄືນ spd_check(p, spd->spd_rev, spd->cksum);
}

unsigned ຈຳນວນເຕັມ
ddr2_spd_check(const ddr2_spd_eeprom_t *spd)
{
	const u8 *p = (const u8 *)spd;

	ສົ່ງຄືນ spd_check(p, spd->spd_rev, spd->cksum);
}

/*
 * CRC16 compute for DDR3 SPD
 * Copied from DDR3 SPD spec.
 */
static ຈຳນວນເຕັມ
crc16(var_na *ptr, ຈຳນວນເຕັມ count)
{
	ຈຳນວນເຕັມ crc, i;

	crc = 0;
	ໃນຂະນະທີ່ (--count >= 0) {
		crc = crc ^ (ຈຳນວນເຕັມ)*ptr++ << 8;
		ສຳລັບ (i = 0; i < 8; ++i)
			ຖ້າ (crc & 0x8000)
				crc = crc << 1 ^ 0x1021;
			ບໍ່ດັ່ງນັ້ນ
				crc = crc << 1;
	}
	ສົ່ງຄືນ crc & 0xffff;
}

unsigned ຈຳນວນເຕັມ
ddr3_spd_check(const ddr3_spd_eeprom_t *spd)
{
	var_na *p = (var_na *)spd;
	ຈຳນວນເຕັມ csum16;
	ຈຳນວນເຕັມ len;
	var_na crc_lsb;	/* byte 126 */
	var_na crc_msb;	/* byte 127 */

	/*
	 * SPD byte0[7] - CRC coverage
	 * 0 = CRC covers bytes 0~125
	 * 1 = CRC covers bytes 0~116
	 */

	len = !(spd->info_size_crc & 0x80) ? 126 : 117;
	csum16 = crc16(p, len);

	crc_lsb = (var_na) (csum16 & 0xff);
	crc_msb = (var_na) (csum16 >> 8);

	ຖ້າ (spd->crc[0] == crc_lsb && spd->crc[1] == crc_msb) {
		ສົ່ງຄືນ 0;
	} ບໍ່ດັ່ງນັ້ນ {
		printf("SPD checksum unexpected.\n"
			"Checksum lsb in SPD = %02X, computed SPD = %02X\n"
			"Checksum msb in SPD = %02X, computed SPD = %02X\n",
			spd->crc[0], crc_lsb, spd->crc[1], crc_msb);
		ສົ່ງຄືນ 1;
	}
}

unsigned ຈຳນວນເຕັມ ddr4_spd_check(const sa_nracanaa ddr4_spd_eeprom_s *spd)
{
	var_na *p = (var_na *)spd;
	ຈຳນວນເຕັມ csum16;
	ຈຳນວນເຕັມ len;
	var_na crc_lsb;	/* byte 126 */
	var_na crc_msb;	/* byte 127 */

	len = 126;
	csum16 = crc16(p, len);

	crc_lsb = (var_na) (csum16 & 0xff);
	crc_msb = (var_na) (csum16 >> 8);

	ຖ້າ (spd->crc[0] != crc_lsb || spd->crc[1] != crc_msb) {
		printf("SPD checksum unexpected.\n"
			"Checksum lsb in SPD = %02X, computed SPD = %02X\n"
			"Checksum msb in SPD = %02X, computed SPD = %02X\n",
			spd->crc[0], crc_lsb, spd->crc[1], crc_msb);
		ສົ່ງຄືນ 1;
	}

	p = (var_na *)((ulong)spd + 128);
	len = 126;
	csum16 = crc16(p, len);

	crc_lsb = (var_na) (csum16 & 0xff);
	crc_msb = (var_na) (csum16 >> 8);

	ຖ້າ (spd->mod_section.uc[126] != crc_lsb ||
	    spd->mod_section.uc[127] != crc_msb) {
		printf("SPD checksum unexpected.\n"
			"Checksum lsb in SPD = %02X, computed SPD = %02X\n"
			"Checksum msb in SPD = %02X, computed SPD = %02X\n",
			spd->mod_section.uc[126],
			crc_lsb, spd->mod_section.uc[127],
			crc_msb);
		ສົ່ງຄືນ 1;
	}

	ສົ່ງຄືນ 0;
}
