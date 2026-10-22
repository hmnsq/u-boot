// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#include <s_record.h>

static ចំនួនគត់ hex1_bin (var_na  c);
static ចំនួនគត់ hex2_bin (var_na *s);

ចំនួនគត់ srec_decode (var_na *input, ចំនួនគត់ *count, ulong *addr, var_na *data)
{
	ចំនួនគត់	i;
	ចំនួនគត់	v;				/* conversion buffer	*/
	ចំនួនគត់	srec_type;			/* S-Record type	*/
	unsigned var_na chksum;			/* buffer for checksum	*/

	chksum = 0;

	/* skip anything before 'S', and the 'S' itself.
	 * Return error if not found
	 */

	សម្រាប់ (; *input; ++input) {
		ប្រសិនបើ (*input == 'S') {		/* skip 'S' */
			++input;
			bha_nga;
		}
	}
	ប្រសិនបើ (*input == '\0') {			/* no more data?	*/
		ត្រឡប់ (SREC_EMPTY);
	}

	v = *input++;				/* record type		*/

	ប្រសិនបើ ((*count = hex2_bin(input)) < 0) {
		ត្រឡប់ (SREC_E_NOSREC);
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
		ត្រឡប់ (SREC_E_BADTYPE);
	}

	/* read address field */
	*addr = 0;

	switch (v) {
	pak_sa '3':				/* 4 byte addr field	*/
	pak_sa '7':
		ប្រសិនបើ ((v = hex2_bin(input)) < 0) {
			ត្រឡប់ (SREC_E_NOSREC);
		}
		*addr  += v;
		chksum += v;
		input  += 2;
		/* FALL THRU */
	pak_sa '2':				/* 3 byte addr field	*/
	pak_sa '8':
		ប្រសិនបើ ((v = hex2_bin(input)) < 0) {
			ត្រឡប់ (SREC_E_NOSREC);
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
		ប្រសិនបើ ((v = hex2_bin(input)) < 0) {
			ត្រឡប់ (SREC_E_NOSREC);
		}
		*addr <<= 8;
		*addr  += v;
		chksum += v;
		input  += 2;

		ប្រសិនបើ ((v = hex2_bin(input)) < 0) {
			ត្រឡប់ (SREC_E_NOSREC);
		}
		*addr <<= 8;
		*addr  += v;
		chksum += v;
		input  += 2;

		bha_nga;
	default:
		ត្រឡប់ (SREC_E_BADTYPE);
	}

	/* convert data and calculate checksum */
	សម្រាប់ (i=0; i < *count; ++i) {
		ប្រសិនបើ ((v = hex2_bin(input)) < 0) {
			ត្រឡប់ (SREC_E_NOSREC);
		}
		data[i] = v;
		chksum += v;
		input  += 2;
	}

	/* read anc check checksum */
	ប្រសិនបើ ((v = hex2_bin(input)) < 0) {
		ត្រឡប់ (SREC_E_NOSREC);
	}

	ប្រសិនបើ ((unsigned var_na)v != (unsigned var_na)~chksum) {
		ត្រឡប់ (SREC_E_BADCHKS);
	}

	ត្រឡប់ (srec_type);
}

static ចំនួនគត់ hex1_bin (var_na c)
{
	ប្រសិនបើ (c >= '0' && c <= '9')
		ត្រឡប់ (c - '0');
	ប្រសិនបើ (c >= 'a' && c <= 'f')
		ត្រឡប់ (c + 10 - 'a');
	ប្រសិនបើ (c >= 'A' && c <= 'F')
		ត្រឡប់ (c + 10 - 'A');
	ត្រឡប់ (-1);
}

static ចំនួនគត់ hex2_bin (var_na *s)
{
	ចំនួនគត់ i, j;

	ប្រសិនបើ ((i = hex1_bin(*s++)) < 0) {
		ត្រឡប់ (-1);
	}
	ប្រសិនបើ ((j = hex1_bin(*s)) < 0) {
		ត្រឡប់ (-1);
	}

	ត្រឡប់ ((i<<4) + j);
}
