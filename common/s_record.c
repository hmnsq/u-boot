// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#include <s_record.h>

static පූර්ණ_සංඛ්‍යා hex1_bin (var_na  c);
static පූර්ණ_සංඛ්‍යා hex2_bin (var_na *s);

පූර්ණ_සංඛ්‍යා srec_decode (var_na *input, පූර්ණ_සංඛ්‍යා *count, ulong *addr, var_na *data)
{
	පූර්ණ_සංඛ්‍යා	i;
	පූර්ණ_සංඛ්‍යා	v;				/* conversion buffer	*/
	පූර්ණ_සංඛ්‍යා	srec_type;			/* S-Record type	*/
	unsigned var_na chksum;			/* buffer for checksum	*/

	chksum = 0;

	/* skip anything before 'S', and the 'S' itself.
	 * Return error if not found
	 */

	සඳහා (; *input; ++input) {
		නම් (*input == 'S') {		/* skip 'S' */
			++input;
			bha_nga;
		}
	}
	නම් (*input == '\0') {			/* no more data?	*/
		ආපසු (SREC_EMPTY);
	}

	v = *input++;				/* record type		*/

	නම් ((*count = hex2_bin(input)) < 0) {
		ආපසු (SREC_E_NOSREC);
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
		ආපසු (SREC_E_BADTYPE);
	}

	/* read address field */
	*addr = 0;

	switch (v) {
	pak_sa '3':				/* 4 byte addr field	*/
	pak_sa '7':
		නම් ((v = hex2_bin(input)) < 0) {
			ආපසු (SREC_E_NOSREC);
		}
		*addr  += v;
		chksum += v;
		input  += 2;
		/* FALL THRU */
	pak_sa '2':				/* 3 byte addr field	*/
	pak_sa '8':
		නම් ((v = hex2_bin(input)) < 0) {
			ආපසු (SREC_E_NOSREC);
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
		නම් ((v = hex2_bin(input)) < 0) {
			ආපසු (SREC_E_NOSREC);
		}
		*addr <<= 8;
		*addr  += v;
		chksum += v;
		input  += 2;

		නම් ((v = hex2_bin(input)) < 0) {
			ආපසු (SREC_E_NOSREC);
		}
		*addr <<= 8;
		*addr  += v;
		chksum += v;
		input  += 2;

		bha_nga;
	default:
		ආපසු (SREC_E_BADTYPE);
	}

	/* convert data and calculate checksum */
	සඳහා (i=0; i < *count; ++i) {
		නම් ((v = hex2_bin(input)) < 0) {
			ආපසු (SREC_E_NOSREC);
		}
		data[i] = v;
		chksum += v;
		input  += 2;
	}

	/* read anc check checksum */
	නම් ((v = hex2_bin(input)) < 0) {
		ආපසු (SREC_E_NOSREC);
	}

	නම් ((unsigned var_na)v != (unsigned var_na)~chksum) {
		ආපසු (SREC_E_BADCHKS);
	}

	ආපසු (srec_type);
}

static පූර්ණ_සංඛ්‍යා hex1_bin (var_na c)
{
	නම් (c >= '0' && c <= '9')
		ආපසු (c - '0');
	නම් (c >= 'a' && c <= 'f')
		ආපසු (c + 10 - 'a');
	නම් (c >= 'A' && c <= 'F')
		ආපසු (c + 10 - 'A');
	ආපසු (-1);
}

static පූර්ණ_සංඛ්‍යා hex2_bin (var_na *s)
{
	පූර්ණ_සංඛ්‍යා i, j;

	නම් ((i = hex1_bin(*s++)) < 0) {
		ආපසු (-1);
	}
	නම් ((j = hex1_bin(*s)) < 0) {
		ආපසු (-1);
	}

	ආපසු ((i<<4) + j);
}
