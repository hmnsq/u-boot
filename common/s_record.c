// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#include <s_record.h>

static цео hex1_bin (char  c);
static цео hex2_bin (char *s);

цео srec_decode (char *input, цео *count, ulong *addr, char *data)
{
	цео	i;
	цео	v;				/* conversion buffer	*/
	цео	srec_type;			/* S-Record type	*/
	unsigned char chksum;			/* buffer for checksum	*/

	chksum = 0;

	/* skip anything before 'S', and the 'S' itself.
	 * Return error if not found
	 */

	за (; *input; ++input) {
		ако (*input == 'S') {		/* skip 'S' */
			++input;
			break;
		}
	}
	ако (*input == '\0') {			/* no more data?	*/
		врати (SREC_EMPTY);
	}

	v = *input++;				/* record type		*/

	ако ((*count = hex2_bin(input)) < 0) {
		врати (SREC_E_NOSREC);
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
		врати (SREC_E_BADTYPE);
	}

	/* read address field */
	*addr = 0;

	switch (v) {
	case '3':				/* 4 byte addr field	*/
	case '7':
		ако ((v = hex2_bin(input)) < 0) {
			врати (SREC_E_NOSREC);
		}
		*addr  += v;
		chksum += v;
		input  += 2;
		/* FALL THRU */
	case '2':				/* 3 byte addr field	*/
	case '8':
		ако ((v = hex2_bin(input)) < 0) {
			врати (SREC_E_NOSREC);
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
		ако ((v = hex2_bin(input)) < 0) {
			врати (SREC_E_NOSREC);
		}
		*addr <<= 8;
		*addr  += v;
		chksum += v;
		input  += 2;

		ако ((v = hex2_bin(input)) < 0) {
			врати (SREC_E_NOSREC);
		}
		*addr <<= 8;
		*addr  += v;
		chksum += v;
		input  += 2;

		break;
	default:
		врати (SREC_E_BADTYPE);
	}

	/* convert data and calculate checksum */
	за (i=0; i < *count; ++i) {
		ако ((v = hex2_bin(input)) < 0) {
			врати (SREC_E_NOSREC);
		}
		data[i] = v;
		chksum += v;
		input  += 2;
	}

	/* read anc check checksum */
	ако ((v = hex2_bin(input)) < 0) {
		врати (SREC_E_NOSREC);
	}

	ако ((unsigned char)v != (unsigned char)~chksum) {
		врати (SREC_E_BADCHKS);
	}

	врати (srec_type);
}

static цео hex1_bin (char c)
{
	ако (c >= '0' && c <= '9')
		врати (c - '0');
	ако (c >= 'a' && c <= 'f')
		врати (c + 10 - 'a');
	ако (c >= 'A' && c <= 'F')
		врати (c + 10 - 'A');
	врати (-1);
}

static цео hex2_bin (char *s)
{
	цео i, j;

	ако ((i = hex1_bin(*s++)) < 0) {
		врати (-1);
	}
	ако ((j = hex1_bin(*s)) < 0) {
		врати (-1);
	}

	врати ((i<<4) + j);
}
