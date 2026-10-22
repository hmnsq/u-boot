// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#include <s_record.h>

static ကိန်းပြည့် hex1_bin (var_na  c);
static ကိန်းပြည့် hex2_bin (var_na *s);

ကိန်းပြည့် srec_decode (var_na *input, ကိန်းပြည့် *count, ulong *addr, var_na *data)
{
	ကိန်းပြည့်	i;
	ကိန်းပြည့်	v;				/* conversion buffer	*/
	ကိန်းပြည့်	srec_type;			/* S-Record type	*/
	unsigned var_na chksum;			/* buffer for checksum	*/

	chksum = 0;

	/* skip anything before 'S', and the 'S' itself.
	 * Return error if not found
	 */

	အတွက် (; *input; ++input) {
		အကယ်၍ (*input == 'S') {		/* skip 'S' */
			++input;
			bha_nga;
		}
	}
	အကယ်၍ (*input == '\0') {			/* no more data?	*/
		ပြန်ပေး (SREC_EMPTY);
	}

	v = *input++;				/* record type		*/

	အကယ်၍ ((*count = hex2_bin(input)) < 0) {
		ပြန်ပေး (SREC_E_NOSREC);
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
		ပြန်ပေး (SREC_E_BADTYPE);
	}

	/* read address field */
	*addr = 0;

	switch (v) {
	pak_sa '3':				/* 4 byte addr field	*/
	pak_sa '7':
		အကယ်၍ ((v = hex2_bin(input)) < 0) {
			ပြန်ပေး (SREC_E_NOSREC);
		}
		*addr  += v;
		chksum += v;
		input  += 2;
		/* FALL THRU */
	pak_sa '2':				/* 3 byte addr field	*/
	pak_sa '8':
		အကယ်၍ ((v = hex2_bin(input)) < 0) {
			ပြန်ပေး (SREC_E_NOSREC);
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
		အကယ်၍ ((v = hex2_bin(input)) < 0) {
			ပြန်ပေး (SREC_E_NOSREC);
		}
		*addr <<= 8;
		*addr  += v;
		chksum += v;
		input  += 2;

		အကယ်၍ ((v = hex2_bin(input)) < 0) {
			ပြန်ပေး (SREC_E_NOSREC);
		}
		*addr <<= 8;
		*addr  += v;
		chksum += v;
		input  += 2;

		bha_nga;
	default:
		ပြန်ပေး (SREC_E_BADTYPE);
	}

	/* convert data and calculate checksum */
	အတွက် (i=0; i < *count; ++i) {
		အကယ်၍ ((v = hex2_bin(input)) < 0) {
			ပြန်ပေး (SREC_E_NOSREC);
		}
		data[i] = v;
		chksum += v;
		input  += 2;
	}

	/* read anc check checksum */
	အကယ်၍ ((v = hex2_bin(input)) < 0) {
		ပြန်ပေး (SREC_E_NOSREC);
	}

	အကယ်၍ ((unsigned var_na)v != (unsigned var_na)~chksum) {
		ပြန်ပေး (SREC_E_BADCHKS);
	}

	ပြန်ပေး (srec_type);
}

static ကိန်းပြည့် hex1_bin (var_na c)
{
	အကယ်၍ (c >= '0' && c <= '9')
		ပြန်ပေး (c - '0');
	အကယ်၍ (c >= 'a' && c <= 'f')
		ပြန်ပေး (c + 10 - 'a');
	အကယ်၍ (c >= 'A' && c <= 'F')
		ပြန်ပေး (c + 10 - 'A');
	ပြန်ပေး (-1);
}

static ကိန်းပြည့် hex2_bin (var_na *s)
{
	ကိန်းပြည့် i, j;

	အကယ်၍ ((i = hex1_bin(*s++)) < 0) {
		ပြန်ပေး (-1);
	}
	အကယ်၍ ((j = hex1_bin(*s)) < 0) {
		ပြန်ပေး (-1);
	}

	ပြန်ပေး ((i<<4) + j);
}
