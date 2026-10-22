// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#شامل <s_record.h>

ساکن صحیح hex1_bin (حرف  c);
ساکن صحیح hex2_bin (حرف *s);

صحیح srec_decode (حرف *داخلہ, صحیح *گنتی, ulong *addr, حرف *ڈیٹا)
{
	صحیح	i;
	صحیح	v;				/* conversion buffer	*/
	صحیح	srec_type;			/* S-Record type	*/
	غیرعلامتی حرف chksum;			/* buffer for checksum	*/

	chksum = 0;

	/* skip anything before 'S', and the 'S' itself.
	 * Return error if not found
	 */

	برائے (; *داخلہ; ++داخلہ) {
		اگر (*داخلہ == 'S') {		/* skip 'S' */
			++داخلہ;
			توڑو;
		}
	}
	اگر (*داخلہ == '\0') {			/* no more data?	*/
		واپس (SREC_خالی_خانے);
	}

	v = *داخلہ++;				/* record type		*/

	اگر ((*گنتی = hex2_bin(داخلہ)) < 0) {
		واپس (SREC_E_NOSREC);
	}

	chksum += *گنتی;
	داخلہ  += 2;

	انتخاب (v) {				/* record type		*/

	حالت '0':				/* start record		*/
		srec_type = SREC_آغاز;		/* 2 byte addr field	*/
		*گنتی   -= 3;			/* - checksum and addr	*/
		توڑو;
	حالت '1':
		srec_type = SREC_DATA2;		/* 2 byte addr field	*/
		*گنتی   -= 3;			/* - checksum and addr	*/
		توڑو;
	حالت '2':
		srec_type = SREC_DATA3;		/* 3 byte addr field	*/
		*گنتی   -= 4;			/* - checksum and addr	*/
		توڑو;
	حالت '3':				/* data record with a	*/
		srec_type = SREC_DATA4;		/* 4 byte addr field	*/
		*گنتی   -= 5;			/* - checksum and addr	*/
		توڑو;
/***	case '4'  ***/
	حالت '5':			/* count record, addr field contains */
		srec_type = SREC_گنتی;	/* a 2 byte record counter	*/
		*گنتی    = 0;			/* no data		*/
		توڑو;
/***	case '6' -- not used  ***/
	حالت '7':				/* end record with a	*/
		srec_type = SREC_END4;		/* 4 byte addr field	*/
		*گنتی   -= 5;			/* - checksum and addr	*/
		توڑو;
	حالت '8':				/* end record with a	*/
		srec_type = SREC_END3;		/* 3 byte addr field	*/
		*گنتی   -= 4;			/* - checksum and addr	*/
		توڑو;
	حالت '9':				/* end record with a	*/
		srec_type = SREC_END2;		/* 2 byte addr field	*/
		*گنتی   -= 3;			/* - checksum and addr	*/
		توڑو;
	طےشدہ:
		واپس (SREC_E_BADTYPE);
	}

	/* read address field */
	*addr = 0;

	انتخاب (v) {
	حالت '3':				/* 4 byte addr field	*/
	حالت '7':
		اگر ((v = hex2_bin(داخلہ)) < 0) {
			واپس (SREC_E_NOSREC);
		}
		*addr  += v;
		chksum += v;
		داخلہ  += 2;
		/* FALL THRU */
	حالت '2':				/* 3 byte addr field	*/
	حالت '8':
		اگر ((v = hex2_bin(داخلہ)) < 0) {
			واپس (SREC_E_NOSREC);
		}
		*addr <<= 8;
		*addr  += v;
		chksum += v;
		داخلہ  += 2;
		/* FALL THRU */
	حالت '0':				/* 2 byte addr field	*/
	حالت '1':
	حالت '5':
	حالت '9':
		اگر ((v = hex2_bin(داخلہ)) < 0) {
			واپس (SREC_E_NOSREC);
		}
		*addr <<= 8;
		*addr  += v;
		chksum += v;
		داخلہ  += 2;

		اگر ((v = hex2_bin(داخلہ)) < 0) {
			واپس (SREC_E_NOSREC);
		}
		*addr <<= 8;
		*addr  += v;
		chksum += v;
		داخلہ  += 2;

		توڑو;
	طےشدہ:
		واپس (SREC_E_BADTYPE);
	}

	/* convert data and calculate checksum */
	برائے (i=0; i < *گنتی; ++i) {
		اگر ((v = hex2_bin(داخلہ)) < 0) {
			واپس (SREC_E_NOSREC);
		}
		ڈیٹا[i] = v;
		chksum += v;
		داخلہ  += 2;
	}

	/* read anc check checksum */
	اگر ((v = hex2_bin(داخلہ)) < 0) {
		واپس (SREC_E_NOSREC);
	}

	اگر ((غیرعلامتی حرف)v != (غیرعلامتی حرف)~chksum) {
		واپس (SREC_E_BADCHKS);
	}

	واپس (srec_type);
}

ساکن صحیح hex1_bin (حرف c)
{
	اگر (c >= '0' && c <= '9')
		واپس (c - '0');
	اگر (c >= 'a' && c <= 'f')
		واپس (c + 10 - 'a');
	اگر (c >= 'A' && c <= 'F')
		واپس (c + 10 - 'A');
	واپس (-1);
}

ساکن صحیح hex2_bin (حرف *s)
{
	صحیح i, j;

	اگر ((i = hex1_bin(*s++)) < 0) {
		واپس (-1);
	}
	اگر ((j = hex1_bin(*s)) < 0) {
		واپس (-1);
	}

	واپس ((i<<4) + j);
}
