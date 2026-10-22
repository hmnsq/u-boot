// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#અન્તર્ભાવ <s_record.h>

સ્થિર પૂર્ણાંક hex1_bin (અક્ષર  c);
સ્થિર પૂર્ણાંક hex2_bin (અક્ષર *s);

પૂર્ણાંક srec_decode (અક્ષર *આગત, પૂર્ણાંક *ગણતરી, ulong *addr, અક્ષર *data)
{
	પૂર્ણાંક	i;
	પૂર્ણાંક	v;				/* conversion buffer	*/
	પૂર્ણાંક	srec_type;			/* S-Record type	*/
	unsigned અક્ષર chksum;			/* buffer for checksum	*/

	chksum = 0;

	/* skip anything before 'S', and the 'S' itself.
	 * Return error if not found
	 */

	માટે (; *આગત; ++આગત) {
		જો (*આગત == 'S') {		/* skip 'S' */
			++આગત;
			તોડો;
		}
	}
	જો (*આગત == '\0') {			/* no more data?	*/
		પાછા (SREC_EMPTY);
	}

	v = *આગત++;				/* record type		*/

	જો ((*ગણતરી = hex2_bin(આગત)) < 0) {
		પાછા (SREC_E_NOSREC);
	}

	chksum += *ગણતરી;
	આગત  += 2;

	પસંદગી (v) {				/* record type		*/

	સ્થિતિ '0':				/* start record		*/
		srec_type = SREC_START;		/* 2 byte addr field	*/
		*ગણતરી   -= 3;			/* - checksum and addr	*/
		તોડો;
	સ્થિતિ '1':
		srec_type = SREC_DATA2;		/* 2 byte addr field	*/
		*ગણતરી   -= 3;			/* - checksum and addr	*/
		તોડો;
	સ્થિતિ '2':
		srec_type = SREC_DATA3;		/* 3 byte addr field	*/
		*ગણતરી   -= 4;			/* - checksum and addr	*/
		તોડો;
	સ્થિતિ '3':				/* data record with a	*/
		srec_type = SREC_DATA4;		/* 4 byte addr field	*/
		*ગણતરી   -= 5;			/* - checksum and addr	*/
		તોડો;
/***	case '4'  ***/
	સ્થિતિ '5':			/* count record, addr field contains */
		srec_type = SREC_ગણતરી;	/* a 2 byte record counter	*/
		*ગણતરી    = 0;			/* no data		*/
		તોડો;
/***	case '6' -- not used  ***/
	સ્થિતિ '7':				/* end record with a	*/
		srec_type = SREC_END4;		/* 4 byte addr field	*/
		*ગણતરી   -= 5;			/* - checksum and addr	*/
		તોડો;
	સ્થિતિ '8':				/* end record with a	*/
		srec_type = SREC_END3;		/* 3 byte addr field	*/
		*ગણતરી   -= 4;			/* - checksum and addr	*/
		તોડો;
	સ્થિતિ '9':				/* end record with a	*/
		srec_type = SREC_END2;		/* 2 byte addr field	*/
		*ગણતરી   -= 3;			/* - checksum and addr	*/
		તોડો;
	મૂળભૂત:
		પાછા (SREC_E_BADTYPE);
	}

	/* read address field */
	*addr = 0;

	પસંદગી (v) {
	સ્થિતિ '3':				/* 4 byte addr field	*/
	સ્થિતિ '7':
		જો ((v = hex2_bin(આગત)) < 0) {
			પાછા (SREC_E_NOSREC);
		}
		*addr  += v;
		chksum += v;
		આગત  += 2;
		/* FALL THRU */
	સ્થિતિ '2':				/* 3 byte addr field	*/
	સ્થિતિ '8':
		જો ((v = hex2_bin(આગત)) < 0) {
			પાછા (SREC_E_NOSREC);
		}
		*addr <<= 8;
		*addr  += v;
		chksum += v;
		આગત  += 2;
		/* FALL THRU */
	સ્થિતિ '0':				/* 2 byte addr field	*/
	સ્થિતિ '1':
	સ્થિતિ '5':
	સ્થિતિ '9':
		જો ((v = hex2_bin(આગત)) < 0) {
			પાછા (SREC_E_NOSREC);
		}
		*addr <<= 8;
		*addr  += v;
		chksum += v;
		આગત  += 2;

		જો ((v = hex2_bin(આગત)) < 0) {
			પાછા (SREC_E_NOSREC);
		}
		*addr <<= 8;
		*addr  += v;
		chksum += v;
		આગત  += 2;

		તોડો;
	મૂળભૂત:
		પાછા (SREC_E_BADTYPE);
	}

	/* convert data and calculate checksum */
	માટે (i=0; i < *ગણતરી; ++i) {
		જો ((v = hex2_bin(આગત)) < 0) {
			પાછા (SREC_E_NOSREC);
		}
		data[i] = v;
		chksum += v;
		આગત  += 2;
	}

	/* read anc check checksum */
	જો ((v = hex2_bin(આગત)) < 0) {
		પાછા (SREC_E_NOSREC);
	}

	જો ((unsigned અક્ષર)v != (unsigned અક્ષર)~chksum) {
		પાછા (SREC_E_BADCHKS);
	}

	પાછા (srec_type);
}

સ્થિર પૂર્ણાંક hex1_bin (અક્ષર c)
{
	જો (c >= '0' && c <= '9')
		પાછા (c - '0');
	જો (c >= 'a' && c <= 'f')
		પાછા (c + 10 - 'a');
	જો (c >= 'A' && c <= 'F')
		પાછા (c + 10 - 'A');
	પાછા (-1);
}

સ્થિર પૂર્ણાંક hex2_bin (અક્ષર *s)
{
	પૂર્ણાંક i, j;

	જો ((i = hex1_bin(*s++)) < 0) {
		પાછા (-1);
	}
	જો ((j = hex1_bin(*s)) < 0) {
		પાછા (-1);
	}

	પાછા ((i<<4) + j);
}
