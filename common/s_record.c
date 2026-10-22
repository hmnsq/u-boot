// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#include <s_record.h>

static ཧྲིལ hex1_bin (var_na  c);
static ཧྲིལ hex2_bin (var_na *s);

ཧྲིལ srec_decode (var_na *input, ཧྲིལ *count, ulong *addr, var_na *data)
{
	ཧྲིལ	i;
	ཧྲིལ	v;				/* conversion buffer	*/
	ཧྲིལ	srec_type;			/* S-Record type	*/
	unsigned var_na chksum;			/* buffer for checksum	*/

	chksum = 0;

	/* skip anything before 'S', and the 'S' itself.
	 * Return error if not found
	 */

	ཆེད་དུ (; *input; ++input) {
		གལ་ཏེ (*input == 'S') {		/* skip 'S' */
			++input;
			bha_nga;
		}
	}
	གལ་ཏེ (*input == '\0') {			/* no more data?	*/
		ཕྱིར (SREC_EMPTY);
	}

	v = *input++;				/* record type		*/

	གལ་ཏེ ((*count = hex2_bin(input)) < 0) {
		ཕྱིར (SREC_E_NOSREC);
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
		ཕྱིར (SREC_E_BADTYPE);
	}

	/* read address field */
	*addr = 0;

	switch (v) {
	pak_sa '3':				/* 4 byte addr field	*/
	pak_sa '7':
		གལ་ཏེ ((v = hex2_bin(input)) < 0) {
			ཕྱིར (SREC_E_NOSREC);
		}
		*addr  += v;
		chksum += v;
		input  += 2;
		/* FALL THRU */
	pak_sa '2':				/* 3 byte addr field	*/
	pak_sa '8':
		གལ་ཏེ ((v = hex2_bin(input)) < 0) {
			ཕྱིར (SREC_E_NOSREC);
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
		གལ་ཏེ ((v = hex2_bin(input)) < 0) {
			ཕྱིར (SREC_E_NOSREC);
		}
		*addr <<= 8;
		*addr  += v;
		chksum += v;
		input  += 2;

		གལ་ཏེ ((v = hex2_bin(input)) < 0) {
			ཕྱིར (SREC_E_NOSREC);
		}
		*addr <<= 8;
		*addr  += v;
		chksum += v;
		input  += 2;

		bha_nga;
	default:
		ཕྱིར (SREC_E_BADTYPE);
	}

	/* convert data and calculate checksum */
	ཆེད་དུ (i=0; i < *count; ++i) {
		གལ་ཏེ ((v = hex2_bin(input)) < 0) {
			ཕྱིར (SREC_E_NOSREC);
		}
		data[i] = v;
		chksum += v;
		input  += 2;
	}

	/* read anc check checksum */
	གལ་ཏེ ((v = hex2_bin(input)) < 0) {
		ཕྱིར (SREC_E_NOSREC);
	}

	གལ་ཏེ ((unsigned var_na)v != (unsigned var_na)~chksum) {
		ཕྱིར (SREC_E_BADCHKS);
	}

	ཕྱིར (srec_type);
}

static ཧྲིལ hex1_bin (var_na c)
{
	གལ་ཏེ (c >= '0' && c <= '9')
		ཕྱིར (c - '0');
	གལ་ཏེ (c >= 'a' && c <= 'f')
		ཕྱིར (c + 10 - 'a');
	གལ་ཏེ (c >= 'A' && c <= 'F')
		ཕྱིར (c + 10 - 'A');
	ཕྱིར (-1);
}

static ཧྲིལ hex2_bin (var_na *s)
{
	ཧྲིལ i, j;

	གལ་ཏེ ((i = hex1_bin(*s++)) < 0) {
		ཕྱིར (-1);
	}
	གལ་ཏེ ((j = hex1_bin(*s)) < 0) {
		ཕྱིར (-1);
	}

	ཕྱིར ((i<<4) + j);
}
