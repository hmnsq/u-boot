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
		ܐܢ (*input == 'S') {		/* skip 'S' */
			++input;
			اكسر;
		}
	}
	ܐܢ (*input == '\0') {			/* no more data?	*/
		ܦܢܐ (SREC_EMPTY);
	}

	v = *input++;				/* record type		*/

	ܐܢ ((*count = hex2_bin(input)) < 0) {
		ܦܢܐ (SREC_E_NOSREC);
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
		ܦܢܐ (SREC_E_BADTYPE);
	}

	/* read address field */
	*addr = 0;

	بدل (v) {
	حالة '3':				/* 4 byte addr field	*/
	حالة '7':
		ܐܢ ((v = hex2_bin(input)) < 0) {
			ܦܢܐ (SREC_E_NOSREC);
		}
		*addr  += v;
		chksum += v;
		input  += 2;
		/* FALL THRU */
	حالة '2':				/* 3 byte addr field	*/
	حالة '8':
		ܐܢ ((v = hex2_bin(input)) < 0) {
			ܦܢܐ (SREC_E_NOSREC);
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
		ܐܢ ((v = hex2_bin(input)) < 0) {
			ܦܢܐ (SREC_E_NOSREC);
		}
		*addr <<= 8;
		*addr  += v;
		chksum += v;
		input  += 2;

		ܐܢ ((v = hex2_bin(input)) < 0) {
			ܦܢܐ (SREC_E_NOSREC);
		}
		*addr <<= 8;
		*addr  += v;
		chksum += v;
		input  += 2;

		اكسر;
	افتراضي:
		ܦܢܐ (SREC_E_BADTYPE);
	}

	/* convert data and calculate checksum */
	لكل (i=0; i < *count; ++i) {
		ܐܢ ((v = hex2_bin(input)) < 0) {
			ܦܢܐ (SREC_E_NOSREC);
		}
		data[i] = v;
		chksum += v;
		input  += 2;
	}

	/* read anc check checksum */
	ܐܢ ((v = hex2_bin(input)) < 0) {
		ܦܢܐ (SREC_E_NOSREC);
	}

	ܐܢ ((unsigned حرف)v != (unsigned حرف)~chksum) {
		ܦܢܐ (SREC_E_BADCHKS);
	}

	ܦܢܐ (srec_type);
}

ساكن صحيح hex1_bin (حرف c)
{
	ܐܢ (c >= '0' && c <= '9')
		ܦܢܐ (c - '0');
	ܐܢ (c >= 'a' && c <= 'f')
		ܦܢܐ (c + 10 - 'a');
	ܐܢ (c >= 'A' && c <= 'F')
		ܦܢܐ (c + 10 - 'A');
	ܦܢܐ (-1);
}

ساكن صحيح hex2_bin (حرف *s)
{
	صحيح i, j;

	ܐܢ ((i = hex1_bin(*s++)) < 0) {
		ܦܢܐ (-1);
	}
	ܐܢ ((j = hex1_bin(*s)) < 0) {
		ܦܢܐ (-1);
	}

	ܦܢܐ ((i<<4) + j);
}
