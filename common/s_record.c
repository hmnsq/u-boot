// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#अन्तर्भाव <s_record.h>

स्थिर पूर्णाङ्क hex1_bin (वर्ण  c);
स्थिर पूर्णाङ्क hex2_bin (वर्ण *s);

पूर्णाङ्क srec_decode (वर्ण *निवेशः, पूर्णाङ्क *गणनम्, ulong *addr, वर्ण *दत्त)
{
	पूर्णाङ्क	i;
	पूर्णाङ्क	v;				/* conversion buffer	*/
	पूर्णाङ्क	srec_type;			/* S-Record type	*/
	अचिह्नित वर्ण chksum;			/* buffer for checksum	*/

	chksum = 0;

	/* skip anything before 'S', and the 'S' itself.
	 * Return error if not found
	 */

	प्रति (; *निवेशः; ++निवेशः) {
		यदि (*निवेशः == 'S') {		/* skip 'S' */
			++निवेशः;
			भङ्ग;
		}
	}
	यदि (*निवेशः == '\0') {			/* no more data?	*/
		प्रत्यागम (SREC_रिक्तानि);
	}

	v = *निवेशः++;				/* record type		*/

	यदि ((*गणनम् = hex2_bin(निवेशः)) < 0) {
		प्रत्यागम (SREC_E_NOSREC);
	}

	chksum += *गणनम्;
	निवेशः  += 2;

	विकल्प (v) {				/* record type		*/

	पक्ष '0':				/* start record		*/
		srec_type = SREC_START;		/* 2 byte addr field	*/
		*गणनम्   -= 3;			/* - checksum and addr	*/
		भङ्ग;
	पक्ष '1':
		srec_type = SREC_DATA2;		/* 2 byte addr field	*/
		*गणनम्   -= 3;			/* - checksum and addr	*/
		भङ्ग;
	पक्ष '2':
		srec_type = SREC_DATA3;		/* 3 byte addr field	*/
		*गणनम्   -= 4;			/* - checksum and addr	*/
		भङ्ग;
	पक्ष '3':				/* data record with a	*/
		srec_type = SREC_DATA4;		/* 4 byte addr field	*/
		*गणनम्   -= 5;			/* - checksum and addr	*/
		भङ्ग;
/***	case '4'  ***/
	पक्ष '5':			/* count record, addr field contains */
		srec_type = SREC_गणनम्;	/* a 2 byte record counter	*/
		*गणनम्    = 0;			/* no data		*/
		भङ्ग;
/***	case '6' -- not used  ***/
	पक्ष '7':				/* end record with a	*/
		srec_type = SREC_END4;		/* 4 byte addr field	*/
		*गणनम्   -= 5;			/* - checksum and addr	*/
		भङ्ग;
	पक्ष '8':				/* end record with a	*/
		srec_type = SREC_END3;		/* 3 byte addr field	*/
		*गणनम्   -= 4;			/* - checksum and addr	*/
		भङ्ग;
	पक्ष '9':				/* end record with a	*/
		srec_type = SREC_END2;		/* 2 byte addr field	*/
		*गणनम्   -= 3;			/* - checksum and addr	*/
		भङ्ग;
	स्वभाव:
		प्रत्यागम (SREC_E_BADTYPE);
	}

	/* read address field */
	*addr = 0;

	विकल्प (v) {
	पक्ष '3':				/* 4 byte addr field	*/
	पक्ष '7':
		यदि ((v = hex2_bin(निवेशः)) < 0) {
			प्रत्यागम (SREC_E_NOSREC);
		}
		*addr  += v;
		chksum += v;
		निवेशः  += 2;
		/* FALL THRU */
	पक्ष '2':				/* 3 byte addr field	*/
	पक्ष '8':
		यदि ((v = hex2_bin(निवेशः)) < 0) {
			प्रत्यागम (SREC_E_NOSREC);
		}
		*addr <<= 8;
		*addr  += v;
		chksum += v;
		निवेशः  += 2;
		/* FALL THRU */
	पक्ष '0':				/* 2 byte addr field	*/
	पक्ष '1':
	पक्ष '5':
	पक्ष '9':
		यदि ((v = hex2_bin(निवेशः)) < 0) {
			प्रत्यागम (SREC_E_NOSREC);
		}
		*addr <<= 8;
		*addr  += v;
		chksum += v;
		निवेशः  += 2;

		यदि ((v = hex2_bin(निवेशः)) < 0) {
			प्रत्यागम (SREC_E_NOSREC);
		}
		*addr <<= 8;
		*addr  += v;
		chksum += v;
		निवेशः  += 2;

		भङ्ग;
	स्वभाव:
		प्रत्यागम (SREC_E_BADTYPE);
	}

	/* convert data and calculate checksum */
	प्रति (i=0; i < *गणनम्; ++i) {
		यदि ((v = hex2_bin(निवेशः)) < 0) {
			प्रत्यागम (SREC_E_NOSREC);
		}
		दत्त[i] = v;
		chksum += v;
		निवेशः  += 2;
	}

	/* read anc check checksum */
	यदि ((v = hex2_bin(निवेशः)) < 0) {
		प्रत्यागम (SREC_E_NOSREC);
	}

	यदि ((अचिह्नित वर्ण)v != (अचिह्नित वर्ण)~chksum) {
		प्रत्यागम (SREC_E_BADCHKS);
	}

	प्रत्यागम (srec_type);
}

स्थिर पूर्णाङ्क hex1_bin (वर्ण c)
{
	यदि (c >= '0' && c <= '9')
		प्रत्यागम (c - '0');
	यदि (c >= 'a' && c <= 'f')
		प्रत्यागम (c + 10 - 'a');
	यदि (c >= 'A' && c <= 'F')
		प्रत्यागम (c + 10 - 'A');
	प्रत्यागम (-1);
}

स्थिर पूर्णाङ्क hex2_bin (वर्ण *s)
{
	पूर्णाङ्क i, j;

	यदि ((i = hex1_bin(*s++)) < 0) {
		प्रत्यागम (-1);
	}
	यदि ((j = hex1_bin(*s)) < 0) {
		प्रत्यागम (-1);
	}

	प्रत्यागम ((i<<4) + j);
}
