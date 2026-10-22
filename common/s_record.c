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

	ꦏꦁꦒꦺꦴ (; *input; ++input) {
		ꦪꦼꦤ꧀ (*input == 'S') {		/* skip 'S' */
			++input;
			bha_nga;
		}
	}
	ꦪꦼꦤ꧀ (*input == '\0') {			/* no more data?	*/
		ꦮꦭꦶ (SREC_EMPTY);
	}

	v = *input++;				/* record type		*/

	ꦪꦼꦤ꧀ ((*count = hex2_bin(input)) < 0) {
		ꦮꦭꦶ (SREC_E_NOSREC);
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
		ꦮꦭꦶ (SREC_E_BADTYPE);
	}

	/* read address field */
	*addr = 0;

	switch (v) {
	pak_sa '3':				/* 4 byte addr field	*/
	pak_sa '7':
		ꦪꦼꦤ꧀ ((v = hex2_bin(input)) < 0) {
			ꦮꦭꦶ (SREC_E_NOSREC);
		}
		*addr  += v;
		chksum += v;
		input  += 2;
		/* FALL THRU */
	pak_sa '2':				/* 3 byte addr field	*/
	pak_sa '8':
		ꦪꦼꦤ꧀ ((v = hex2_bin(input)) < 0) {
			ꦮꦭꦶ (SREC_E_NOSREC);
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
		ꦪꦼꦤ꧀ ((v = hex2_bin(input)) < 0) {
			ꦮꦭꦶ (SREC_E_NOSREC);
		}
		*addr <<= 8;
		*addr  += v;
		chksum += v;
		input  += 2;

		ꦪꦼꦤ꧀ ((v = hex2_bin(input)) < 0) {
			ꦮꦭꦶ (SREC_E_NOSREC);
		}
		*addr <<= 8;
		*addr  += v;
		chksum += v;
		input  += 2;

		bha_nga;
	default:
		ꦮꦭꦶ (SREC_E_BADTYPE);
	}

	/* convert data and calculate checksum */
	ꦏꦁꦒꦺꦴ (i=0; i < *count; ++i) {
		ꦪꦼꦤ꧀ ((v = hex2_bin(input)) < 0) {
			ꦮꦭꦶ (SREC_E_NOSREC);
		}
		data[i] = v;
		chksum += v;
		input  += 2;
	}

	/* read anc check checksum */
	ꦪꦼꦤ꧀ ((v = hex2_bin(input)) < 0) {
		ꦮꦭꦶ (SREC_E_NOSREC);
	}

	ꦪꦼꦤ꧀ ((unsigned var_na)v != (unsigned var_na)~chksum) {
		ꦮꦭꦶ (SREC_E_BADCHKS);
	}

	ꦮꦭꦶ (srec_type);
}

static puur_naa_nka hex1_bin (var_na c)
{
	ꦪꦼꦤ꧀ (c >= '0' && c <= '9')
		ꦮꦭꦶ (c - '0');
	ꦪꦼꦤ꧀ (c >= 'a' && c <= 'f')
		ꦮꦭꦶ (c + 10 - 'a');
	ꦪꦼꦤ꧀ (c >= 'A' && c <= 'F')
		ꦮꦭꦶ (c + 10 - 'A');
	ꦮꦭꦶ (-1);
}

static puur_naa_nka hex2_bin (var_na *s)
{
	puur_naa_nka i, j;

	ꦪꦼꦤ꧀ ((i = hex1_bin(*s++)) < 0) {
		ꦮꦭꦶ (-1);
	}
	ꦪꦼꦤ꧀ ((j = hex1_bin(*s)) < 0) {
		ꦮꦭꦶ (-1);
	}

	ꦮꦭꦶ ((i<<4) + j);
}
