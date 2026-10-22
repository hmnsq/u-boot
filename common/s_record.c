// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#ਅਨ੍ਤਰ੍ਭਾਵ <s_record.h>

ਸਥਿਰ ਪੂਰਨ hex1_bin (ਅੱਖਰ  c);
ਸਥਿਰ ਪੂਰਨ hex2_bin (ਅੱਖਰ *s);

ਪੂਰਨ srec_decode (ਅੱਖਰ *ਨਿਵੇਸ਼, ਪੂਰਨ *ਗਿਣਤੀ_2, ulong *addr, ਅੱਖਰ *ਅੰਕੜੇ)
{
	ਪੂਰਨ	i;
	ਪੂਰਨ	v;				/* conversion buffer	*/
	ਪੂਰਨ	srec_type;			/* S-Record type	*/
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ chksum;			/* buffer for checksum	*/

	chksum = 0;

	/* skip anything before 'S', and the 'S' itself.
	 * Return error if not found
	 */

	ਲਈ (; *ਨਿਵੇਸ਼; ++ਨਿਵੇਸ਼) {
		ਜੇ (*ਨਿਵੇਸ਼ == 'S') {		/* skip 'S' */
			++ਨਿਵੇਸ਼;
			ਰੋਕੋ;
		}
	}
	ਜੇ (*ਨਿਵੇਸ਼ == '\0') {			/* no more data?	*/
		ਵਾਪਸ (SREC_ਖਾਲੀ_ਥਾਵਾਂ);
	}

	v = *ਨਿਵੇਸ਼++;				/* record type		*/

	ਜੇ ((*ਗਿਣਤੀ_2 = hex2_bin(ਨਿਵੇਸ਼)) < 0) {
		ਵਾਪਸ (SREC_E_NOSREC);
	}

	chksum += *ਗਿਣਤੀ_2;
	ਨਿਵੇਸ਼  += 2;

	ਚੋਣ (v) {				/* record type		*/

	ਹਾਲਤ '0':				/* start record		*/
		srec_type = SREC_ਸ਼ੁਰੂ;		/* 2 byte addr field	*/
		*ਗਿਣਤੀ_2   -= 3;			/* - checksum and addr	*/
		ਰੋਕੋ;
	ਹਾਲਤ '1':
		srec_type = SREC_DATA2;		/* 2 byte addr field	*/
		*ਗਿਣਤੀ_2   -= 3;			/* - checksum and addr	*/
		ਰੋਕੋ;
	ਹਾਲਤ '2':
		srec_type = SREC_DATA3;		/* 3 byte addr field	*/
		*ਗਿਣਤੀ_2   -= 4;			/* - checksum and addr	*/
		ਰੋਕੋ;
	ਹਾਲਤ '3':				/* data record with a	*/
		srec_type = SREC_DATA4;		/* 4 byte addr field	*/
		*ਗਿਣਤੀ_2   -= 5;			/* - checksum and addr	*/
		ਰੋਕੋ;
/***	case '4'  ***/
	ਹਾਲਤ '5':			/* count record, addr field contains */
		srec_type = SREC_ਗਿਣਤੀ;	/* a 2 byte record counter	*/
		*ਗਿਣਤੀ_2    = 0;			/* no data		*/
		ਰੋਕੋ;
/***	case '6' -- not used  ***/
	ਹਾਲਤ '7':				/* end record with a	*/
		srec_type = SREC_END4;		/* 4 byte addr field	*/
		*ਗਿਣਤੀ_2   -= 5;			/* - checksum and addr	*/
		ਰੋਕੋ;
	ਹਾਲਤ '8':				/* end record with a	*/
		srec_type = SREC_END3;		/* 3 byte addr field	*/
		*ਗਿਣਤੀ_2   -= 4;			/* - checksum and addr	*/
		ਰੋਕੋ;
	ਹਾਲਤ '9':				/* end record with a	*/
		srec_type = SREC_END2;		/* 2 byte addr field	*/
		*ਗਿਣਤੀ_2   -= 3;			/* - checksum and addr	*/
		ਰੋਕੋ;
	ਮੂਲ:
		ਵਾਪਸ (SREC_E_BADTYPE);
	}

	/* read address field */
	*addr = 0;

	ਚੋਣ (v) {
	ਹਾਲਤ '3':				/* 4 byte addr field	*/
	ਹਾਲਤ '7':
		ਜੇ ((v = hex2_bin(ਨਿਵੇਸ਼)) < 0) {
			ਵਾਪਸ (SREC_E_NOSREC);
		}
		*addr  += v;
		chksum += v;
		ਨਿਵੇਸ਼  += 2;
		/* FALL THRU */
	ਹਾਲਤ '2':				/* 3 byte addr field	*/
	ਹਾਲਤ '8':
		ਜੇ ((v = hex2_bin(ਨਿਵੇਸ਼)) < 0) {
			ਵਾਪਸ (SREC_E_NOSREC);
		}
		*addr <<= 8;
		*addr  += v;
		chksum += v;
		ਨਿਵੇਸ਼  += 2;
		/* FALL THRU */
	ਹਾਲਤ '0':				/* 2 byte addr field	*/
	ਹਾਲਤ '1':
	ਹਾਲਤ '5':
	ਹਾਲਤ '9':
		ਜੇ ((v = hex2_bin(ਨਿਵੇਸ਼)) < 0) {
			ਵਾਪਸ (SREC_E_NOSREC);
		}
		*addr <<= 8;
		*addr  += v;
		chksum += v;
		ਨਿਵੇਸ਼  += 2;

		ਜੇ ((v = hex2_bin(ਨਿਵੇਸ਼)) < 0) {
			ਵਾਪਸ (SREC_E_NOSREC);
		}
		*addr <<= 8;
		*addr  += v;
		chksum += v;
		ਨਿਵੇਸ਼  += 2;

		ਰੋਕੋ;
	ਮੂਲ:
		ਵਾਪਸ (SREC_E_BADTYPE);
	}

	/* convert data and calculate checksum */
	ਲਈ (i=0; i < *ਗਿਣਤੀ_2; ++i) {
		ਜੇ ((v = hex2_bin(ਨਿਵੇਸ਼)) < 0) {
			ਵਾਪਸ (SREC_E_NOSREC);
		}
		ਅੰਕੜੇ[i] = v;
		chksum += v;
		ਨਿਵੇਸ਼  += 2;
	}

	/* read anc check checksum */
	ਜੇ ((v = hex2_bin(ਨਿਵੇਸ਼)) < 0) {
		ਵਾਪਸ (SREC_E_NOSREC);
	}

	ਜੇ ((ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)v != (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)~chksum) {
		ਵਾਪਸ (SREC_E_BADCHKS);
	}

	ਵਾਪਸ (srec_type);
}

ਸਥਿਰ ਪੂਰਨ hex1_bin (ਅੱਖਰ c)
{
	ਜੇ (c >= '0' && c <= '9')
		ਵਾਪਸ (c - '0');
	ਜੇ (c >= 'a' && c <= 'f')
		ਵਾਪਸ (c + 10 - 'a');
	ਜੇ (c >= 'A' && c <= 'F')
		ਵਾਪਸ (c + 10 - 'A');
	ਵਾਪਸ (-1);
}

ਸਥਿਰ ਪੂਰਨ hex2_bin (ਅੱਖਰ *s)
{
	ਪੂਰਨ i, j;

	ਜੇ ((i = hex1_bin(*s++)) < 0) {
		ਵਾਪਸ (-1);
	}
	ਜੇ ((j = hex1_bin(*s)) < 0) {
		ਵਾਪਸ (-1);
	}

	ਵਾਪਸ ((i<<4) + j);
}
