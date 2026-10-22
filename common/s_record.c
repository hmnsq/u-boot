// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#ଅନ୍ତର୍ଭାଵ <s_record.h>

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା hex1_bin (ଅକ୍ଷର  c);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା hex2_bin (ଅକ୍ଷର *s);

ପୂର୍ଣ୍ଣସଂଖ୍ୟା srec_decode (ଅକ୍ଷର *ନିବେଶ, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *ଗଣନା_2, ulong *addr, ଅକ୍ଷର *data)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା	i;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା	v;				/* conversion buffer	*/
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା	srec_type;			/* S-Record type	*/
	unsigned ଅକ୍ଷର chksum;			/* buffer for checksum	*/

	chksum = 0;

	/* skip anything before 'S', and the 'S' itself.
	 * Return error if not found
	 */

	ପାଇଁ (; *ନିବେଶ; ++ନିବେଶ) {
		ଯଦି (*ନିବେଶ == 'S') {		/* skip 'S' */
			++ନିବେଶ;
			ଭାଙ୍ଗ;
		}
	}
	ଯଦି (*ନିବେଶ == '\0') {			/* no more data?	*/
		ଫେରନ୍ତୁ (SREC_EMPTY);
	}

	v = *ନିବେଶ++;				/* record type		*/

	ଯଦି ((*ଗଣନା_2 = hex2_bin(ନିବେଶ)) < 0) {
		ଫେରନ୍ତୁ (SREC_E_NOSREC);
	}

	chksum += *ଗଣନା_2;
	ନିବେଶ  += 2;

	ବାଛ (v) {				/* record type		*/

	ସ୍ଥିତି '0':				/* start record		*/
		srec_type = SREC_START;		/* 2 byte addr field	*/
		*ଗଣନା_2   -= 3;			/* - checksum and addr	*/
		ଭାଙ୍ଗ;
	ସ୍ଥିତି '1':
		srec_type = SREC_DATA2;		/* 2 byte addr field	*/
		*ଗଣନା_2   -= 3;			/* - checksum and addr	*/
		ଭାଙ୍ଗ;
	ସ୍ଥିତି '2':
		srec_type = SREC_DATA3;		/* 3 byte addr field	*/
		*ଗଣନା_2   -= 4;			/* - checksum and addr	*/
		ଭାଙ୍ଗ;
	ସ୍ଥିତି '3':				/* data record with a	*/
		srec_type = SREC_DATA4;		/* 4 byte addr field	*/
		*ଗଣନା_2   -= 5;			/* - checksum and addr	*/
		ଭାଙ୍ଗ;
/***	case '4'  ***/
	ସ୍ଥିତି '5':			/* count record, addr field contains */
		srec_type = SREC_ଗଣନା;	/* a 2 byte record counter	*/
		*ଗଣନା_2    = 0;			/* no data		*/
		ଭାଙ୍ଗ;
/***	case '6' -- not used  ***/
	ସ୍ଥିତି '7':				/* end record with a	*/
		srec_type = SREC_END4;		/* 4 byte addr field	*/
		*ଗଣନା_2   -= 5;			/* - checksum and addr	*/
		ଭାଙ୍ଗ;
	ସ୍ଥିତି '8':				/* end record with a	*/
		srec_type = SREC_END3;		/* 3 byte addr field	*/
		*ଗଣନା_2   -= 4;			/* - checksum and addr	*/
		ଭାଙ୍ଗ;
	ସ୍ଥିତି '9':				/* end record with a	*/
		srec_type = SREC_END2;		/* 2 byte addr field	*/
		*ଗଣନା_2   -= 3;			/* - checksum and addr	*/
		ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ଫେରନ୍ତୁ (SREC_E_BADTYPE);
	}

	/* read address field */
	*addr = 0;

	ବାଛ (v) {
	ସ୍ଥିତି '3':				/* 4 byte addr field	*/
	ସ୍ଥିତି '7':
		ଯଦି ((v = hex2_bin(ନିବେଶ)) < 0) {
			ଫେରନ୍ତୁ (SREC_E_NOSREC);
		}
		*addr  += v;
		chksum += v;
		ନିବେଶ  += 2;
		/* FALL THRU */
	ସ୍ଥିତି '2':				/* 3 byte addr field	*/
	ସ୍ଥିତି '8':
		ଯଦି ((v = hex2_bin(ନିବେଶ)) < 0) {
			ଫେରନ୍ତୁ (SREC_E_NOSREC);
		}
		*addr <<= 8;
		*addr  += v;
		chksum += v;
		ନିବେଶ  += 2;
		/* FALL THRU */
	ସ୍ଥିତି '0':				/* 2 byte addr field	*/
	ସ୍ଥିତି '1':
	ସ୍ଥିତି '5':
	ସ୍ଥିତି '9':
		ଯଦି ((v = hex2_bin(ନିବେଶ)) < 0) {
			ଫେରନ୍ତୁ (SREC_E_NOSREC);
		}
		*addr <<= 8;
		*addr  += v;
		chksum += v;
		ନିବେଶ  += 2;

		ଯଦି ((v = hex2_bin(ନିବେଶ)) < 0) {
			ଫେରନ୍ତୁ (SREC_E_NOSREC);
		}
		*addr <<= 8;
		*addr  += v;
		chksum += v;
		ନିବେଶ  += 2;

		ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ଫେରନ୍ତୁ (SREC_E_BADTYPE);
	}

	/* convert data and calculate checksum */
	ପାଇଁ (i=0; i < *ଗଣନା_2; ++i) {
		ଯଦି ((v = hex2_bin(ନିବେଶ)) < 0) {
			ଫେରନ୍ତୁ (SREC_E_NOSREC);
		}
		data[i] = v;
		chksum += v;
		ନିବେଶ  += 2;
	}

	/* read anc check checksum */
	ଯଦି ((v = hex2_bin(ନିବେଶ)) < 0) {
		ଫେରନ୍ତୁ (SREC_E_NOSREC);
	}

	ଯଦି ((unsigned ଅକ୍ଷର)v != (unsigned ଅକ୍ଷର)~chksum) {
		ଫେରନ୍ତୁ (SREC_E_BADCHKS);
	}

	ଫେରନ୍ତୁ (srec_type);
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା hex1_bin (ଅକ୍ଷର c)
{
	ଯଦି (c >= '0' && c <= '9')
		ଫେରନ୍ତୁ (c - '0');
	ଯଦି (c >= 'a' && c <= 'f')
		ଫେରନ୍ତୁ (c + 10 - 'a');
	ଯଦି (c >= 'A' && c <= 'F')
		ଫେରନ୍ତୁ (c + 10 - 'A');
	ଫେରନ୍ତୁ (-1);
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା hex2_bin (ଅକ୍ଷର *s)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, j;

	ଯଦି ((i = hex1_bin(*s++)) < 0) {
		ଫେରନ୍ତୁ (-1);
	}
	ଯଦି ((j = hex1_bin(*s)) < 0) {
		ଫେରନ୍ତୁ (-1);
	}

	ଫେରନ୍ତୁ ((i<<4) + j);
}
