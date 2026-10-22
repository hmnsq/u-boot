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
		ᬬᬾᬦ᭄ (*input == 'S') {		/* skip 'S' */
			++input;
			bha_nga;
		}
	}
	ᬬᬾᬦ᭄ (*input == '\0') {			/* no more data?	*/
		ᬯᬮᬶ (SREC_EMPTY);
	}

	v = *input++;				/* record type		*/

	ᬬᬾᬦ᭄ ((*count = hex2_bin(input)) < 0) {
		ᬯᬮᬶ (SREC_E_NOSREC);
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
		ᬯᬮᬶ (SREC_E_BADTYPE);
	}

	/* read address field */
	*addr = 0;

	switch (v) {
	pak_sa '3':				/* 4 byte addr field	*/
	pak_sa '7':
		ᬬᬾᬦ᭄ ((v = hex2_bin(input)) < 0) {
			ᬯᬮᬶ (SREC_E_NOSREC);
		}
		*addr  += v;
		chksum += v;
		input  += 2;
		/* FALL THRU */
	pak_sa '2':				/* 3 byte addr field	*/
	pak_sa '8':
		ᬬᬾᬦ᭄ ((v = hex2_bin(input)) < 0) {
			ᬯᬮᬶ (SREC_E_NOSREC);
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
		ᬬᬾᬦ᭄ ((v = hex2_bin(input)) < 0) {
			ᬯᬮᬶ (SREC_E_NOSREC);
		}
		*addr <<= 8;
		*addr  += v;
		chksum += v;
		input  += 2;

		ᬬᬾᬦ᭄ ((v = hex2_bin(input)) < 0) {
			ᬯᬮᬶ (SREC_E_NOSREC);
		}
		*addr <<= 8;
		*addr  += v;
		chksum += v;
		input  += 2;

		bha_nga;
	default:
		ᬯᬮᬶ (SREC_E_BADTYPE);
	}

	/* convert data and calculate checksum */
	for (i=0; i < *count; ++i) {
		ᬬᬾᬦ᭄ ((v = hex2_bin(input)) < 0) {
			ᬯᬮᬶ (SREC_E_NOSREC);
		}
		data[i] = v;
		chksum += v;
		input  += 2;
	}

	/* read anc check checksum */
	ᬬᬾᬦ᭄ ((v = hex2_bin(input)) < 0) {
		ᬯᬮᬶ (SREC_E_NOSREC);
	}

	ᬬᬾᬦ᭄ ((unsigned var_na)v != (unsigned var_na)~chksum) {
		ᬯᬮᬶ (SREC_E_BADCHKS);
	}

	ᬯᬮᬶ (srec_type);
}

static puur_naa_nka hex1_bin (var_na c)
{
	ᬬᬾᬦ᭄ (c >= '0' && c <= '9')
		ᬯᬮᬶ (c - '0');
	ᬬᬾᬦ᭄ (c >= 'a' && c <= 'f')
		ᬯᬮᬶ (c + 10 - 'a');
	ᬬᬾᬦ᭄ (c >= 'A' && c <= 'F')
		ᬯᬮᬶ (c + 10 - 'A');
	ᬯᬮᬶ (-1);
}

static puur_naa_nka hex2_bin (var_na *s)
{
	puur_naa_nka i, j;

	ᬬᬾᬦ᭄ ((i = hex1_bin(*s++)) < 0) {
		ᬯᬮᬶ (-1);
	}
	ᬬᬾᬦ᭄ ((j = hex1_bin(*s)) < 0) {
		ᬯᬮᬶ (-1);
	}

	ᬯᬮᬶ ((i<<4) + j);
}
