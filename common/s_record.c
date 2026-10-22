// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#समावेश <s_record.h>

अटल पूर्णांक hex1_bin (अक्षर  c);
अटल पूर्णांक hex2_bin (अक्षर *s);

पूर्णांक srec_decode (अक्षर *निवेश, पूर्णांक *गिनती, ulong *addr, अक्षर *तथ्य)
{
	पूर्णांक	i;
	पूर्णांक	v;				/* conversion buffer	*/
	पूर्णांक	srec_type;			/* S-Record type	*/
	अचिन्हित अक्षर chksum;			/* buffer for checksum	*/

	chksum = 0;

	/* skip anything before 'S', and the 'S' itself.
	 * Return error if not found
	 */

	क्रम (; *निवेश; ++निवेश) {
		अगर (*निवेश == 'S') {		/* skip 'S' */
			++निवेश;
			अवरोध;
		}
	}
	अगर (*निवेश == '\0') {			/* no more data?	*/
		वापस (SREC_EMPTY);
	}

	v = *निवेश++;				/* record type		*/

	अगर ((*गिनती = hex2_bin(निवेश)) < 0) {
		वापस (SREC_E_NOSREC);
	}

	chksum += *गिनती;
	निवेश  += 2;

	चयन (v) {				/* record type		*/

	हाल '0':				/* start record		*/
		srec_type = SREC_START;		/* 2 byte addr field	*/
		*गिनती   -= 3;			/* - checksum and addr	*/
		अवरोध;
	हाल '1':
		srec_type = SREC_DATA2;		/* 2 byte addr field	*/
		*गिनती   -= 3;			/* - checksum and addr	*/
		अवरोध;
	हाल '2':
		srec_type = SREC_DATA3;		/* 3 byte addr field	*/
		*गिनती   -= 4;			/* - checksum and addr	*/
		अवरोध;
	हाल '3':				/* data record with a	*/
		srec_type = SREC_DATA4;		/* 4 byte addr field	*/
		*गिनती   -= 5;			/* - checksum and addr	*/
		अवरोध;
/***	case '4'  ***/
	हाल '5':			/* count record, addr field contains */
		srec_type = SREC_गिनती;	/* a 2 byte record counter	*/
		*गिनती    = 0;			/* no data		*/
		अवरोध;
/***	case '6' -- not used  ***/
	हाल '7':				/* end record with a	*/
		srec_type = SREC_END4;		/* 4 byte addr field	*/
		*गिनती   -= 5;			/* - checksum and addr	*/
		अवरोध;
	हाल '8':				/* end record with a	*/
		srec_type = SREC_END3;		/* 3 byte addr field	*/
		*गिनती   -= 4;			/* - checksum and addr	*/
		अवरोध;
	हाल '9':				/* end record with a	*/
		srec_type = SREC_END2;		/* 2 byte addr field	*/
		*गिनती   -= 3;			/* - checksum and addr	*/
		अवरोध;
	शेष:
		वापस (SREC_E_BADTYPE);
	}

	/* read address field */
	*addr = 0;

	चयन (v) {
	हाल '3':				/* 4 byte addr field	*/
	हाल '7':
		अगर ((v = hex2_bin(निवेश)) < 0) {
			वापस (SREC_E_NOSREC);
		}
		*addr  += v;
		chksum += v;
		निवेश  += 2;
		/* FALL THRU */
	हाल '2':				/* 3 byte addr field	*/
	हाल '8':
		अगर ((v = hex2_bin(निवेश)) < 0) {
			वापस (SREC_E_NOSREC);
		}
		*addr <<= 8;
		*addr  += v;
		chksum += v;
		निवेश  += 2;
		/* FALL THRU */
	हाल '0':				/* 2 byte addr field	*/
	हाल '1':
	हाल '5':
	हाल '9':
		अगर ((v = hex2_bin(निवेश)) < 0) {
			वापस (SREC_E_NOSREC);
		}
		*addr <<= 8;
		*addr  += v;
		chksum += v;
		निवेश  += 2;

		अगर ((v = hex2_bin(निवेश)) < 0) {
			वापस (SREC_E_NOSREC);
		}
		*addr <<= 8;
		*addr  += v;
		chksum += v;
		निवेश  += 2;

		अवरोध;
	शेष:
		वापस (SREC_E_BADTYPE);
	}

	/* convert data and calculate checksum */
	क्रम (i=0; i < *गिनती; ++i) {
		अगर ((v = hex2_bin(निवेश)) < 0) {
			वापस (SREC_E_NOSREC);
		}
		तथ्य[i] = v;
		chksum += v;
		निवेश  += 2;
	}

	/* read anc check checksum */
	अगर ((v = hex2_bin(निवेश)) < 0) {
		वापस (SREC_E_NOSREC);
	}

	अगर ((अचिन्हित अक्षर)v != (अचिन्हित अक्षर)~chksum) {
		वापस (SREC_E_BADCHKS);
	}

	वापस (srec_type);
}

अटल पूर्णांक hex1_bin (अक्षर c)
{
	अगर (c >= '0' && c <= '9')
		वापस (c - '0');
	अगर (c >= 'a' && c <= 'f')
		वापस (c + 10 - 'a');
	अगर (c >= 'A' && c <= 'F')
		वापस (c + 10 - 'A');
	वापस (-1);
}

अटल पूर्णांक hex2_bin (अक्षर *s)
{
	पूर्णांक i, j;

	अगर ((i = hex1_bin(*s++)) < 0) {
		वापस (-1);
	}
	अगर ((j = hex1_bin(*s)) < 0) {
		वापस (-1);
	}

	वापस ((i<<4) + j);
}
