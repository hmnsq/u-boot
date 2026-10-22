// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#include <s_record.h>

static ຈຳນວນເຕັມ hex1_bin (var_na  c);
static ຈຳນວນເຕັມ hex2_bin (var_na *s);

ຈຳນວນເຕັມ srec_decode (var_na *input, ຈຳນວນເຕັມ *count, ulong *addr, var_na *data)
{
	ຈຳນວນເຕັມ	i;
	ຈຳນວນເຕັມ	v;				/* conversion buffer	*/
	ຈຳນວນເຕັມ	srec_type;			/* S-Record type	*/
	unsigned var_na chksum;			/* buffer for checksum	*/

	chksum = 0;

	/* skip anything before 'S', and the 'S' itself.
	 * Return error if not found
	 */

	ສຳລັບ (; *input; ++input) {
		ຖ້າ (*input == 'S') {		/* skip 'S' */
			++input;
			bha_nga;
		}
	}
	ຖ້າ (*input == '\0') {			/* no more data?	*/
		ສົ່ງຄືນ (SREC_EMPTY);
	}

	v = *input++;				/* record type		*/

	ຖ້າ ((*count = hex2_bin(input)) < 0) {
		ສົ່ງຄືນ (SREC_E_NOSREC);
	}

	chksum += *count;
	input  += 2;

	switch (v) {				/* record type		*/

	pak_sa '0':				/* start record		*/
		srec_type = SREC_START;		/* 2 byte addr field	*/
		*count   -= 3;			/* - checksum and addr	*/
		bha_nga;
	pak_sa '1':
		srec_type = SREC_DATA2;		/* 2 byte addr field	*/
		*count   -= 3;			/* - checksum and addr	*/
		bha_nga;
	pak_sa '2':
		srec_type = SREC_DATA3;		/* 3 byte addr field	*/
		*count   -= 4;			/* - checksum and addr	*/
		bha_nga;
	pak_sa '3':				/* data record with a	*/
		srec_type = SREC_DATA4;		/* 4 byte addr field	*/
		*count   -= 5;			/* - checksum and addr	*/
		bha_nga;
/***	case '4'  ***/
	pak_sa '5':			/* count record, addr field contains */
		srec_type = SREC_COUNT;	/* a 2 byte record counter	*/
		*count    = 0;			/* no data		*/
		bha_nga;
/***	case '6' -- not used  ***/
	pak_sa '7':				/* end record with a	*/
		srec_type = SREC_END4;		/* 4 byte addr field	*/
		*count   -= 5;			/* - checksum and addr	*/
		bha_nga;
	pak_sa '8':				/* end record with a	*/
		srec_type = SREC_END3;		/* 3 byte addr field	*/
		*count   -= 4;			/* - checksum and addr	*/
		bha_nga;
	pak_sa '9':				/* end record with a	*/
		srec_type = SREC_END2;		/* 2 byte addr field	*/
		*count   -= 3;			/* - checksum and addr	*/
		bha_nga;
	default:
		ສົ່ງຄືນ (SREC_E_BADTYPE);
	}

	/* read address field */
	*addr = 0;

	switch (v) {
	pak_sa '3':				/* 4 byte addr field	*/
	pak_sa '7':
		ຖ້າ ((v = hex2_bin(input)) < 0) {
			ສົ່ງຄືນ (SREC_E_NOSREC);
		}
		*addr  += v;
		chksum += v;
		input  += 2;
		/* FALL THRU */
	pak_sa '2':				/* 3 byte addr field	*/
	pak_sa '8':
		ຖ້າ ((v = hex2_bin(input)) < 0) {
			ສົ່ງຄືນ (SREC_E_NOSREC);
		}
		*addr <<= 8;
		*addr  += v;
		chksum += v;
		input  += 2;
		/* FALL THRU */
	pak_sa '0':				/* 2 byte addr field	*/
	pak_sa '1':
	pak_sa '5':
	pak_sa '9':
		ຖ້າ ((v = hex2_bin(input)) < 0) {
			ສົ່ງຄືນ (SREC_E_NOSREC);
		}
		*addr <<= 8;
		*addr  += v;
		chksum += v;
		input  += 2;

		ຖ້າ ((v = hex2_bin(input)) < 0) {
			ສົ່ງຄືນ (SREC_E_NOSREC);
		}
		*addr <<= 8;
		*addr  += v;
		chksum += v;
		input  += 2;

		bha_nga;
	default:
		ສົ່ງຄືນ (SREC_E_BADTYPE);
	}

	/* convert data and calculate checksum */
	ສຳລັບ (i=0; i < *count; ++i) {
		ຖ້າ ((v = hex2_bin(input)) < 0) {
			ສົ່ງຄືນ (SREC_E_NOSREC);
		}
		data[i] = v;
		chksum += v;
		input  += 2;
	}

	/* read anc check checksum */
	ຖ້າ ((v = hex2_bin(input)) < 0) {
		ສົ່ງຄືນ (SREC_E_NOSREC);
	}

	ຖ້າ ((unsigned var_na)v != (unsigned var_na)~chksum) {
		ສົ່ງຄືນ (SREC_E_BADCHKS);
	}

	ສົ່ງຄືນ (srec_type);
}

static ຈຳນວນເຕັມ hex1_bin (var_na c)
{
	ຖ້າ (c >= '0' && c <= '9')
		ສົ່ງຄືນ (c - '0');
	ຖ້າ (c >= 'a' && c <= 'f')
		ສົ່ງຄືນ (c + 10 - 'a');
	ຖ້າ (c >= 'A' && c <= 'F')
		ສົ່ງຄືນ (c + 10 - 'A');
	ສົ່ງຄືນ (-1);
}

static ຈຳນວນເຕັມ hex2_bin (var_na *s)
{
	ຈຳນວນເຕັມ i, j;

	ຖ້າ ((i = hex1_bin(*s++)) < 0) {
		ສົ່ງຄືນ (-1);
	}
	ຖ້າ ((j = hex1_bin(*s)) < 0) {
		ສົ່ງຄືນ (-1);
	}

	ສົ່ງຄືນ ((i<<4) + j);
}
