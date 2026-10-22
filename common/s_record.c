// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#अन्तर्भाव <s_record.h>

स्थिर पूर्णांक hex1_bin (अक्षर  c);
स्थिर पूर्णांक hex2_bin (अक्षर *s);

पूर्णांक srec_decode (अक्षर *आदान, पूर्णांक *मोजणी, ulong *addr, अक्षर *data)
{
	पूर्णांक	i;
	पूर्णांक	v;				/* conversion buffer	*/
	पूर्णांक	srec_type;			/* S-Record type	*/
	अचिन्हित अक्षर chksum;			/* buffer for checksum	*/

	chksum = 0;

	/* skip anything before 'S', and the 'S' itself.
	 * Return error if not found
	 */

	साठी (; *आदान; ++आदान) {
		जर (*आदान == 'S') {		/* skip 'S' */
			++आदान;
			थांबा;
		}
	}
	जर (*आदान == '\0') {			/* no more data?	*/
		परत (SREC_EMPTY);
	}

	v = *आदान++;				/* record type		*/

	जर ((*मोजणी = hex2_bin(आदान)) < 0) {
		परत (SREC_E_NOSREC);
	}

	chksum += *मोजणी;
	आदान  += 2;

	निवड (v) {				/* record type		*/

	स्थिती '0':				/* start record		*/
		srec_type = SREC_START;		/* 2 byte addr field	*/
		*मोजणी   -= 3;			/* - checksum and addr	*/
		थांबा;
	स्थिती '1':
		srec_type = SREC_DATA2;		/* 2 byte addr field	*/
		*मोजणी   -= 3;			/* - checksum and addr	*/
		थांबा;
	स्थिती '2':
		srec_type = SREC_DATA3;		/* 3 byte addr field	*/
		*मोजणी   -= 4;			/* - checksum and addr	*/
		थांबा;
	स्थिती '3':				/* data record with a	*/
		srec_type = SREC_DATA4;		/* 4 byte addr field	*/
		*मोजणी   -= 5;			/* - checksum and addr	*/
		थांबा;
/***	case '4'  ***/
	स्थिती '5':			/* count record, addr field contains */
		srec_type = SREC_मोजणी;	/* a 2 byte record counter	*/
		*मोजणी    = 0;			/* no data		*/
		थांबा;
/***	case '6' -- not used  ***/
	स्थिती '7':				/* end record with a	*/
		srec_type = SREC_END4;		/* 4 byte addr field	*/
		*मोजणी   -= 5;			/* - checksum and addr	*/
		थांबा;
	स्थिती '8':				/* end record with a	*/
		srec_type = SREC_END3;		/* 3 byte addr field	*/
		*मोजणी   -= 4;			/* - checksum and addr	*/
		थांबा;
	स्थिती '9':				/* end record with a	*/
		srec_type = SREC_END2;		/* 2 byte addr field	*/
		*मोजणी   -= 3;			/* - checksum and addr	*/
		थांबा;
	पूर्वनिर्धारित:
		परत (SREC_E_BADTYPE);
	}

	/* read address field */
	*addr = 0;

	निवड (v) {
	स्थिती '3':				/* 4 byte addr field	*/
	स्थिती '7':
		जर ((v = hex2_bin(आदान)) < 0) {
			परत (SREC_E_NOSREC);
		}
		*addr  += v;
		chksum += v;
		आदान  += 2;
		/* FALL THRU */
	स्थिती '2':				/* 3 byte addr field	*/
	स्थिती '8':
		जर ((v = hex2_bin(आदान)) < 0) {
			परत (SREC_E_NOSREC);
		}
		*addr <<= 8;
		*addr  += v;
		chksum += v;
		आदान  += 2;
		/* FALL THRU */
	स्थिती '0':				/* 2 byte addr field	*/
	स्थिती '1':
	स्थिती '5':
	स्थिती '9':
		जर ((v = hex2_bin(आदान)) < 0) {
			परत (SREC_E_NOSREC);
		}
		*addr <<= 8;
		*addr  += v;
		chksum += v;
		आदान  += 2;

		जर ((v = hex2_bin(आदान)) < 0) {
			परत (SREC_E_NOSREC);
		}
		*addr <<= 8;
		*addr  += v;
		chksum += v;
		आदान  += 2;

		थांबा;
	पूर्वनिर्धारित:
		परत (SREC_E_BADTYPE);
	}

	/* convert data and calculate checksum */
	साठी (i=0; i < *मोजणी; ++i) {
		जर ((v = hex2_bin(आदान)) < 0) {
			परत (SREC_E_NOSREC);
		}
		data[i] = v;
		chksum += v;
		आदान  += 2;
	}

	/* read anc check checksum */
	जर ((v = hex2_bin(आदान)) < 0) {
		परत (SREC_E_NOSREC);
	}

	जर ((अचिन्हित अक्षर)v != (अचिन्हित अक्षर)~chksum) {
		परत (SREC_E_BADCHKS);
	}

	परत (srec_type);
}

स्थिर पूर्णांक hex1_bin (अक्षर c)
{
	जर (c >= '0' && c <= '9')
		परत (c - '0');
	जर (c >= 'a' && c <= 'f')
		परत (c + 10 - 'a');
	जर (c >= 'A' && c <= 'F')
		परत (c + 10 - 'A');
	परत (-1);
}

स्थिर पूर्णांक hex2_bin (अक्षर *s)
{
	पूर्णांक i, j;

	जर ((i = hex1_bin(*s++)) < 0) {
		परत (-1);
	}
	जर ((j = hex1_bin(*s)) < 0) {
		परत (-1);
	}

	परत ((i<<4) + j);
}
