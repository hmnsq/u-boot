// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#అన్తర్భావ <s_record.h>

స్థిర పూర్ణాంకం hex1_bin (అక్షరం  c);
స్థిర పూర్ణాంకం hex2_bin (అక్షరం *s);

పూర్ణాంకం srec_decode (అక్షరం *ఆదానం, పూర్ణాంకం *లెక్క, ulong *addr, అక్షరం *దత్తాంశం)
{
	పూర్ణాంకం	i;
	పూర్ణాంకం	v;				/* conversion buffer	*/
	పూర్ణాంకం	srec_type;			/* S-Record type	*/
	గుర్తులేని అక్షరం chksum;			/* buffer for checksum	*/

	chksum = 0;

	/* skip anything before 'S', and the 'S' itself.
	 * Return error if not found
	 */

	కొరకు (; *ఆదానం; ++ఆదానం) {
		ఒకవేళ (*ఆదానం == 'S') {		/* skip 'S' */
			++ఆదానం;
			ఆపు;
		}
	}
	ఒకవేళ (*ఆదానం == '\0') {			/* no more data?	*/
		తిరిగి (SREC_ఖాళీలు);
	}

	v = *ఆదానం++;				/* record type		*/

	ఒకవేళ ((*లెక్క = hex2_bin(ఆదానం)) < 0) {
		తిరిగి (SREC_E_NOSREC);
	}

	chksum += *లెక్క;
	ఆదానం  += 2;

	ఎంపిక (v) {				/* record type		*/

	స్థితి '0':				/* start record		*/
		srec_type = SREC_ప్రారంభం;		/* 2 byte addr field	*/
		*లెక్క   -= 3;			/* - checksum and addr	*/
		ఆపు;
	స్థితి '1':
		srec_type = SREC_DATA2;		/* 2 byte addr field	*/
		*లెక్క   -= 3;			/* - checksum and addr	*/
		ఆపు;
	స్థితి '2':
		srec_type = SREC_DATA3;		/* 3 byte addr field	*/
		*లెక్క   -= 4;			/* - checksum and addr	*/
		ఆపు;
	స్థితి '3':				/* data record with a	*/
		srec_type = SREC_DATA4;		/* 4 byte addr field	*/
		*లెక్క   -= 5;			/* - checksum and addr	*/
		ఆపు;
/***	case '4'  ***/
	స్థితి '5':			/* count record, addr field contains */
		srec_type = SREC_లెక్క;	/* a 2 byte record counter	*/
		*లెక్క    = 0;			/* no data		*/
		ఆపు;
/***	case '6' -- not used  ***/
	స్థితి '7':				/* end record with a	*/
		srec_type = SREC_END4;		/* 4 byte addr field	*/
		*లెక్క   -= 5;			/* - checksum and addr	*/
		ఆపు;
	స్థితి '8':				/* end record with a	*/
		srec_type = SREC_END3;		/* 3 byte addr field	*/
		*లెక్క   -= 4;			/* - checksum and addr	*/
		ఆపు;
	స్థితి '9':				/* end record with a	*/
		srec_type = SREC_END2;		/* 2 byte addr field	*/
		*లెక్క   -= 3;			/* - checksum and addr	*/
		ఆపు;
	అప్రమేయ:
		తిరిగి (SREC_E_BADTYPE);
	}

	/* read address field */
	*addr = 0;

	ఎంపిక (v) {
	స్థితి '3':				/* 4 byte addr field	*/
	స్థితి '7':
		ఒకవేళ ((v = hex2_bin(ఆదానం)) < 0) {
			తిరిగి (SREC_E_NOSREC);
		}
		*addr  += v;
		chksum += v;
		ఆదానం  += 2;
		/* FALL THRU */
	స్థితి '2':				/* 3 byte addr field	*/
	స్థితి '8':
		ఒకవేళ ((v = hex2_bin(ఆదానం)) < 0) {
			తిరిగి (SREC_E_NOSREC);
		}
		*addr <<= 8;
		*addr  += v;
		chksum += v;
		ఆదానం  += 2;
		/* FALL THRU */
	స్థితి '0':				/* 2 byte addr field	*/
	స్థితి '1':
	స్థితి '5':
	స్థితి '9':
		ఒకవేళ ((v = hex2_bin(ఆదానం)) < 0) {
			తిరిగి (SREC_E_NOSREC);
		}
		*addr <<= 8;
		*addr  += v;
		chksum += v;
		ఆదానం  += 2;

		ఒకవేళ ((v = hex2_bin(ఆదానం)) < 0) {
			తిరిగి (SREC_E_NOSREC);
		}
		*addr <<= 8;
		*addr  += v;
		chksum += v;
		ఆదానం  += 2;

		ఆపు;
	అప్రమేయ:
		తిరిగి (SREC_E_BADTYPE);
	}

	/* convert data and calculate checksum */
	కొరకు (i=0; i < *లెక్క; ++i) {
		ఒకవేళ ((v = hex2_bin(ఆదానం)) < 0) {
			తిరిగి (SREC_E_NOSREC);
		}
		దత్తాంశం[i] = v;
		chksum += v;
		ఆదానం  += 2;
	}

	/* read anc check checksum */
	ఒకవేళ ((v = hex2_bin(ఆదానం)) < 0) {
		తిరిగి (SREC_E_NOSREC);
	}

	ఒకవేళ ((గుర్తులేని అక్షరం)v != (గుర్తులేని అక్షరం)~chksum) {
		తిరిగి (SREC_E_BADCHKS);
	}

	తిరిగి (srec_type);
}

స్థిర పూర్ణాంకం hex1_bin (అక్షరం c)
{
	ఒకవేళ (c >= '0' && c <= '9')
		తిరిగి (c - '0');
	ఒకవేళ (c >= 'a' && c <= 'f')
		తిరిగి (c + 10 - 'a');
	ఒకవేళ (c >= 'A' && c <= 'F')
		తిరిగి (c + 10 - 'A');
	తిరిగి (-1);
}

స్థిర పూర్ణాంకం hex2_bin (అక్షరం *s)
{
	పూర్ణాంకం i, j;

	ఒకవేళ ((i = hex1_bin(*s++)) < 0) {
		తిరిగి (-1);
	}
	ఒకవేళ ((j = hex1_bin(*s)) < 0) {
		తిరిగి (-1);
	}

	తిరిగి ((i<<4) + j);
}
