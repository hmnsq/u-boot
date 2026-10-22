// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#شامل <s_record.h>

ایستا صحیح hex1_bin (نویسه  c);
ایستا صحیح hex2_bin (نویسه *s);

صحیح srec_decode (نویسه *درون_داد, صحیح *شمار, ulong *addr, نویسه *data)
{
	صحیح	i;
	صحیح	v;				/* conversion buffer	*/
	صحیح	srec_type;			/* S-Record type	*/
	unsigned نویسه chksum;			/* buffer for checksum	*/

	chksum = 0;

	/* skip anything before 'S', and the 'S' itself.
	 * Return error if not found
	 */

	برای (; *درون_داد; ++درون_داد) {
		اگر (*درون_داد == 'S') {		/* skip 'S' */
			++درون_داد;
			بشکن;
		}
	}
	اگر (*درون_داد == '\0') {			/* no more data?	*/
		بازگشت (SREC_EMPTY);
	}

	v = *درون_داد++;				/* record type		*/

	اگر ((*شمار = hex2_bin(درون_داد)) < 0) {
		بازگشت (SREC_E_NOSREC);
	}

	chksum += *شمار;
	درون_داد  += 2;

	انتخاب (v) {				/* record type		*/

	حالت '0':				/* start record		*/
		srec_type = SREC_START;		/* 2 byte addr field	*/
		*شمار   -= 3;			/* - checksum and addr	*/
		بشکن;
	حالت '1':
		srec_type = SREC_DATA2;		/* 2 byte addr field	*/
		*شمار   -= 3;			/* - checksum and addr	*/
		بشکن;
	حالت '2':
		srec_type = SREC_DATA3;		/* 3 byte addr field	*/
		*شمار   -= 4;			/* - checksum and addr	*/
		بشکن;
	حالت '3':				/* data record with a	*/
		srec_type = SREC_DATA4;		/* 4 byte addr field	*/
		*شمار   -= 5;			/* - checksum and addr	*/
		بشکن;
/***	case '4'  ***/
	حالت '5':			/* count record, addr field contains */
		srec_type = SREC_شمار;	/* a 2 byte record counter	*/
		*شمار    = 0;			/* no data		*/
		بشکن;
/***	case '6' -- not used  ***/
	حالت '7':				/* end record with a	*/
		srec_type = SREC_END4;		/* 4 byte addr field	*/
		*شمار   -= 5;			/* - checksum and addr	*/
		بشکن;
	حالت '8':				/* end record with a	*/
		srec_type = SREC_END3;		/* 3 byte addr field	*/
		*شمار   -= 4;			/* - checksum and addr	*/
		بشکن;
	حالت '9':				/* end record with a	*/
		srec_type = SREC_END2;		/* 2 byte addr field	*/
		*شمار   -= 3;			/* - checksum and addr	*/
		بشکن;
	پیشفرض:
		بازگشت (SREC_E_BADTYPE);
	}

	/* read address field */
	*addr = 0;

	انتخاب (v) {
	حالت '3':				/* 4 byte addr field	*/
	حالت '7':
		اگر ((v = hex2_bin(درون_داد)) < 0) {
			بازگشت (SREC_E_NOSREC);
		}
		*addr  += v;
		chksum += v;
		درون_داد  += 2;
		/* FALL THRU */
	حالت '2':				/* 3 byte addr field	*/
	حالت '8':
		اگر ((v = hex2_bin(درون_داد)) < 0) {
			بازگشت (SREC_E_NOSREC);
		}
		*addr <<= 8;
		*addr  += v;
		chksum += v;
		درون_داد  += 2;
		/* FALL THRU */
	حالت '0':				/* 2 byte addr field	*/
	حالت '1':
	حالت '5':
	حالت '9':
		اگر ((v = hex2_bin(درون_داد)) < 0) {
			بازگشت (SREC_E_NOSREC);
		}
		*addr <<= 8;
		*addr  += v;
		chksum += v;
		درون_داد  += 2;

		اگر ((v = hex2_bin(درون_داد)) < 0) {
			بازگشت (SREC_E_NOSREC);
		}
		*addr <<= 8;
		*addr  += v;
		chksum += v;
		درون_داد  += 2;

		بشکن;
	پیشفرض:
		بازگشت (SREC_E_BADTYPE);
	}

	/* convert data and calculate checksum */
	برای (i=0; i < *شمار; ++i) {
		اگر ((v = hex2_bin(درون_داد)) < 0) {
			بازگشت (SREC_E_NOSREC);
		}
		data[i] = v;
		chksum += v;
		درون_داد  += 2;
	}

	/* read anc check checksum */
	اگر ((v = hex2_bin(درون_داد)) < 0) {
		بازگشت (SREC_E_NOSREC);
	}

	اگر ((unsigned نویسه)v != (unsigned نویسه)~chksum) {
		بازگشت (SREC_E_BADCHKS);
	}

	بازگشت (srec_type);
}

ایستا صحیح hex1_bin (نویسه c)
{
	اگر (c >= '0' && c <= '9')
		بازگشت (c - '0');
	اگر (c >= 'a' && c <= 'f')
		بازگشت (c + 10 - 'a');
	اگر (c >= 'A' && c <= 'F')
		بازگشت (c + 10 - 'A');
	بازگشت (-1);
}

ایستا صحیح hex2_bin (نویسه *s)
{
	صحیح i, j;

	اگر ((i = hex1_bin(*s++)) < 0) {
		بازگشت (-1);
	}
	اگر ((j = hex1_bin(*s)) < 0) {
		بازگشت (-1);
	}

	بازگشت ((i<<4) + j);
}
