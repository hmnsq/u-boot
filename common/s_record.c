// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#അന്തര്ഭാവ <s_record.h>

സ്ഥിതം പൂർണ്ണസംഖ്യ hex1_bin (അക്ഷരം  c);
സ്ഥിതം പൂർണ്ണസംഖ്യ hex2_bin (അക്ഷരം *s);

പൂർണ്ണസംഖ്യ srec_decode (അക്ഷരം *നിവേശം, പൂർണ്ണസംഖ്യ *എണ്ണം, ulong *addr, അക്ഷരം *data)
{
	പൂർണ്ണസംഖ്യ	i;
	പൂർണ്ണസംഖ്യ	v;				/* conversion buffer	*/
	പൂർണ്ണസംഖ്യ	srec_type;			/* S-Record type	*/
	unsigned അക്ഷരം chksum;			/* buffer for checksum	*/

	chksum = 0;

	/* skip anything before 'S', and the 'S' itself.
	 * Return error if not found
	 */

	വേണ്ടി (; *നിവേശം; ++നിവേശം) {
		എങ്കിൽ (*നിവേശം == 'S') {		/* skip 'S' */
			++നിവേശം;
			നിർത്തുക;
		}
	}
	എങ്കിൽ (*നിവേശം == '\0') {			/* no more data?	*/
		തിരികെ (SREC_EMPTY);
	}

	v = *നിവേശം++;				/* record type		*/

	എങ്കിൽ ((*എണ്ണം = hex2_bin(നിവേശം)) < 0) {
		തിരികെ (SREC_E_NOSREC);
	}

	chksum += *എണ്ണം;
	നിവേശം  += 2;

	തിരഞ്ഞെടുപ്പ് (v) {				/* record type		*/

	അവസ്ഥ '0':				/* start record		*/
		srec_type = SREC_START;		/* 2 byte addr field	*/
		*എണ്ണം   -= 3;			/* - checksum and addr	*/
		നിർത്തുക;
	അവസ്ഥ '1':
		srec_type = SREC_DATA2;		/* 2 byte addr field	*/
		*എണ്ണം   -= 3;			/* - checksum and addr	*/
		നിർത്തുക;
	അവസ്ഥ '2':
		srec_type = SREC_DATA3;		/* 3 byte addr field	*/
		*എണ്ണം   -= 4;			/* - checksum and addr	*/
		നിർത്തുക;
	അവസ്ഥ '3':				/* data record with a	*/
		srec_type = SREC_DATA4;		/* 4 byte addr field	*/
		*എണ്ണം   -= 5;			/* - checksum and addr	*/
		നിർത്തുക;
/***	case '4'  ***/
	അവസ്ഥ '5':			/* count record, addr field contains */
		srec_type = SREC_എണ്ണം;	/* a 2 byte record counter	*/
		*എണ്ണം    = 0;			/* no data		*/
		നിർത്തുക;
/***	case '6' -- not used  ***/
	അവസ്ഥ '7':				/* end record with a	*/
		srec_type = SREC_END4;		/* 4 byte addr field	*/
		*എണ്ണം   -= 5;			/* - checksum and addr	*/
		നിർത്തുക;
	അവസ്ഥ '8':				/* end record with a	*/
		srec_type = SREC_END3;		/* 3 byte addr field	*/
		*എണ്ണം   -= 4;			/* - checksum and addr	*/
		നിർത്തുക;
	അവസ്ഥ '9':				/* end record with a	*/
		srec_type = SREC_END2;		/* 2 byte addr field	*/
		*എണ്ണം   -= 3;			/* - checksum and addr	*/
		നിർത്തുക;
	സ്ഥിരസ്ഥിതി:
		തിരികെ (SREC_E_BADTYPE);
	}

	/* read address field */
	*addr = 0;

	തിരഞ്ഞെടുപ്പ് (v) {
	അവസ്ഥ '3':				/* 4 byte addr field	*/
	അവസ്ഥ '7':
		എങ്കിൽ ((v = hex2_bin(നിവേശം)) < 0) {
			തിരികെ (SREC_E_NOSREC);
		}
		*addr  += v;
		chksum += v;
		നിവേശം  += 2;
		/* FALL THRU */
	അവസ്ഥ '2':				/* 3 byte addr field	*/
	അവസ്ഥ '8':
		എങ്കിൽ ((v = hex2_bin(നിവേശം)) < 0) {
			തിരികെ (SREC_E_NOSREC);
		}
		*addr <<= 8;
		*addr  += v;
		chksum += v;
		നിവേശം  += 2;
		/* FALL THRU */
	അവസ്ഥ '0':				/* 2 byte addr field	*/
	അവസ്ഥ '1':
	അവസ്ഥ '5':
	അവസ്ഥ '9':
		എങ്കിൽ ((v = hex2_bin(നിവേശം)) < 0) {
			തിരികെ (SREC_E_NOSREC);
		}
		*addr <<= 8;
		*addr  += v;
		chksum += v;
		നിവേശം  += 2;

		എങ്കിൽ ((v = hex2_bin(നിവേശം)) < 0) {
			തിരികെ (SREC_E_NOSREC);
		}
		*addr <<= 8;
		*addr  += v;
		chksum += v;
		നിവേശം  += 2;

		നിർത്തുക;
	സ്ഥിരസ്ഥിതി:
		തിരികെ (SREC_E_BADTYPE);
	}

	/* convert data and calculate checksum */
	വേണ്ടി (i=0; i < *എണ്ണം; ++i) {
		എങ്കിൽ ((v = hex2_bin(നിവേശം)) < 0) {
			തിരികെ (SREC_E_NOSREC);
		}
		data[i] = v;
		chksum += v;
		നിവേശം  += 2;
	}

	/* read anc check checksum */
	എങ്കിൽ ((v = hex2_bin(നിവേശം)) < 0) {
		തിരികെ (SREC_E_NOSREC);
	}

	എങ്കിൽ ((unsigned അക്ഷരം)v != (unsigned അക്ഷരം)~chksum) {
		തിരികെ (SREC_E_BADCHKS);
	}

	തിരികെ (srec_type);
}

സ്ഥിതം പൂർണ്ണസംഖ്യ hex1_bin (അക്ഷരം c)
{
	എങ്കിൽ (c >= '0' && c <= '9')
		തിരികെ (c - '0');
	എങ്കിൽ (c >= 'a' && c <= 'f')
		തിരികെ (c + 10 - 'a');
	എങ്കിൽ (c >= 'A' && c <= 'F')
		തിരികെ (c + 10 - 'A');
	തിരികെ (-1);
}

സ്ഥിതം പൂർണ്ണസംഖ്യ hex2_bin (അക്ഷരം *s)
{
	പൂർണ്ണസംഖ്യ i, j;

	എങ്കിൽ ((i = hex1_bin(*s++)) < 0) {
		തിരികെ (-1);
	}
	എങ്കിൽ ((j = hex1_bin(*s)) < 0) {
		തിരികെ (-1);
	}

	തിരികെ ((i<<4) + j);
}
