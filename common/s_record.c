// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#ضمن <s_record.h>

ساكن صحيح hex1_bin (حرف  c);
ساكن صحيح hex2_bin (حرف *s);

صحيح srec_decode (حرف *input, صحيح *count, ulong *addr, حرف *data)
{
	صحيح	i;
	صحيح	v;				/* conversion buffer	*/
	صحيح	srec_type;			/* S-Record type	*/
	unsigned حرف chksum;			/* buffer for checksum	*/

	chksum = 0;

	/* skip anything before 'S', and the 'S' itself.
	 * Return error if not found
	 */

	لكل (; *input; ++input) {
		𐡄𐡍 (*input == 'S') {		/* skip 'S' */
			++input;
			اكسر;
		}
	}
	𐡄𐡍 (*input == '\0') {			/* no more data?	*/
		أرجع (SREC_EMPTY);
	}

	v = *input++;				/* record type		*/

	𐡄𐡍 ((*count = hex2_bin(input)) < 0) {
		أرجع (SREC_E_NOSREC);
	}

	chksum += *count;
	input  += 2;

	بدل (v) {				/* record type		*/

	حالة '0':				/* start record		*/
		srec_type = SREC_START;		/* 2 byte addr field	*/
		*count   -= 3;			/* - checksum and addr	*/
		اكسر;
	حالة '1':
		srec_type = SREC_DATA2;		/* 2 byte addr field	*/
		*count   -= 3;			/* - checksum and addr	*/
		اكسر;
	حالة '2':
		srec_type = SREC_DATA3;		/* 3 byte addr field	*/
		*count   -= 4;			/* - checksum and addr	*/
		اكسر;
	حالة '3':				/* data record with a	*/
		srec_type = SREC_DATA4;		/* 4 byte addr field	*/
		*count   -= 5;			/* - checksum and addr	*/
		اكسر;
/***	case '4'  ***/
	حالة '5':			/* count record, addr field contains */
		srec_type = SREC_COUNT;	/* a 2 byte record counter	*/
		*count    = 0;			/* no data		*/
		اكسر;
/***	case '6' -- not used  ***/
	حالة '7':				/* end record with a	*/
		srec_type = SREC_END4;		/* 4 byte addr field	*/
		*count   -= 5;			/* - checksum and addr	*/
		اكسر;
	حالة '8':				/* end record with a	*/
		srec_type = SREC_END3;		/* 3 byte addr field	*/
		*count   -= 4;			/* - checksum and addr	*/
		اكسر;
	حالة '9':				/* end record with a	*/
		srec_type = SREC_END2;		/* 2 byte addr field	*/
		*count   -= 3;			/* - checksum and addr	*/
		اكسر;
	افتراضي:
		أرجع (SREC_E_BADTYPE);
	}

	/* read address field */
	*addr = 0;

	بدل (v) {
	حالة '3':				/* 4 byte addr field	*/
	حالة '7':
		𐡄𐡍 ((v = hex2_bin(input)) < 0) {
			أرجع (SREC_E_NOSREC);
		}
		*addr  += v;
		chksum += v;
		input  += 2;
		/* FALL THRU */
	حالة '2':				/* 3 byte addr field	*/
	حالة '8':
		𐡄𐡍 ((v = hex2_bin(input)) < 0) {
			أرجع (SREC_E_NOSREC);
		}
		*addr <<= 8;
		*addr  += v;
		chksum += v;
		input  += 2;
		/* FALL THRU */
	حالة '0':				/* 2 byte addr field	*/
	حالة '1':
	حالة '5':
	حالة '9':
		𐡄𐡍 ((v = hex2_bin(input)) < 0) {
			أرجع (SREC_E_NOSREC);
		}
		*addr <<= 8;
		*addr  += v;
		chksum += v;
		input  += 2;

		𐡄𐡍 ((v = hex2_bin(input)) < 0) {
			أرجع (SREC_E_NOSREC);
		}
		*addr <<= 8;
		*addr  += v;
		chksum += v;
		input  += 2;

		اكسر;
	افتراضي:
		أرجع (SREC_E_BADTYPE);
	}

	/* convert data and calculate checksum */
	لكل (i=0; i < *count; ++i) {
		𐡄𐡍 ((v = hex2_bin(input)) < 0) {
			أرجع (SREC_E_NOSREC);
		}
		data[i] = v;
		chksum += v;
		input  += 2;
	}

	/* read anc check checksum */
	𐡄𐡍 ((v = hex2_bin(input)) < 0) {
		أرجع (SREC_E_NOSREC);
	}

	𐡄𐡍 ((unsigned حرف)v != (unsigned حرف)~chksum) {
		أرجع (SREC_E_BADCHKS);
	}

	أرجع (srec_type);
}

ساكن صحيح hex1_bin (حرف c)
{
	𐡄𐡍 (c >= '0' && c <= '9')
		أرجع (c - '0');
	𐡄𐡍 (c >= 'a' && c <= 'f')
		أرجع (c + 10 - 'a');
	𐡄𐡍 (c >= 'A' && c <= 'F')
		أرجع (c + 10 - 'A');
	أرجع (-1);
}

ساكن صحيح hex2_bin (حرف *s)
{
	صحيح i, j;

	𐡄𐡍 ((i = hex1_bin(*s++)) < 0) {
		أرجع (-1);
	}
	𐡄𐡍 ((j = hex1_bin(*s)) < 0) {
		أرجع (-1);
	}

	أرجع ((i<<4) + j);
}
