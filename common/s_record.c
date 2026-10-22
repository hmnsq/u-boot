// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#ಅನ್ತರ್ಭಾವ <s_record.h>

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ hex1_bin (ವರ್ಣ  c);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ hex2_bin (ವರ್ಣ *s);

ಪೂರ್ಣಾಙ್ಕ srec_decode (ವರ್ಣ *ಒಳಹರಿವು, ಪೂರ್ಣಾಙ್ಕ *ಎಣಿಕೆ, ulong *addr, ವರ್ಣ *data)
{
	ಪೂರ್ಣಾಙ್ಕ	i;
	ಪೂರ್ಣಾಙ್ಕ	v;				/* conversion buffer	*/
	ಪೂರ್ಣಾಙ್ಕ	srec_type;			/* S-Record type	*/
	unsigned ವರ್ಣ chksum;			/* buffer for checksum	*/

	chksum = 0;

	/* skip anything before 'S', and the 'S' itself.
	 * Return error if not found
	 */

	ಪ್ರತಿ (; *ಒಳಹರಿವು; ++ಒಳಹರಿವು) {
		ಯದಿ (*ಒಳಹರಿವು == 'S') {		/* skip 'S' */
			++ಒಳಹರಿವು;
			ಭಙ್ಗ;
		}
	}
	ಯದಿ (*ಒಳಹರಿವು == '\0') {			/* no more data?	*/
		ಪ್ರತ್ಯಾಗಮ (SREC_EMPTY);
	}

	v = *ಒಳಹರಿವು++;				/* record type		*/

	ಯದಿ ((*ಎಣಿಕೆ = hex2_bin(ಒಳಹರಿವು)) < 0) {
		ಪ್ರತ್ಯಾಗಮ (SREC_E_NOSREC);
	}

	chksum += *ಎಣಿಕೆ;
	ಒಳಹರಿವು  += 2;

	ವಿಕಲ್ಪ (v) {				/* record type		*/

	ಪಕ್ಷ '0':				/* start record		*/
		srec_type = SREC_START;		/* 2 byte addr field	*/
		*ಎಣಿಕೆ   -= 3;			/* - checksum and addr	*/
		ಭಙ್ಗ;
	ಪಕ್ಷ '1':
		srec_type = SREC_DATA2;		/* 2 byte addr field	*/
		*ಎಣಿಕೆ   -= 3;			/* - checksum and addr	*/
		ಭಙ್ಗ;
	ಪಕ್ಷ '2':
		srec_type = SREC_DATA3;		/* 3 byte addr field	*/
		*ಎಣಿಕೆ   -= 4;			/* - checksum and addr	*/
		ಭಙ್ಗ;
	ಪಕ್ಷ '3':				/* data record with a	*/
		srec_type = SREC_DATA4;		/* 4 byte addr field	*/
		*ಎಣಿಕೆ   -= 5;			/* - checksum and addr	*/
		ಭಙ್ಗ;
/***	case '4'  ***/
	ಪಕ್ಷ '5':			/* count record, addr field contains */
		srec_type = SREC_ಎಣಿಕೆ;	/* a 2 byte record counter	*/
		*ಎಣಿಕೆ    = 0;			/* no data		*/
		ಭಙ್ಗ;
/***	case '6' -- not used  ***/
	ಪಕ್ಷ '7':				/* end record with a	*/
		srec_type = SREC_END4;		/* 4 byte addr field	*/
		*ಎಣಿಕೆ   -= 5;			/* - checksum and addr	*/
		ಭಙ್ಗ;
	ಪಕ್ಷ '8':				/* end record with a	*/
		srec_type = SREC_END3;		/* 3 byte addr field	*/
		*ಎಣಿಕೆ   -= 4;			/* - checksum and addr	*/
		ಭಙ್ಗ;
	ಪಕ್ಷ '9':				/* end record with a	*/
		srec_type = SREC_END2;		/* 2 byte addr field	*/
		*ಎಣಿಕೆ   -= 3;			/* - checksum and addr	*/
		ಭಙ್ಗ;
	ಸ್ವಭಾವ:
		ಪ್ರತ್ಯಾಗಮ (SREC_E_BADTYPE);
	}

	/* read address field */
	*addr = 0;

	ವಿಕಲ್ಪ (v) {
	ಪಕ್ಷ '3':				/* 4 byte addr field	*/
	ಪಕ್ಷ '7':
		ಯದಿ ((v = hex2_bin(ಒಳಹರಿವು)) < 0) {
			ಪ್ರತ್ಯಾಗಮ (SREC_E_NOSREC);
		}
		*addr  += v;
		chksum += v;
		ಒಳಹರಿವು  += 2;
		/* FALL THRU */
	ಪಕ್ಷ '2':				/* 3 byte addr field	*/
	ಪಕ್ಷ '8':
		ಯದಿ ((v = hex2_bin(ಒಳಹರಿವು)) < 0) {
			ಪ್ರತ್ಯಾಗಮ (SREC_E_NOSREC);
		}
		*addr <<= 8;
		*addr  += v;
		chksum += v;
		ಒಳಹರಿವು  += 2;
		/* FALL THRU */
	ಪಕ್ಷ '0':				/* 2 byte addr field	*/
	ಪಕ್ಷ '1':
	ಪಕ್ಷ '5':
	ಪಕ್ಷ '9':
		ಯದಿ ((v = hex2_bin(ಒಳಹರಿವು)) < 0) {
			ಪ್ರತ್ಯಾಗಮ (SREC_E_NOSREC);
		}
		*addr <<= 8;
		*addr  += v;
		chksum += v;
		ಒಳಹರಿವು  += 2;

		ಯದಿ ((v = hex2_bin(ಒಳಹರಿವು)) < 0) {
			ಪ್ರತ್ಯಾಗಮ (SREC_E_NOSREC);
		}
		*addr <<= 8;
		*addr  += v;
		chksum += v;
		ಒಳಹರಿವು  += 2;

		ಭಙ್ಗ;
	ಸ್ವಭಾವ:
		ಪ್ರತ್ಯಾಗಮ (SREC_E_BADTYPE);
	}

	/* convert data and calculate checksum */
	ಪ್ರತಿ (i=0; i < *ಎಣಿಕೆ; ++i) {
		ಯದಿ ((v = hex2_bin(ಒಳಹರಿವು)) < 0) {
			ಪ್ರತ್ಯಾಗಮ (SREC_E_NOSREC);
		}
		data[i] = v;
		chksum += v;
		ಒಳಹರಿವು  += 2;
	}

	/* read anc check checksum */
	ಯದಿ ((v = hex2_bin(ಒಳಹರಿವು)) < 0) {
		ಪ್ರತ್ಯಾಗಮ (SREC_E_NOSREC);
	}

	ಯದಿ ((unsigned ವರ್ಣ)v != (unsigned ವರ್ಣ)~chksum) {
		ಪ್ರತ್ಯಾಗಮ (SREC_E_BADCHKS);
	}

	ಪ್ರತ್ಯಾಗಮ (srec_type);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ hex1_bin (ವರ್ಣ c)
{
	ಯದಿ (c >= '0' && c <= '9')
		ಪ್ರತ್ಯಾಗಮ (c - '0');
	ಯದಿ (c >= 'a' && c <= 'f')
		ಪ್ರತ್ಯಾಗಮ (c + 10 - 'a');
	ಯದಿ (c >= 'A' && c <= 'F')
		ಪ್ರತ್ಯಾಗಮ (c + 10 - 'A');
	ಪ್ರತ್ಯಾಗಮ (-1);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ hex2_bin (ವರ್ಣ *s)
{
	ಪೂರ್ಣಾಙ್ಕ i, j;

	ಯದಿ ((i = hex1_bin(*s++)) < 0) {
		ಪ್ರತ್ಯಾಗಮ (-1);
	}
	ಯದಿ ((j = hex1_bin(*s)) < 0) {
		ಪ್ರತ್ಯಾಗಮ (-1);
	}

	ಪ್ರತ್ಯಾಗಮ ((i<<4) + j);
}
