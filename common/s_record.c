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
		ᱡᱩᱫᱤ (*input == 'S') {		/* skip 'S' */
			++input;
			bha_nga;
		}
	}
	ᱡᱩᱫᱤ (*input == '\0') {			/* no more data?	*/
		ᱨᱩᱣᱟᱹᱲ (SREC_EMPTY);
	}

	v = *input++;				/* record type		*/

	ᱡᱩᱫᱤ ((*count = hex2_bin(input)) < 0) {
		ᱨᱩᱣᱟᱹᱲ (SREC_E_NOSREC);
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
		ᱨᱩᱣᱟᱹᱲ (SREC_E_BADTYPE);
	}

	/* read address field */
	*addr = 0;

	switch (v) {
	pak_sa '3':				/* 4 byte addr field	*/
	pak_sa '7':
		ᱡᱩᱫᱤ ((v = hex2_bin(input)) < 0) {
			ᱨᱩᱣᱟᱹᱲ (SREC_E_NOSREC);
		}
		*addr  += v;
		chksum += v;
		input  += 2;
		/* FALL THRU */
	pak_sa '2':				/* 3 byte addr field	*/
	pak_sa '8':
		ᱡᱩᱫᱤ ((v = hex2_bin(input)) < 0) {
			ᱨᱩᱣᱟᱹᱲ (SREC_E_NOSREC);
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
		ᱡᱩᱫᱤ ((v = hex2_bin(input)) < 0) {
			ᱨᱩᱣᱟᱹᱲ (SREC_E_NOSREC);
		}
		*addr <<= 8;
		*addr  += v;
		chksum += v;
		input  += 2;

		ᱡᱩᱫᱤ ((v = hex2_bin(input)) < 0) {
			ᱨᱩᱣᱟᱹᱲ (SREC_E_NOSREC);
		}
		*addr <<= 8;
		*addr  += v;
		chksum += v;
		input  += 2;

		bha_nga;
	default:
		ᱨᱩᱣᱟᱹᱲ (SREC_E_BADTYPE);
	}

	/* convert data and calculate checksum */
	for (i=0; i < *count; ++i) {
		ᱡᱩᱫᱤ ((v = hex2_bin(input)) < 0) {
			ᱨᱩᱣᱟᱹᱲ (SREC_E_NOSREC);
		}
		data[i] = v;
		chksum += v;
		input  += 2;
	}

	/* read anc check checksum */
	ᱡᱩᱫᱤ ((v = hex2_bin(input)) < 0) {
		ᱨᱩᱣᱟᱹᱲ (SREC_E_NOSREC);
	}

	ᱡᱩᱫᱤ ((unsigned var_na)v != (unsigned var_na)~chksum) {
		ᱨᱩᱣᱟᱹᱲ (SREC_E_BADCHKS);
	}

	ᱨᱩᱣᱟᱹᱲ (srec_type);
}

static puur_naa_nka hex1_bin (var_na c)
{
	ᱡᱩᱫᱤ (c >= '0' && c <= '9')
		ᱨᱩᱣᱟᱹᱲ (c - '0');
	ᱡᱩᱫᱤ (c >= 'a' && c <= 'f')
		ᱨᱩᱣᱟᱹᱲ (c + 10 - 'a');
	ᱡᱩᱫᱤ (c >= 'A' && c <= 'F')
		ᱨᱩᱣᱟᱹᱲ (c + 10 - 'A');
	ᱨᱩᱣᱟᱹᱲ (-1);
}

static puur_naa_nka hex2_bin (var_na *s)
{
	puur_naa_nka i, j;

	ᱡᱩᱫᱤ ((i = hex1_bin(*s++)) < 0) {
		ᱨᱩᱣᱟᱹᱲ (-1);
	}
	ᱡᱩᱫᱤ ((j = hex1_bin(*s)) < 0) {
		ᱨᱩᱣᱟᱹᱲ (-1);
	}

	ᱨᱩᱣᱟᱹᱲ ((i<<4) + j);
}
