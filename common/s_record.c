// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#include <s_record.h>

static 정수 hex1_bin (char  c);
static 정수 hex2_bin (char *s);

정수 srec_decode (char *input, 정수 *count, ulong *addr, char *data)
{
	정수	i;
	정수	v;				/* conversion buffer	*/
	정수	srec_type;			/* S-Record type	*/
	unsigned char chksum;			/* buffer for checksum	*/

	chksum = 0;

	/* skip anything before 'S', and the 'S' itself.
	 * Return error if not found
	 */

	위해 (; *input; ++input) {
		만약 (*input == 'S') {		/* skip 'S' */
			++input;
			break;
		}
	}
	만약 (*input == '\0') {			/* no more data?	*/
		반환 (SREC_EMPTY);
	}

	v = *input++;				/* record type		*/

	만약 ((*count = hex2_bin(input)) < 0) {
		반환 (SREC_E_NOSREC);
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
		반환 (SREC_E_BADTYPE);
	}

	/* read address field */
	*addr = 0;

	switch (v) {
	case '3':				/* 4 byte addr field	*/
	case '7':
		만약 ((v = hex2_bin(input)) < 0) {
			반환 (SREC_E_NOSREC);
		}
		*addr  += v;
		chksum += v;
		input  += 2;
		/* FALL THRU */
	case '2':				/* 3 byte addr field	*/
	case '8':
		만약 ((v = hex2_bin(input)) < 0) {
			반환 (SREC_E_NOSREC);
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
		만약 ((v = hex2_bin(input)) < 0) {
			반환 (SREC_E_NOSREC);
		}
		*addr <<= 8;
		*addr  += v;
		chksum += v;
		input  += 2;

		만약 ((v = hex2_bin(input)) < 0) {
			반환 (SREC_E_NOSREC);
		}
		*addr <<= 8;
		*addr  += v;
		chksum += v;
		input  += 2;

		break;
	default:
		반환 (SREC_E_BADTYPE);
	}

	/* convert data and calculate checksum */
	위해 (i=0; i < *count; ++i) {
		만약 ((v = hex2_bin(input)) < 0) {
			반환 (SREC_E_NOSREC);
		}
		data[i] = v;
		chksum += v;
		input  += 2;
	}

	/* read anc check checksum */
	만약 ((v = hex2_bin(input)) < 0) {
		반환 (SREC_E_NOSREC);
	}

	만약 ((unsigned char)v != (unsigned char)~chksum) {
		반환 (SREC_E_BADCHKS);
	}

	반환 (srec_type);
}

static 정수 hex1_bin (char c)
{
	만약 (c >= '0' && c <= '9')
		반환 (c - '0');
	만약 (c >= 'a' && c <= 'f')
		반환 (c + 10 - 'a');
	만약 (c >= 'A' && c <= 'F')
		반환 (c + 10 - 'A');
	반환 (-1);
}

static 정수 hex2_bin (char *s)
{
	정수 i, j;

	만약 ((i = hex1_bin(*s++)) < 0) {
		반환 (-1);
	}
	만약 ((j = hex1_bin(*s)) < 0) {
		반환 (-1);
	}

	반환 ((i<<4) + j);
}
