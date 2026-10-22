// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#include <s_record.h>

static 整数 hex1_bin (char  c);
static 整数 hex2_bin (char *s);

整数 srec_decode (char *input, 整数 *count, ulong *addr, char *data)
{
	整数	i;
	整数	v;				/* conversion buffer	*/
	整数	srec_type;			/* S-Record type	*/
	unsigned char chksum;			/* buffer for checksum	*/

	chksum = 0;

	/* skip anything before 'S', and the 'S' itself.
	 * Return error if not found
	 */

	ために (; *input; ++input) {
		もし (*input == 'S') {		/* skip 'S' */
			++input;
			break;
		}
	}
	もし (*input == '\0') {			/* no more data?	*/
		戻る (SREC_EMPTY);
	}

	v = *input++;				/* record type		*/

	もし ((*count = hex2_bin(input)) < 0) {
		戻る (SREC_E_NOSREC);
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
		戻る (SREC_E_BADTYPE);
	}

	/* read address field */
	*addr = 0;

	switch (v) {
	case '3':				/* 4 byte addr field	*/
	case '7':
		もし ((v = hex2_bin(input)) < 0) {
			戻る (SREC_E_NOSREC);
		}
		*addr  += v;
		chksum += v;
		input  += 2;
		/* FALL THRU */
	case '2':				/* 3 byte addr field	*/
	case '8':
		もし ((v = hex2_bin(input)) < 0) {
			戻る (SREC_E_NOSREC);
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
		もし ((v = hex2_bin(input)) < 0) {
			戻る (SREC_E_NOSREC);
		}
		*addr <<= 8;
		*addr  += v;
		chksum += v;
		input  += 2;

		もし ((v = hex2_bin(input)) < 0) {
			戻る (SREC_E_NOSREC);
		}
		*addr <<= 8;
		*addr  += v;
		chksum += v;
		input  += 2;

		break;
	default:
		戻る (SREC_E_BADTYPE);
	}

	/* convert data and calculate checksum */
	ために (i=0; i < *count; ++i) {
		もし ((v = hex2_bin(input)) < 0) {
			戻る (SREC_E_NOSREC);
		}
		data[i] = v;
		chksum += v;
		input  += 2;
	}

	/* read anc check checksum */
	もし ((v = hex2_bin(input)) < 0) {
		戻る (SREC_E_NOSREC);
	}

	もし ((unsigned char)v != (unsigned char)~chksum) {
		戻る (SREC_E_BADCHKS);
	}

	戻る (srec_type);
}

static 整数 hex1_bin (char c)
{
	もし (c >= '0' && c <= '9')
		戻る (c - '0');
	もし (c >= 'a' && c <= 'f')
		戻る (c + 10 - 'a');
	もし (c >= 'A' && c <= 'F')
		戻る (c + 10 - 'A');
	戻る (-1);
}

static 整数 hex2_bin (char *s)
{
	整数 i, j;

	もし ((i = hex1_bin(*s++)) < 0) {
		戻る (-1);
	}
	もし ((j = hex1_bin(*s)) < 0) {
		戻る (-1);
	}

	戻る ((i<<4) + j);
}
