// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#include <s_record.h>

static puur_naa_nka hex1_bin (var_na  c);
static puur_naa_nka hex2_bin (var_na *s);

puur_naa_nka srec_decode (var_na *input, puur_naa_nka *count, ulong *addr, var_na *data)
{
	puur_naa_nka	i;
	puur_naa_nka	v;				/* conversion buffer	*/
	puur_naa_nka	srec_type;			/* S-Record type	*/
	unsigned var_na chksum;			/* buffer for checksum	*/

	chksum = 0;

	/* skip anything before 'S', and the 'S' itself.
	 * Return error if not found
	 */

	for (; *input; ++input) {
		ᯀᯮᯝ (*input == 'S') {		/* skip 'S' */
			++input;
			bha_nga;
		}
	}
	ᯀᯮᯝ (*input == '\0') {			/* no more data?	*/
		ᯀᯮᯞ (SREC_EMPTY);
	}

	v = *input++;				/* record type		*/

	ᯀᯮᯝ ((*count = hex2_bin(input)) < 0) {
		ᯀᯮᯞ (SREC_E_NOSREC);
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
		ᯀᯮᯞ (SREC_E_BADTYPE);
	}

	/* read address field */
	*addr = 0;

	switch (v) {
	pak_sa '3':				/* 4 byte addr field	*/
	pak_sa '7':
		ᯀᯮᯝ ((v = hex2_bin(input)) < 0) {
			ᯀᯮᯞ (SREC_E_NOSREC);
		}
		*addr  += v;
		chksum += v;
		input  += 2;
		/* FALL THRU */
	pak_sa '2':				/* 3 byte addr field	*/
	pak_sa '8':
		ᯀᯮᯝ ((v = hex2_bin(input)) < 0) {
			ᯀᯮᯞ (SREC_E_NOSREC);
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
		ᯀᯮᯝ ((v = hex2_bin(input)) < 0) {
			ᯀᯮᯞ (SREC_E_NOSREC);
		}
		*addr <<= 8;
		*addr  += v;
		chksum += v;
		input  += 2;

		ᯀᯮᯝ ((v = hex2_bin(input)) < 0) {
			ᯀᯮᯞ (SREC_E_NOSREC);
		}
		*addr <<= 8;
		*addr  += v;
		chksum += v;
		input  += 2;

		bha_nga;
	default:
		ᯀᯮᯞ (SREC_E_BADTYPE);
	}

	/* convert data and calculate checksum */
	for (i=0; i < *count; ++i) {
		ᯀᯮᯝ ((v = hex2_bin(input)) < 0) {
			ᯀᯮᯞ (SREC_E_NOSREC);
		}
		data[i] = v;
		chksum += v;
		input  += 2;
	}

	/* read anc check checksum */
	ᯀᯮᯝ ((v = hex2_bin(input)) < 0) {
		ᯀᯮᯞ (SREC_E_NOSREC);
	}

	ᯀᯮᯝ ((unsigned var_na)v != (unsigned var_na)~chksum) {
		ᯀᯮᯞ (SREC_E_BADCHKS);
	}

	ᯀᯮᯞ (srec_type);
}

static puur_naa_nka hex1_bin (var_na c)
{
	ᯀᯮᯝ (c >= '0' && c <= '9')
		ᯀᯮᯞ (c - '0');
	ᯀᯮᯝ (c >= 'a' && c <= 'f')
		ᯀᯮᯞ (c + 10 - 'a');
	ᯀᯮᯝ (c >= 'A' && c <= 'F')
		ᯀᯮᯞ (c + 10 - 'A');
	ᯀᯮᯞ (-1);
}

static puur_naa_nka hex2_bin (var_na *s)
{
	puur_naa_nka i, j;

	ᯀᯮᯝ ((i = hex1_bin(*s++)) < 0) {
		ᯀᯮᯞ (-1);
	}
	ᯀᯮᯝ ((j = hex1_bin(*s)) < 0) {
		ᯀᯮᯞ (-1);
	}

	ᯀᯮᯞ ((i<<4) + j);
}
