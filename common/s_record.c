// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#include <s_record.h>

static số_nguyên hex1_bin (char  c);
static số_nguyên hex2_bin (char *s);

số_nguyên srec_decode (char *input, số_nguyên *count, ulong *addr, char *data)
{
	số_nguyên	i;
	số_nguyên	v;				/* conversion buffer	*/
	số_nguyên	srec_type;			/* S-Record type	*/
	unsigned char chksum;			/* buffer for checksum	*/

	chksum = 0;

	/* skip anything before 'S', and the 'S' itself.
	 * Return error if not found
	 */

	cho (; *input; ++input) {
		nếu (*input == 'S') {		/* skip 'S' */
			++input;
			break;
		}
	}
	nếu (*input == '\0') {			/* no more data?	*/
		trả_về (SREC_EMPTY);
	}

	v = *input++;				/* record type		*/

	nếu ((*count = hex2_bin(input)) < 0) {
		trả_về (SREC_E_NOSREC);
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
		trả_về (SREC_E_BADTYPE);
	}

	/* read address field */
	*addr = 0;

	switch (v) {
	case '3':				/* 4 byte addr field	*/
	case '7':
		nếu ((v = hex2_bin(input)) < 0) {
			trả_về (SREC_E_NOSREC);
		}
		*addr  += v;
		chksum += v;
		input  += 2;
		/* FALL THRU */
	case '2':				/* 3 byte addr field	*/
	case '8':
		nếu ((v = hex2_bin(input)) < 0) {
			trả_về (SREC_E_NOSREC);
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
		nếu ((v = hex2_bin(input)) < 0) {
			trả_về (SREC_E_NOSREC);
		}
		*addr <<= 8;
		*addr  += v;
		chksum += v;
		input  += 2;

		nếu ((v = hex2_bin(input)) < 0) {
			trả_về (SREC_E_NOSREC);
		}
		*addr <<= 8;
		*addr  += v;
		chksum += v;
		input  += 2;

		break;
	default:
		trả_về (SREC_E_BADTYPE);
	}

	/* convert data and calculate checksum */
	cho (i=0; i < *count; ++i) {
		nếu ((v = hex2_bin(input)) < 0) {
			trả_về (SREC_E_NOSREC);
		}
		data[i] = v;
		chksum += v;
		input  += 2;
	}

	/* read anc check checksum */
	nếu ((v = hex2_bin(input)) < 0) {
		trả_về (SREC_E_NOSREC);
	}

	nếu ((unsigned char)v != (unsigned char)~chksum) {
		trả_về (SREC_E_BADCHKS);
	}

	trả_về (srec_type);
}

static số_nguyên hex1_bin (char c)
{
	nếu (c >= '0' && c <= '9')
		trả_về (c - '0');
	nếu (c >= 'a' && c <= 'f')
		trả_về (c + 10 - 'a');
	nếu (c >= 'A' && c <= 'F')
		trả_về (c + 10 - 'A');
	trả_về (-1);
}

static số_nguyên hex2_bin (char *s)
{
	số_nguyên i, j;

	nếu ((i = hex1_bin(*s++)) < 0) {
		trả_về (-1);
	}
	nếu ((j = hex1_bin(*s)) < 0) {
		trả_về (-1);
	}

	trả_về ((i<<4) + j);
}
