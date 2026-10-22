// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#include <s_record.h>

static целое hex1_bin (char  c);
static целое hex2_bin (char *s);

целое srec_decode (char *input, целое *count, ulong *addr, char *data)
{
	целое	i;
	целое	v;				/* conversion buffer	*/
	целое	srec_type;			/* S-Record type	*/
	unsigned char chksum;			/* buffer for checksum	*/

	chksum = 0;

	/* skip anything before 'S', and the 'S' itself.
	 * Return error if not found
	 */

	для (; *input; ++input) {
		если (*input == 'S') {		/* skip 'S' */
			++input;
			break;
		}
	}
	если (*input == '\0') {			/* no more data?	*/
		вернуть (SREC_EMPTY);
	}

	v = *input++;				/* record type		*/

	если ((*count = hex2_bin(input)) < 0) {
		вернуть (SREC_E_NOSREC);
	}

	chksum += *count;
	input  += 2;

	switch (v) {				/* record type		*/

	case '0':				/* start record		*/
		srec_type = SREC_START;		/* 2 byte addr field	*/
		*count   -= 3;			/* - checksum and addr	*/
		break;
	case '1':
		srec_type = SREC_DATA2;		/* 2 byte addr field	*/
		*count   -= 3;			/* - checksum and addr	*/
		break;
	case '2':
		srec_type = SREC_DATA3;		/* 3 byte addr field	*/
		*count   -= 4;			/* - checksum and addr	*/
		break;
	case '3':				/* data record with a	*/
		srec_type = SREC_DATA4;		/* 4 byte addr field	*/
		*count   -= 5;			/* - checksum and addr	*/
		break;
/***	case '4'  ***/
	case '5':			/* count record, addr field contains */
		srec_type = SREC_COUNT;	/* a 2 byte record counter	*/
		*count    = 0;			/* no data		*/
		break;
/***	case '6' -- not used  ***/
	case '7':				/* end record with a	*/
		srec_type = SREC_END4;		/* 4 byte addr field	*/
		*count   -= 5;			/* - checksum and addr	*/
		break;
	case '8':				/* end record with a	*/
		srec_type = SREC_END3;		/* 3 byte addr field	*/
		*count   -= 4;			/* - checksum and addr	*/
		break;
	case '9':				/* end record with a	*/
		srec_type = SREC_END2;		/* 2 byte addr field	*/
		*count   -= 3;			/* - checksum and addr	*/
		break;
	default:
		вернуть (SREC_E_BADTYPE);
	}

	/* read address field */
	*addr = 0;

	switch (v) {
	case '3':				/* 4 byte addr field	*/
	case '7':
		если ((v = hex2_bin(input)) < 0) {
			вернуть (SREC_E_NOSREC);
		}
		*addr  += v;
		chksum += v;
		input  += 2;
		/* FALL THRU */
	case '2':				/* 3 byte addr field	*/
	case '8':
		если ((v = hex2_bin(input)) < 0) {
			вернуть (SREC_E_NOSREC);
		}
		*addr <<= 8;
		*addr  += v;
		chksum += v;
		input  += 2;
		/* FALL THRU */
	case '0':				/* 2 byte addr field	*/
	case '1':
	case '5':
	case '9':
		если ((v = hex2_bin(input)) < 0) {
			вернуть (SREC_E_NOSREC);
		}
		*addr <<= 8;
		*addr  += v;
		chksum += v;
		input  += 2;

		если ((v = hex2_bin(input)) < 0) {
			вернуть (SREC_E_NOSREC);
		}
		*addr <<= 8;
		*addr  += v;
		chksum += v;
		input  += 2;

		break;
	default:
		вернуть (SREC_E_BADTYPE);
	}

	/* convert data and calculate checksum */
	для (i=0; i < *count; ++i) {
		если ((v = hex2_bin(input)) < 0) {
			вернуть (SREC_E_NOSREC);
		}
		data[i] = v;
		chksum += v;
		input  += 2;
	}

	/* read anc check checksum */
	если ((v = hex2_bin(input)) < 0) {
		вернуть (SREC_E_NOSREC);
	}

	если ((unsigned char)v != (unsigned char)~chksum) {
		вернуть (SREC_E_BADCHKS);
	}

	вернуть (srec_type);
}

static целое hex1_bin (char c)
{
	если (c >= '0' && c <= '9')
		вернуть (c - '0');
	если (c >= 'a' && c <= 'f')
		вернуть (c + 10 - 'a');
	если (c >= 'A' && c <= 'F')
		вернуть (c + 10 - 'A');
	вернуть (-1);
}

static целое hex2_bin (char *s)
{
	целое i, j;

	если ((i = hex1_bin(*s++)) < 0) {
		вернуть (-1);
	}
	если ((j = hex1_bin(*s)) < 0) {
		вернуть (-1);
	}

	вернуть ((i<<4) + j);
}
