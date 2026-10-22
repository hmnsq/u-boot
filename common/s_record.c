// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#अन्तर्भाव <s_record.h>

स्थिर पूर्णाङ्क hex1_bin (वर्ण  c);
स्थिर पूर्णाङ्क hex2_bin (वर्ण *s);

पूर्णाङ्क srec_decode (वर्ण *आगत, पूर्णाङ्क *गन्ती, ulong *addr, वर्ण *data)
{
	पूर्णाङ्क	i;
	पूर्णाङ्क	v;				/* conversion buffer	*/
	पूर्णाङ्क	srec_type;			/* S-Record type	*/
	अचिन्हित वर्ण chksum;			/* buffer for checksum	*/

	chksum = 0;

	/* skip anything before 'S', and the 'S' itself.
	 * Return error if not found
	 */

	लागि (; *आगत; ++आगत) {
		यदि (*आगत == 'S') {		/* skip 'S' */
			++आगत;
			रोक;
		}
	}
	यदि (*आगत == '\0') {			/* no more data?	*/
		फर्काऊ (SREC_EMPTY);
	}

	v = *आगत++;				/* record type		*/

	यदि ((*गन्ती = hex2_bin(आगत)) < 0) {
		फर्काऊ (SREC_E_NOSREC);
	}

	chksum += *गन्ती;
	आगत  += 2;

	विकल्प (v) {				/* record type		*/

	पक्ष '0':				/* start record		*/
		srec_type = SREC_START;		/* 2 byte addr field	*/
		*गन्ती   -= 3;			/* - checksum and addr	*/
		रोक;
	पक्ष '1':
		srec_type = SREC_DATA2;		/* 2 byte addr field	*/
		*गन्ती   -= 3;			/* - checksum and addr	*/
		रोक;
	पक्ष '2':
		srec_type = SREC_DATA3;		/* 3 byte addr field	*/
		*गन्ती   -= 4;			/* - checksum and addr	*/
		रोक;
	पक्ष '3':				/* data record with a	*/
		srec_type = SREC_DATA4;		/* 4 byte addr field	*/
		*गन्ती   -= 5;			/* - checksum and addr	*/
		रोक;
/***	case '4'  ***/
	पक्ष '5':			/* count record, addr field contains */
		srec_type = SREC_गन्ती;	/* a 2 byte record counter	*/
		*गन्ती    = 0;			/* no data		*/
		रोक;
/***	case '6' -- not used  ***/
	पक्ष '7':				/* end record with a	*/
		srec_type = SREC_END4;		/* 4 byte addr field	*/
		*गन्ती   -= 5;			/* - checksum and addr	*/
		रोक;
	पक्ष '8':				/* end record with a	*/
		srec_type = SREC_END3;		/* 3 byte addr field	*/
		*गन्ती   -= 4;			/* - checksum and addr	*/
		रोक;
	पक्ष '9':				/* end record with a	*/
		srec_type = SREC_END2;		/* 2 byte addr field	*/
		*गन्ती   -= 3;			/* - checksum and addr	*/
		रोक;
	स्वभाव:
		फर्काऊ (SREC_E_BADTYPE);
	}

	/* read address field */
	*addr = 0;

	विकल्प (v) {
	पक्ष '3':				/* 4 byte addr field	*/
	पक्ष '7':
		यदि ((v = hex2_bin(आगत)) < 0) {
			फर्काऊ (SREC_E_NOSREC);
		}
		*addr  += v;
		chksum += v;
		आगत  += 2;
		/* FALL THRU */
	पक्ष '2':				/* 3 byte addr field	*/
	पक्ष '8':
		यदि ((v = hex2_bin(आगत)) < 0) {
			फर्काऊ (SREC_E_NOSREC);
		}
		*addr <<= 8;
		*addr  += v;
		chksum += v;
		आगत  += 2;
		/* FALL THRU */
	पक्ष '0':				/* 2 byte addr field	*/
	पक्ष '1':
	पक्ष '5':
	पक्ष '9':
		यदि ((v = hex2_bin(आगत)) < 0) {
			फर्काऊ (SREC_E_NOSREC);
		}
		*addr <<= 8;
		*addr  += v;
		chksum += v;
		आगत  += 2;

		यदि ((v = hex2_bin(आगत)) < 0) {
			फर्काऊ (SREC_E_NOSREC);
		}
		*addr <<= 8;
		*addr  += v;
		chksum += v;
		आगत  += 2;

		रोक;
	स्वभाव:
		फर्काऊ (SREC_E_BADTYPE);
	}

	/* convert data and calculate checksum */
	लागि (i=0; i < *गन्ती; ++i) {
		यदि ((v = hex2_bin(आगत)) < 0) {
			फर्काऊ (SREC_E_NOSREC);
		}
		data[i] = v;
		chksum += v;
		आगत  += 2;
	}

	/* read anc check checksum */
	यदि ((v = hex2_bin(आगत)) < 0) {
		फर्काऊ (SREC_E_NOSREC);
	}

	यदि ((अचिन्हित वर्ण)v != (अचिन्हित वर्ण)~chksum) {
		फर्काऊ (SREC_E_BADCHKS);
	}

	फर्काऊ (srec_type);
}

स्थिर पूर्णाङ्क hex1_bin (वर्ण c)
{
	यदि (c >= '0' && c <= '9')
		फर्काऊ (c - '0');
	यदि (c >= 'a' && c <= 'f')
		फर्काऊ (c + 10 - 'a');
	यदि (c >= 'A' && c <= 'F')
		फर्काऊ (c + 10 - 'A');
	फर्काऊ (-1);
}

स्थिर पूर्णाङ्क hex2_bin (वर्ण *s)
{
	पूर्णाङ्क i, j;

	यदि ((i = hex1_bin(*s++)) < 0) {
		फर्काऊ (-1);
	}
	यदि ((j = hex1_bin(*s)) < 0) {
		फर्काऊ (-1);
	}

	फर्काऊ ((i<<4) + j);
}
