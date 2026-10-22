// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#include <s_record.h>

static ціле hex1_bin (char  c);
static ціле hex2_bin (char *s);

ціле srec_decode (char *input, ціле *count, ulong *addr, char *data)
{
	ціле	i;
	ціле	v;				/* conversion buffer	*/
	ціле	srec_type;			/* S-Record type	*/
	unsigned char chksum;			/* buffer for checksum	*/

	chksum = 0;

	/* skip anything before 'S', and the 'S' itself.
	 * Return error if not found
	 */

	для (; *input; ++input) {
		якщо (*input == 'S') {		/* skip 'S' */
			++input;
			break;
		}
	}
	якщо (*input == '\0') {			/* no more data?	*/
		повернути (SREC_EMPTY);
	}

	v = *input++;				/* record type		*/

	якщо ((*count = hex2_bin(input)) < 0) {
		повернути (SREC_E_NOSREC);
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
		повернути (SREC_E_BADTYPE);
	}

	/* read address field */
	*addr = 0;

	switch (v) {
	case '3':				/* 4 byte addr field	*/
	case '7':
		якщо ((v = hex2_bin(input)) < 0) {
			повернути (SREC_E_NOSREC);
		}
		*addr  += v;
		chksum += v;
		input  += 2;
		/* FALL THRU */
	case '2':				/* 3 byte addr field	*/
	case '8':
		якщо ((v = hex2_bin(input)) < 0) {
			повернути (SREC_E_NOSREC);
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
		якщо ((v = hex2_bin(input)) < 0) {
			повернути (SREC_E_NOSREC);
		}
		*addr <<= 8;
		*addr  += v;
		chksum += v;
		input  += 2;

		якщо ((v = hex2_bin(input)) < 0) {
			повернути (SREC_E_NOSREC);
		}
		*addr <<= 8;
		*addr  += v;
		chksum += v;
		input  += 2;

		break;
	default:
		повернути (SREC_E_BADTYPE);
	}

	/* convert data and calculate checksum */
	для (i=0; i < *count; ++i) {
		якщо ((v = hex2_bin(input)) < 0) {
			повернути (SREC_E_NOSREC);
		}
		data[i] = v;
		chksum += v;
		input  += 2;
	}

	/* read anc check checksum */
	якщо ((v = hex2_bin(input)) < 0) {
		повернути (SREC_E_NOSREC);
	}

	якщо ((unsigned char)v != (unsigned char)~chksum) {
		повернути (SREC_E_BADCHKS);
	}

	повернути (srec_type);
}

static ціле hex1_bin (char c)
{
	якщо (c >= '0' && c <= '9')
		повернути (c - '0');
	якщо (c >= 'a' && c <= 'f')
		повернути (c + 10 - 'a');
	якщо (c >= 'A' && c <= 'F')
		повернути (c + 10 - 'A');
	повернути (-1);
}

static ціле hex2_bin (char *s)
{
	ціле i, j;

	якщо ((i = hex1_bin(*s++)) < 0) {
		повернути (-1);
	}
	якщо ((j = hex1_bin(*s)) < 0) {
		повернути (-1);
	}

	повернути ((i<<4) + j);
}
