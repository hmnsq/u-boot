// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#அந்தர்பாவ <s_record.h>

நிலையான முழு hex1_bin (எழுத்து  c);
நிலையான முழு hex2_bin (எழுத்து *s);

முழு srec_decode (எழுத்து *உள்ளீடு, முழு *மொத்தம், ulong *addr, எழுத்து *data)
{
	முழு	i;
	முழு	v;				/* conversion buffer	*/
	முழு	srec_type;			/* S-Record type	*/
	unsigned எழுத்து chksum;			/* buffer for checksum	*/

	chksum = 0;

	/* skip anything before 'S', and the 'S' itself.
	 * Return error if not found
	 */

	க்கு (; *உள்ளீடு; ++உள்ளீடு) {
		எனில் (*உள்ளீடு == 'S') {		/* skip 'S' */
			++உள்ளீடு;
			நிறுத்து;
		}
	}
	எனில் (*உள்ளீடு == '\0') {			/* no more data?	*/
		திரும்பு (SREC_EMPTY);
	}

	v = *உள்ளீடு++;				/* record type		*/

	எனில் ((*மொத்தம் = hex2_bin(உள்ளீடு)) < 0) {
		திரும்பு (SREC_E_NOSREC);
	}

	chksum += *மொத்தம்;
	உள்ளீடு  += 2;

	தேர்வு (v) {				/* record type		*/

	நிலை '0':				/* start record		*/
		srec_type = SREC_START;		/* 2 byte addr field	*/
		*மொத்தம்   -= 3;			/* - checksum and addr	*/
		நிறுத்து;
	நிலை '1':
		srec_type = SREC_DATA2;		/* 2 byte addr field	*/
		*மொத்தம்   -= 3;			/* - checksum and addr	*/
		நிறுத்து;
	நிலை '2':
		srec_type = SREC_DATA3;		/* 3 byte addr field	*/
		*மொத்தம்   -= 4;			/* - checksum and addr	*/
		நிறுத்து;
	நிலை '3':				/* data record with a	*/
		srec_type = SREC_DATA4;		/* 4 byte addr field	*/
		*மொத்தம்   -= 5;			/* - checksum and addr	*/
		நிறுத்து;
/***	case '4'  ***/
	நிலை '5':			/* count record, addr field contains */
		srec_type = SREC_மொத்தம்;	/* a 2 byte record counter	*/
		*மொத்தம்    = 0;			/* no data		*/
		நிறுத்து;
/***	case '6' -- not used  ***/
	நிலை '7':				/* end record with a	*/
		srec_type = SREC_END4;		/* 4 byte addr field	*/
		*மொத்தம்   -= 5;			/* - checksum and addr	*/
		நிறுத்து;
	நிலை '8':				/* end record with a	*/
		srec_type = SREC_END3;		/* 3 byte addr field	*/
		*மொத்தம்   -= 4;			/* - checksum and addr	*/
		நிறுத்து;
	நிலை '9':				/* end record with a	*/
		srec_type = SREC_END2;		/* 2 byte addr field	*/
		*மொத்தம்   -= 3;			/* - checksum and addr	*/
		நிறுத்து;
	இயல்பு:
		திரும்பு (SREC_E_BADTYPE);
	}

	/* read address field */
	*addr = 0;

	தேர்வு (v) {
	நிலை '3':				/* 4 byte addr field	*/
	நிலை '7':
		எனில் ((v = hex2_bin(உள்ளீடு)) < 0) {
			திரும்பு (SREC_E_NOSREC);
		}
		*addr  += v;
		chksum += v;
		உள்ளீடு  += 2;
		/* FALL THRU */
	நிலை '2':				/* 3 byte addr field	*/
	நிலை '8':
		எனில் ((v = hex2_bin(உள்ளீடு)) < 0) {
			திரும்பு (SREC_E_NOSREC);
		}
		*addr <<= 8;
		*addr  += v;
		chksum += v;
		உள்ளீடு  += 2;
		/* FALL THRU */
	நிலை '0':				/* 2 byte addr field	*/
	நிலை '1':
	நிலை '5':
	நிலை '9':
		எனில் ((v = hex2_bin(உள்ளீடு)) < 0) {
			திரும்பு (SREC_E_NOSREC);
		}
		*addr <<= 8;
		*addr  += v;
		chksum += v;
		உள்ளீடு  += 2;

		எனில் ((v = hex2_bin(உள்ளீடு)) < 0) {
			திரும்பு (SREC_E_NOSREC);
		}
		*addr <<= 8;
		*addr  += v;
		chksum += v;
		உள்ளீடு  += 2;

		நிறுத்து;
	இயல்பு:
		திரும்பு (SREC_E_BADTYPE);
	}

	/* convert data and calculate checksum */
	க்கு (i=0; i < *மொத்தம்; ++i) {
		எனில் ((v = hex2_bin(உள்ளீடு)) < 0) {
			திரும்பு (SREC_E_NOSREC);
		}
		data[i] = v;
		chksum += v;
		உள்ளீடு  += 2;
	}

	/* read anc check checksum */
	எனில் ((v = hex2_bin(உள்ளீடு)) < 0) {
		திரும்பு (SREC_E_NOSREC);
	}

	எனில் ((unsigned எழுத்து)v != (unsigned எழுத்து)~chksum) {
		திரும்பு (SREC_E_BADCHKS);
	}

	திரும்பு (srec_type);
}

நிலையான முழு hex1_bin (எழுத்து c)
{
	எனில் (c >= '0' && c <= '9')
		திரும்பு (c - '0');
	எனில் (c >= 'a' && c <= 'f')
		திரும்பு (c + 10 - 'a');
	எனில் (c >= 'A' && c <= 'F')
		திரும்பு (c + 10 - 'A');
	திரும்பு (-1);
}

நிலையான முழு hex2_bin (எழுத்து *s)
{
	முழு i, j;

	எனில் ((i = hex1_bin(*s++)) < 0) {
		திரும்பு (-1);
	}
	எனில் ((j = hex1_bin(*s)) < 0) {
		திரும்பு (-1);
	}

	திரும்பு ((i<<4) + j);
}
