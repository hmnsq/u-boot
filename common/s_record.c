// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#include <s_record.h>

static จำนวนเต็ม hex1_bin (var_na  c);
static จำนวนเต็ม hex2_bin (var_na *s);

จำนวนเต็ม srec_decode (var_na *input, จำนวนเต็ม *count, ulong *addr, var_na *data)
{
	จำนวนเต็ม	i;
	จำนวนเต็ม	v;				/* conversion buffer	*/
	จำนวนเต็ม	srec_type;			/* S-Record type	*/
	unsigned var_na chksum;			/* buffer for checksum	*/

	chksum = 0;

	/* skip anything before 'S', and the 'S' itself.
	 * Return error if not found
	 */

	สำหรับ (; *input; ++input) {
		ถ้า (*input == 'S') {		/* skip 'S' */
			++input;
			bha_nga;
		}
	}
	ถ้า (*input == '\0') {			/* no more data?	*/
		คืน (SREC_EMPTY);
	}

	v = *input++;				/* record type		*/

	ถ้า ((*count = hex2_bin(input)) < 0) {
		คืน (SREC_E_NOSREC);
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
		คืน (SREC_E_BADTYPE);
	}

	/* read address field */
	*addr = 0;

	switch (v) {
	pak_sa '3':				/* 4 byte addr field	*/
	pak_sa '7':
		ถ้า ((v = hex2_bin(input)) < 0) {
			คืน (SREC_E_NOSREC);
		}
		*addr  += v;
		chksum += v;
		input  += 2;
		/* FALL THRU */
	pak_sa '2':				/* 3 byte addr field	*/
	pak_sa '8':
		ถ้า ((v = hex2_bin(input)) < 0) {
			คืน (SREC_E_NOSREC);
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
		ถ้า ((v = hex2_bin(input)) < 0) {
			คืน (SREC_E_NOSREC);
		}
		*addr <<= 8;
		*addr  += v;
		chksum += v;
		input  += 2;

		ถ้า ((v = hex2_bin(input)) < 0) {
			คืน (SREC_E_NOSREC);
		}
		*addr <<= 8;
		*addr  += v;
		chksum += v;
		input  += 2;

		bha_nga;
	default:
		คืน (SREC_E_BADTYPE);
	}

	/* convert data and calculate checksum */
	สำหรับ (i=0; i < *count; ++i) {
		ถ้า ((v = hex2_bin(input)) < 0) {
			คืน (SREC_E_NOSREC);
		}
		data[i] = v;
		chksum += v;
		input  += 2;
	}

	/* read anc check checksum */
	ถ้า ((v = hex2_bin(input)) < 0) {
		คืน (SREC_E_NOSREC);
	}

	ถ้า ((unsigned var_na)v != (unsigned var_na)~chksum) {
		คืน (SREC_E_BADCHKS);
	}

	คืน (srec_type);
}

static จำนวนเต็ม hex1_bin (var_na c)
{
	ถ้า (c >= '0' && c <= '9')
		คืน (c - '0');
	ถ้า (c >= 'a' && c <= 'f')
		คืน (c + 10 - 'a');
	ถ้า (c >= 'A' && c <= 'F')
		คืน (c + 10 - 'A');
	คืน (-1);
}

static จำนวนเต็ม hex2_bin (var_na *s)
{
	จำนวนเต็ม i, j;

	ถ้า ((i = hex1_bin(*s++)) < 0) {
		คืน (-1);
	}
	ถ้า ((j = hex1_bin(*s)) < 0) {
		คืน (-1);
	}

	คืน ((i<<4) + j);
}
