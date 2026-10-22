// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#অন্তর্ভাৱ <s_record.h>

স্থির পূর্ণাঙ্ক hex1_bin (ৱর্ণ  c);
স্থির পূর্ণাঙ্ক hex2_bin (ৱর্ণ *s);

পূর্ণাঙ্ক srec_decode (ৱর্ণ *প্ৰবেশ, পূর্ণাঙ্ক *গুনতি, ulong *addr, ৱর্ণ *data)
{
	পূর্ণাঙ্ক	i;
	পূর্ণাঙ্ক	v;				/* conversion buffer	*/
	পূর্ণাঙ্ক	srec_type;			/* S-Record type	*/
	unsigned ৱর্ণ chksum;			/* buffer for checksum	*/

	chksum = 0;

	/* skip anything before 'S', and the 'S' itself.
	 * Return error if not found
	 */

	বাবে (; *প্ৰবেশ; ++প্ৰবেশ) {
		যদি (*প্ৰবেশ == 'S') {		/* skip 'S' */
			++প্ৰবেশ;
			ভঙ্গ;
		}
	}
	যদি (*প্ৰবেশ == '\0') {			/* no more data?	*/
		ঘূৰাই (SREC_EMPTY);
	}

	v = *প্ৰবেশ++;				/* record type		*/

	যদি ((*গুনতি = hex2_bin(প্ৰবেশ)) < 0) {
		ঘূৰাই (SREC_E_NOSREC);
	}

	chksum += *গুনতি;
	প্ৰবেশ  += 2;

	ৱিকল্প (v) {				/* record type		*/

	পক্ষ '0':				/* start record		*/
		srec_type = SREC_START;		/* 2 byte addr field	*/
		*গুনতি   -= 3;			/* - checksum and addr	*/
		ভঙ্গ;
	পক্ষ '1':
		srec_type = SREC_DATA2;		/* 2 byte addr field	*/
		*গুনতি   -= 3;			/* - checksum and addr	*/
		ভঙ্গ;
	পক্ষ '2':
		srec_type = SREC_DATA3;		/* 3 byte addr field	*/
		*গুনতি   -= 4;			/* - checksum and addr	*/
		ভঙ্গ;
	পক্ষ '3':				/* data record with a	*/
		srec_type = SREC_DATA4;		/* 4 byte addr field	*/
		*গুনতি   -= 5;			/* - checksum and addr	*/
		ভঙ্গ;
/***	case '4'  ***/
	পক্ষ '5':			/* count record, addr field contains */
		srec_type = SREC_গুনতি;	/* a 2 byte record counter	*/
		*গুনতি    = 0;			/* no data		*/
		ভঙ্গ;
/***	case '6' -- not used  ***/
	পক্ষ '7':				/* end record with a	*/
		srec_type = SREC_END4;		/* 4 byte addr field	*/
		*গুনতি   -= 5;			/* - checksum and addr	*/
		ভঙ্গ;
	পক্ষ '8':				/* end record with a	*/
		srec_type = SREC_END3;		/* 3 byte addr field	*/
		*গুনতি   -= 4;			/* - checksum and addr	*/
		ভঙ্গ;
	পক্ষ '9':				/* end record with a	*/
		srec_type = SREC_END2;		/* 2 byte addr field	*/
		*গুনতি   -= 3;			/* - checksum and addr	*/
		ভঙ্গ;
	স্ৱভাৱ:
		ঘূৰাই (SREC_E_BADTYPE);
	}

	/* read address field */
	*addr = 0;

	ৱিকল্প (v) {
	পক্ষ '3':				/* 4 byte addr field	*/
	পক্ষ '7':
		যদি ((v = hex2_bin(প্ৰবেশ)) < 0) {
			ঘূৰাই (SREC_E_NOSREC);
		}
		*addr  += v;
		chksum += v;
		প্ৰবেশ  += 2;
		/* FALL THRU */
	পক্ষ '2':				/* 3 byte addr field	*/
	পক্ষ '8':
		যদি ((v = hex2_bin(প্ৰবেশ)) < 0) {
			ঘূৰাই (SREC_E_NOSREC);
		}
		*addr <<= 8;
		*addr  += v;
		chksum += v;
		প্ৰবেশ  += 2;
		/* FALL THRU */
	পক্ষ '0':				/* 2 byte addr field	*/
	পক্ষ '1':
	পক্ষ '5':
	পক্ষ '9':
		যদি ((v = hex2_bin(প্ৰবেশ)) < 0) {
			ঘূৰাই (SREC_E_NOSREC);
		}
		*addr <<= 8;
		*addr  += v;
		chksum += v;
		প্ৰবেশ  += 2;

		যদি ((v = hex2_bin(প্ৰবেশ)) < 0) {
			ঘূৰাই (SREC_E_NOSREC);
		}
		*addr <<= 8;
		*addr  += v;
		chksum += v;
		প্ৰবেশ  += 2;

		ভঙ্গ;
	স্ৱভাৱ:
		ঘূৰাই (SREC_E_BADTYPE);
	}

	/* convert data and calculate checksum */
	বাবে (i=0; i < *গুনতি; ++i) {
		যদি ((v = hex2_bin(প্ৰবেশ)) < 0) {
			ঘূৰাই (SREC_E_NOSREC);
		}
		data[i] = v;
		chksum += v;
		প্ৰবেশ  += 2;
	}

	/* read anc check checksum */
	যদি ((v = hex2_bin(প্ৰবেশ)) < 0) {
		ঘূৰাই (SREC_E_NOSREC);
	}

	যদি ((unsigned ৱর্ণ)v != (unsigned ৱর্ণ)~chksum) {
		ঘূৰাই (SREC_E_BADCHKS);
	}

	ঘূৰাই (srec_type);
}

স্থির পূর্ণাঙ্ক hex1_bin (ৱর্ণ c)
{
	যদি (c >= '0' && c <= '9')
		ঘূৰাই (c - '0');
	যদি (c >= 'a' && c <= 'f')
		ঘূৰাই (c + 10 - 'a');
	যদি (c >= 'A' && c <= 'F')
		ঘূৰাই (c + 10 - 'A');
	ঘূৰাই (-1);
}

স্থির পূর্ণাঙ্ক hex2_bin (ৱর্ণ *s)
{
	পূর্ণাঙ্ক i, j;

	যদি ((i = hex1_bin(*s++)) < 0) {
		ঘূৰাই (-1);
	}
	যদি ((j = hex1_bin(*s)) < 0) {
		ঘূৰাই (-1);
	}

	ঘূৰাই ((i<<4) + j);
}
