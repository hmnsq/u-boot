// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#अन्तर्भाव <s_record.h>

स्थिर पूर्णाङ्क hex1_bin (वर्ण  c);
स्थिर पूर्णाङ्क hex2_bin (वर्ण *s);

पूर्णाङ्क srec_decode (वर्ण *निवेशः, पूर्णाङ्क *गणनम्, ulong *addr, वर्ण *data)
{
	पूर्णाङ्क	i;
	पूर्णाङ्क	v;				/* conversion buffer	*/
	पूर्णाङ्क	srec_type;			/* S-Record type	*/
	unsigned वर्ण chksum;			/* buffer for checksum	*/

	chksum = 0;

	/* skip anything before 'S', and the 'S' itself.
	 * Return error if not found
	 */

	अत्थाय (; *निवेशः; ++निवेशः) {
		सचे (*निवेशः == 'S') {		/* skip 'S' */
			++निवेशः;
			भङ्ग;
		}
	}
	सचे (*निवेशः == '\0') {			/* no more data?	*/
		पटिनिवत्त (SREC_EMPTY);
	}

	v = *निवेशः++;				/* record type		*/

	सचे ((*गणनम् = hex2_bin(निवेशः)) < 0) {
		पटिनिवत्त (SREC_E_NOSREC);
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
		पटिनिवत्त (SREC_E_BADTYPE);
	}

	/* read address field */
	*addr = 0;

	विकल्प (v) {
	पक्ष '3':				/* 4 byte addr field	*/
	पक्ष '7':
		सचे ((v = hex2_bin(निवेशः)) < 0) {
			पटिनिवत्त (SREC_E_NOSREC);
		}
		*addr  += v;
		chksum += v;
		निवेशः  += 2;
		/* FALL THRU */
	पक्ष '2':				/* 3 byte addr field	*/
	पक्ष '8':
		सचे ((v = hex2_bin(निवेशः)) < 0) {
			पटिनिवत्त (SREC_E_NOSREC);
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
		सचे ((v = hex2_bin(निवेशः)) < 0) {
			पटिनिवत्त (SREC_E_NOSREC);
		}
		*addr <<= 8;
		*addr  += v;
		chksum += v;
		निवेशः  += 2;

		सचे ((v = hex2_bin(निवेशः)) < 0) {
			पटिनिवत्त (SREC_E_NOSREC);
		}
		*addr <<= 8;
		*addr  += v;
		chksum += v;
		निवेशः  += 2;

		भङ्ग;
	स्वभाव:
		पटिनिवत्त (SREC_E_BADTYPE);
	}

	/* convert data and calculate checksum */
	अत्थाय (i=0; i < *गणनम्; ++i) {
		सचे ((v = hex2_bin(निवेशः)) < 0) {
			पटिनिवत्त (SREC_E_NOSREC);
		}
		data[i] = v;
		chksum += v;
		निवेशः  += 2;
	}

	/* read anc check checksum */
	सचे ((v = hex2_bin(निवेशः)) < 0) {
		पटिनिवत्त (SREC_E_NOSREC);
	}

	सचे ((unsigned वर्ण)v != (unsigned वर्ण)~chksum) {
		पटिनिवत्त (SREC_E_BADCHKS);
	}

	पटिनिवत्त (srec_type);
}

स्थिर पूर्णाङ्क hex1_bin (वर्ण c)
{
	सचे (c >= '0' && c <= '9')
		पटिनिवत्त (c - '0');
	सचे (c >= 'a' && c <= 'f')
		पटिनिवत्त (c + 10 - 'a');
	सचे (c >= 'A' && c <= 'F')
		पटिनिवत्त (c + 10 - 'A');
	पटिनिवत्त (-1);
}

स्थिर पूर्णाङ्क hex2_bin (वर्ण *s)
{
	पूर्णाङ्क i, j;

	सचे ((i = hex1_bin(*s++)) < 0) {
		पटिनिवत्त (-1);
	}
	सचे ((j = hex1_bin(*s)) < 0) {
		पटिनिवत्त (-1);
	}

	पटिनिवत्त ((i<<4) + j);
}
