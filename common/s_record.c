// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#ಅನ್ತರ್ಭಾವ <s_record.h>

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ hex1_bin (ಅಕ್ಷರ  c);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ hex2_bin (ಅಕ್ಷರ *s);

ಪೂರ್ಣಾಂಕ srec_decode (ಅಕ್ಷರ *ಒಳಹರಿವು, ಪೂರ್ಣಾಂಕ *ಎಣಿಕೆ_2, ulong *addr, ಅಕ್ಷರ *ದತ್ತಾಂಶ)
{
	ಪೂರ್ಣಾಂಕ	i;
	ಪೂರ್ಣಾಂಕ	v;				/* conversion buffer	*/
	ಪೂರ್ಣಾಂಕ	srec_type;			/* S-Record type	*/
	ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ chksum;			/* buffer for checksum	*/

	chksum = 0;

	/* skip anything before 'S', and the 'S' itself.
	 * Return error if not found
	 */

	ಗಾಗಿ (; *ಒಳಹರಿವು; ++ಒಳಹರಿವು) {
		ಆದರೆ (*ಒಳಹರಿವು == 'S') {		/* skip 'S' */
			++ಒಳಹರಿವು;
			ನಿಲ್ಲಿಸು;
		}
	}
	ಆದರೆ (*ಒಳಹರಿವು == '\0') {			/* no more data?	*/
		ಹಿಂತಿರುಗಿ (SREC_ಖಾಲಿಗಳು);
	}

	v = *ಒಳಹರಿವು++;				/* record type		*/

	ಆದರೆ ((*ಎಣಿಕೆ_2 = hex2_bin(ಒಳಹರಿವು)) < 0) {
		ಹಿಂತಿರುಗಿ (SREC_E_NOSREC);
	}

	chksum += *ಎಣಿಕೆ_2;
	ಒಳಹರಿವು  += 2;

	ಆಯ್ಕೆ (v) {				/* record type		*/

	ಸ್ಥಿತಿ '0':				/* start record		*/
		srec_type = SREC_ಆರಂಭ;		/* 2 byte addr field	*/
		*ಎಣಿಕೆ_2   -= 3;			/* - checksum and addr	*/
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ '1':
		srec_type = SREC_DATA2;		/* 2 byte addr field	*/
		*ಎಣಿಕೆ_2   -= 3;			/* - checksum and addr	*/
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ '2':
		srec_type = SREC_DATA3;		/* 3 byte addr field	*/
		*ಎಣಿಕೆ_2   -= 4;			/* - checksum and addr	*/
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ '3':				/* data record with a	*/
		srec_type = SREC_DATA4;		/* 4 byte addr field	*/
		*ಎಣಿಕೆ_2   -= 5;			/* - checksum and addr	*/
		ನಿಲ್ಲಿಸು;
/***	case '4'  ***/
	ಸ್ಥಿತಿ '5':			/* count record, addr field contains */
		srec_type = SREC_ಎಣಿಕೆ;	/* a 2 byte record counter	*/
		*ಎಣಿಕೆ_2    = 0;			/* no data		*/
		ನಿಲ್ಲಿಸು;
/***	case '6' -- not used  ***/
	ಸ್ಥಿತಿ '7':				/* end record with a	*/
		srec_type = SREC_END4;		/* 4 byte addr field	*/
		*ಎಣಿಕೆ_2   -= 5;			/* - checksum and addr	*/
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ '8':				/* end record with a	*/
		srec_type = SREC_END3;		/* 3 byte addr field	*/
		*ಎಣಿಕೆ_2   -= 4;			/* - checksum and addr	*/
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ '9':				/* end record with a	*/
		srec_type = SREC_END2;		/* 2 byte addr field	*/
		*ಎಣಿಕೆ_2   -= 3;			/* - checksum and addr	*/
		ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
		ಹಿಂತಿರುಗಿ (SREC_E_BADTYPE);
	}

	/* read address field */
	*addr = 0;

	ಆಯ್ಕೆ (v) {
	ಸ್ಥಿತಿ '3':				/* 4 byte addr field	*/
	ಸ್ಥಿತಿ '7':
		ಆದರೆ ((v = hex2_bin(ಒಳಹರಿವು)) < 0) {
			ಹಿಂತಿರುಗಿ (SREC_E_NOSREC);
		}
		*addr  += v;
		chksum += v;
		ಒಳಹರಿವು  += 2;
		/* FALL THRU */
	ಸ್ಥಿತಿ '2':				/* 3 byte addr field	*/
	ಸ್ಥಿತಿ '8':
		ಆದರೆ ((v = hex2_bin(ಒಳಹರಿವು)) < 0) {
			ಹಿಂತಿರುಗಿ (SREC_E_NOSREC);
		}
		*addr <<= 8;
		*addr  += v;
		chksum += v;
		ಒಳಹರಿವು  += 2;
		/* FALL THRU */
	ಸ್ಥಿತಿ '0':				/* 2 byte addr field	*/
	ಸ್ಥಿತಿ '1':
	ಸ್ಥಿತಿ '5':
	ಸ್ಥಿತಿ '9':
		ಆದರೆ ((v = hex2_bin(ಒಳಹರಿವು)) < 0) {
			ಹಿಂತಿರುಗಿ (SREC_E_NOSREC);
		}
		*addr <<= 8;
		*addr  += v;
		chksum += v;
		ಒಳಹರಿವು  += 2;

		ಆದರೆ ((v = hex2_bin(ಒಳಹರಿವು)) < 0) {
			ಹಿಂತಿರುಗಿ (SREC_E_NOSREC);
		}
		*addr <<= 8;
		*addr  += v;
		chksum += v;
		ಒಳಹರಿವು  += 2;

		ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
		ಹಿಂತಿರುಗಿ (SREC_E_BADTYPE);
	}

	/* convert data and calculate checksum */
	ಗಾಗಿ (i=0; i < *ಎಣಿಕೆ_2; ++i) {
		ಆದರೆ ((v = hex2_bin(ಒಳಹರಿವು)) < 0) {
			ಹಿಂತಿರುಗಿ (SREC_E_NOSREC);
		}
		ದತ್ತಾಂಶ[i] = v;
		chksum += v;
		ಒಳಹರಿವು  += 2;
	}

	/* read anc check checksum */
	ಆದರೆ ((v = hex2_bin(ಒಳಹರಿವು)) < 0) {
		ಹಿಂತಿರುಗಿ (SREC_E_NOSREC);
	}

	ಆದರೆ ((ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)v != (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)~chksum) {
		ಹಿಂತಿರುಗಿ (SREC_E_BADCHKS);
	}

	ಹಿಂತಿರುಗಿ (srec_type);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ hex1_bin (ಅಕ್ಷರ c)
{
	ಆದರೆ (c >= '0' && c <= '9')
		ಹಿಂತಿರುಗಿ (c - '0');
	ಆದರೆ (c >= 'a' && c <= 'f')
		ಹಿಂತಿರುಗಿ (c + 10 - 'a');
	ಆದರೆ (c >= 'A' && c <= 'F')
		ಹಿಂತಿರುಗಿ (c + 10 - 'A');
	ಹಿಂತಿರುಗಿ (-1);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ hex2_bin (ಅಕ್ಷರ *s)
{
	ಪೂರ್ಣಾಂಕ i, j;

	ಆದರೆ ((i = hex1_bin(*s++)) < 0) {
		ಹಿಂತಿರುಗಿ (-1);
	}
	ಆದರೆ ((j = hex1_bin(*s)) < 0) {
		ಹಿಂತಿರುಗಿ (-1);
	}

	ಹಿಂತಿರುಗಿ ((i<<4) + j);
}
