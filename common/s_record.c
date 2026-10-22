// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#ضمن <s_record.h>

ساكن صحيح hex1_bin (حرف  c);
ساكن صحيح hex2_bin (حرف *s);

صحيح srec_decode (حرف *درون_داد, صحيح *شمار, ulong *addr, حرف *داده)
{
	صحيح	i;
	صحيح	v;				/* conversion buffer	*/
	صحيح	srec_type;			/* S-Record type	*/
	بیعلامت حرف chksum;			/* buffer for checksum	*/

	chksum = 0;

	/* skip anything before 'S', and the 'S' itself.
	 * Return error if not found
	 */

	لكل (; *درون_داد; ++درون_داد) {
		إذا (*درون_داد == 'S') {		/* skip 'S' */
			++درون_داد;
			اكسر;
		}
	}
	إذا (*درون_داد == '\0') {			/* no more data?	*/
		أرجع (SREC_خالی);
	}

	v = *درون_داد++;				/* record type		*/

	إذا ((*شمار = hex2_bin(درون_داد)) < 0) {
		أرجع (SREC_E_NOSREC);
	}

	chksum += *شمار;
	درون_داد  += 2;

	بدل (v) {				/* record type		*/

	حالة '0':				/* start record		*/
		srec_type = SREC_شروع;		/* 2 byte addr field	*/
		*شمار   -= 3;			/* - checksum and addr	*/
		اكسر;
	حالة '1':
		srec_type = SREC_DATA2;		/* 2 byte addr field	*/
		*شمار   -= 3;			/* - checksum and addr	*/
		اكسر;
	حالة '2':
		srec_type = SREC_DATA3;		/* 3 byte addr field	*/
		*شمار   -= 4;			/* - checksum and addr	*/
		اكسر;
	حالة '3':				/* data record with a	*/
		srec_type = SREC_DATA4;		/* 4 byte addr field	*/
		*شمار   -= 5;			/* - checksum and addr	*/
		اكسر;
/***	case '4'  ***/
	حالة '5':			/* count record, addr field contains */
		srec_type = SREC_شمار;	/* a 2 byte record counter	*/
		*شمار    = 0;			/* no data		*/
		اكسر;
/***	case '6' -- not used  ***/
	حالة '7':				/* end record with a	*/
		srec_type = SREC_END4;		/* 4 byte addr field	*/
		*شمار   -= 5;			/* - checksum and addr	*/
		اكسر;
	حالة '8':				/* end record with a	*/
		srec_type = SREC_END3;		/* 3 byte addr field	*/
		*شمار   -= 4;			/* - checksum and addr	*/
		اكسر;
	حالة '9':				/* end record with a	*/
		srec_type = SREC_END2;		/* 2 byte addr field	*/
		*شمار   -= 3;			/* - checksum and addr	*/
		اكسر;
	افتراضي:
		أرجع (SREC_E_BADTYPE);
	}

	/* read address field */
	*addr = 0;

	بدل (v) {
	حالة '3':				/* 4 byte addr field	*/
	حالة '7':
		إذا ((v = hex2_bin(درون_داد)) < 0) {
			أرجع (SREC_E_NOSREC);
		}
		*addr  += v;
		chksum += v;
		درون_داد  += 2;
		/* FALL THRU */
	حالة '2':				/* 3 byte addr field	*/
	حالة '8':
		إذا ((v = hex2_bin(درون_داد)) < 0) {
			أرجع (SREC_E_NOSREC);
		}
		*addr <<= 8;
		*addr  += v;
		chksum += v;
		درون_داد  += 2;
		/* FALL THRU */
	حالة '0':				/* 2 byte addr field	*/
	حالة '1':
	حالة '5':
	حالة '9':
		إذا ((v = hex2_bin(درون_داد)) < 0) {
			أرجع (SREC_E_NOSREC);
		}
		*addr <<= 8;
		*addr  += v;
		chksum += v;
		درون_داد  += 2;

		إذا ((v = hex2_bin(درون_داد)) < 0) {
			أرجع (SREC_E_NOSREC);
		}
		*addr <<= 8;
		*addr  += v;
		chksum += v;
		درون_داد  += 2;

		اكسر;
	افتراضي:
		أرجع (SREC_E_BADTYPE);
	}

	/* convert data and calculate checksum */
	لكل (i=0; i < *شمار; ++i) {
		إذا ((v = hex2_bin(درون_داد)) < 0) {
			أرجع (SREC_E_NOSREC);
		}
		داده[i] = v;
		chksum += v;
		درون_داد  += 2;
	}

	/* read anc check checksum */
	إذا ((v = hex2_bin(درون_داد)) < 0) {
		أرجع (SREC_E_NOSREC);
	}

	إذا ((بیعلامت حرف)v != (بیعلامت حرف)~chksum) {
		أرجع (SREC_E_BADCHKS);
	}

	أرجع (srec_type);
}

ساكن صحيح hex1_bin (حرف c)
{
	إذا (c >= '0' && c <= '9')
		أرجع (c - '0');
	إذا (c >= 'a' && c <= 'f')
		أرجع (c + 10 - 'a');
	إذا (c >= 'A' && c <= 'F')
		أرجع (c + 10 - 'A');
	أرجع (-1);
}

ساكن صحيح hex2_bin (حرف *s)
{
	صحيح i, j;

	إذا ((i = hex1_bin(*s++)) < 0) {
		أرجع (-1);
	}
	إذا ((j = hex1_bin(*s)) < 0) {
		أرجع (-1);
	}

	أرجع ((i<<4) + j);
}
