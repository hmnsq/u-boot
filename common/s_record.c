// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#অন্তর্ভাব <s_record.h>

স্থির পূর্ণসংখ্যা hex1_bin (অক্ষর  c);
স্থির পূর্ণসংখ্যা hex2_bin (অক্ষর *s);

পূর্ণসংখ্যা srec_decode (অক্ষর *প্রবেশ, পূর্ণসংখ্যা *গুনতি, ulong *addr, অক্ষর *তথ্য)
{
	পূর্ণসংখ্যা	i;
	পূর্ণসংখ্যা	v;				/* conversion buffer	*/
	পূর্ণসংখ্যা	srec_type;			/* S-Record type	*/
	unsigned অক্ষর chksum;			/* buffer for checksum	*/

	chksum = 0;

	/* skip anything before 'S', and the 'S' itself.
	 * Return error if not found
	 */

	জন্য (; *প্রবেশ; ++প্রবেশ) {
		যদি (*প্রবেশ == 'S') {		/* skip 'S' */
			++প্রবেশ;
			বিরতি;
		}
	}
	যদি (*প্রবেশ == '\0') {			/* no more data?	*/
		ফেরত (SREC_EMPTY);
	}

	v = *প্রবেশ++;				/* record type		*/

	যদি ((*গুনতি = hex2_bin(প্রবেশ)) < 0) {
		ফেরত (SREC_E_NOSREC);
	}

	chksum += *গুনতি;
	প্রবেশ  += 2;

	নির্বাচন (v) {				/* record type		*/

	ক্ষেত্র '0':				/* start record		*/
		srec_type = SREC_START;		/* 2 byte addr field	*/
		*গুনতি   -= 3;			/* - checksum and addr	*/
		বিরতি;
	ক্ষেত্র '1':
		srec_type = SREC_DATA2;		/* 2 byte addr field	*/
		*গুনতি   -= 3;			/* - checksum and addr	*/
		বিরতি;
	ক্ষেত্র '2':
		srec_type = SREC_DATA3;		/* 3 byte addr field	*/
		*গুনতি   -= 4;			/* - checksum and addr	*/
		বিরতি;
	ক্ষেত্র '3':				/* data record with a	*/
		srec_type = SREC_DATA4;		/* 4 byte addr field	*/
		*গুনতি   -= 5;			/* - checksum and addr	*/
		বিরতি;
/***	case '4'  ***/
	ক্ষেত্র '5':			/* count record, addr field contains */
		srec_type = SREC_গুনতি;	/* a 2 byte record counter	*/
		*গুনতি    = 0;			/* no data		*/
		বিরতি;
/***	case '6' -- not used  ***/
	ক্ষেত্র '7':				/* end record with a	*/
		srec_type = SREC_END4;		/* 4 byte addr field	*/
		*গুনতি   -= 5;			/* - checksum and addr	*/
		বিরতি;
	ক্ষেত্র '8':				/* end record with a	*/
		srec_type = SREC_END3;		/* 3 byte addr field	*/
		*গুনতি   -= 4;			/* - checksum and addr	*/
		বিরতি;
	ক্ষেত্র '9':				/* end record with a	*/
		srec_type = SREC_END2;		/* 2 byte addr field	*/
		*গুনতি   -= 3;			/* - checksum and addr	*/
		বিরতি;
	পূর্বনির্ধারিত:
		ফেরত (SREC_E_BADTYPE);
	}

	/* read address field */
	*addr = 0;

	নির্বাচন (v) {
	ক্ষেত্র '3':				/* 4 byte addr field	*/
	ক্ষেত্র '7':
		যদি ((v = hex2_bin(প্রবেশ)) < 0) {
			ফেরত (SREC_E_NOSREC);
		}
		*addr  += v;
		chksum += v;
		প্রবেশ  += 2;
		/* FALL THRU */
	ক্ষেত্র '2':				/* 3 byte addr field	*/
	ক্ষেত্র '8':
		যদি ((v = hex2_bin(প্রবেশ)) < 0) {
			ফেরত (SREC_E_NOSREC);
		}
		*addr <<= 8;
		*addr  += v;
		chksum += v;
		প্রবেশ  += 2;
		/* FALL THRU */
	ক্ষেত্র '0':				/* 2 byte addr field	*/
	ক্ষেত্র '1':
	ক্ষেত্র '5':
	ক্ষেত্র '9':
		যদি ((v = hex2_bin(প্রবেশ)) < 0) {
			ফেরত (SREC_E_NOSREC);
		}
		*addr <<= 8;
		*addr  += v;
		chksum += v;
		প্রবেশ  += 2;

		যদি ((v = hex2_bin(প্রবেশ)) < 0) {
			ফেরত (SREC_E_NOSREC);
		}
		*addr <<= 8;
		*addr  += v;
		chksum += v;
		প্রবেশ  += 2;

		বিরতি;
	পূর্বনির্ধারিত:
		ফেরত (SREC_E_BADTYPE);
	}

	/* convert data and calculate checksum */
	জন্য (i=0; i < *গুনতি; ++i) {
		যদি ((v = hex2_bin(প্রবেশ)) < 0) {
			ফেরত (SREC_E_NOSREC);
		}
		তথ্য[i] = v;
		chksum += v;
		প্রবেশ  += 2;
	}

	/* read anc check checksum */
	যদি ((v = hex2_bin(প্রবেশ)) < 0) {
		ফেরত (SREC_E_NOSREC);
	}

	যদি ((unsigned অক্ষর)v != (unsigned অক্ষর)~chksum) {
		ফেরত (SREC_E_BADCHKS);
	}

	ফেরত (srec_type);
}

স্থির পূর্ণসংখ্যা hex1_bin (অক্ষর c)
{
	যদি (c >= '0' && c <= '9')
		ফেরত (c - '0');
	যদি (c >= 'a' && c <= 'f')
		ফেরত (c + 10 - 'a');
	যদি (c >= 'A' && c <= 'F')
		ফেরত (c + 10 - 'A');
	ফেরত (-1);
}

স্থির পূর্ণসংখ্যা hex2_bin (অক্ষর *s)
{
	পূর্ণসংখ্যা i, j;

	যদি ((i = hex1_bin(*s++)) < 0) {
		ফেরত (-1);
	}
	যদি ((j = hex1_bin(*s)) < 0) {
		ফেরত (-1);
	}

	ফেরত ((i<<4) + j);
}
