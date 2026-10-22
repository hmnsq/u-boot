// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2009-2016 CompuLab, Ltd.
 *
 * Authors: Nikita Kiryanov <nikita@compulab.co.il>
 *	    Igor Grinberg <grinberg@compulab.co.il>
 */

#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <vsprintf.h>
#ಅನ್ತರ್ಭಾವ <linux/string.h>
#ಅನ್ತರ್ಭಾವ <eeprom_field.h>

ಸ್ಥಿರ ಶೂನ್ಯ __eeprom_field_print_bin(ಸ್ಥಿರಾಂಕ ರಚನೆ eeprom_ಕ್ಷೇತ್ರ *ಕ್ಷೇತ್ರ,
				     ಅಕ್ಷರ *delimiter, bool ಹಿಮ್ಮುಖ)
{
	ಪೂರ್ಣಾಂಕ i;
	ಪೂರ್ಣಾಂಕ from = ಹಿಮ್ಮುಖ ? ಕ್ಷೇತ್ರ->ಗಾತ್ರ_2 - 1 : 0;
	ಪೂರ್ಣಾಂಕ ಗಮ್ಯ = ಹಿಮ್ಮುಖ ? 0 : ಕ್ಷೇತ್ರ->ಗಾತ್ರ_2 - 1;

	ಬರೆ(PRINT_ಕ್ಷೇತ್ರ_SEGMENT, ಕ್ಷೇತ್ರ->name);
	ಗಾಗಿ (i = from; i != ಗಮ್ಯ; ಹಿಮ್ಮುಖ ? i-- : i++)
		ಬರೆ("%02x%s", ಕ್ಷೇತ್ರ->buf[i], delimiter);

	ಬರೆ("%02x\n", ಕ್ಷೇತ್ರ->buf[i]);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ __eeprom_field_update_bin(ರಚನೆ eeprom_ಕ್ಷೇತ್ರ *ಕ್ಷೇತ್ರ,
				     ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮೌಲ್ಯ, bool ಹಿಮ್ಮುಖ)
{
	ಪೂರ್ಣಾಂಕ len = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(ಮೌಲ್ಯ);
	ಪೂರ್ಣಾಂಕ k, j, i = ಹಿಮ್ಮುಖ ? len - 1 : 0;
	ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ byte;
	ಅಕ್ಷರ *endptr;

	/* each two characters in the string fit in one byte */
	ಆದರೆ (len > ಕ್ಷೇತ್ರ->ಗಾತ್ರ_2 * 2)
		ಹಿಂತಿರುಗಿ -1;

	memset(ಕ್ಷೇತ್ರ->buf, 0, ಕ್ಷೇತ್ರ->ಗಾತ್ರ_2);

	/* i - string iterator, j - buf iterator */
	ಗಾಗಿ (j = 0; j < ಕ್ಷೇತ್ರ->ಗಾತ್ರ_2; j++) {
		byte = 0;
		ಅಕ್ಷರ tmp[3] = { 0, 0, 0 };

		ಆದರೆ ((ಹಿಮ್ಮುಖ && i < 0) || (!ಹಿಮ್ಮುಖ && i >= len))
			ನಿಲ್ಲಿಸು;

		ಗಾಗಿ (k = 0; k < 2; k++) {
			ಆದರೆ (ಹಿಮ್ಮುಖ && i == 0) {
				tmp[k] = ಮೌಲ್ಯ[i];
				ನಿಲ್ಲಿಸು;
			}

			tmp[k] = ಮೌಲ್ಯ[ಹಿಮ್ಮುಖ ? i - 1 + k : i + k];
		}

		byte = simple_strtoul(tmp, &endptr, 16);
		ಆದರೆ (*endptr != '\0')
			ಹಿಂತಿರುಗಿ -1;

		ಕ್ಷೇತ್ರ->buf[j] = byte;
		i = ಹಿಮ್ಮುಖ ? i - 2 : i + 2;
	}

	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ __eeprom_field_update_bin_delim(ರಚನೆ eeprom_ಕ್ಷೇತ್ರ *ಕ್ಷೇತ್ರ,
					   ಅಕ್ಷರ *ಮೌಲ್ಯ, ಅಕ್ಷರ *delimiter)
{
	ಪೂರ್ಣಾಂಕ ಎಣಿಕೆ_2 = 0;
	ಪೂರ್ಣಾಂಕ i, val;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *tmp = ಮೌಲ್ಯ;
	ಅಕ್ಷರ *tok;
	ಅಕ್ಷರ *endptr;

	tmp = ಅಕ್ಷರಸರಣಿ_ಹುಡುಕು(tmp, delimiter);
	ಆಗ (tmp != ಖಾಲಿ) {
		ಎಣಿಕೆ_2++;
		tmp++;
		tmp = ಅಕ್ಷರಸರಣಿ_ಹುಡುಕು(tmp, delimiter);
	}

	ಆದರೆ (ಎಣಿಕೆ_2 > ಕ್ಷೇತ್ರ->ಗಾತ್ರ_2)
		ಹಿಂತಿರುಗಿ -1;

	tok = strtok(ಮೌಲ್ಯ, delimiter);
	ಗಾಗಿ (i = 0; tok && i < ಕ್ಷೇತ್ರ->ಗಾತ್ರ_2; i++) {
		val = simple_strtoul(tok, &endptr, 0);
		ಆದರೆ (*endptr != '\0')
			ಹಿಂತಿರುಗಿ -1;

		/* here we assume that each tok is no more than byte long */
		ಕ್ಷೇತ್ರ->buf[i] = (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)val;
		tok = strtok(ಖಾಲಿ, delimiter);
	}

	ಹಿಂತಿರುಗಿ 0;
}

/**
 * eeprom_field_print_bin() - print a field which contains binary data
 *
 * Treat the field data as simple binary data, and print it as two digit
 * hexadecimal values.
 * Sample output:
 *      Field Name       0102030405060708090a
 *
 * @field:	an initialized field to print
 */
ಶೂನ್ಯ eeprom_ಕ್ಷೇತ್ರ_print_bin(ಸ್ಥಿರಾಂಕ ರಚನೆ eeprom_ಕ್ಷೇತ್ರ *ಕ್ಷೇತ್ರ)
{
	__eeprom_field_print_bin(ಕ್ಷೇತ್ರ, "", false);
}

/**
 * eeprom_field_update_bin() - Update field with new data in binary form
 *
 * @field:	an initialized field
 * @value:	a string of values (i.e. "10b234a")
 */
ಪೂರ್ಣಾಂಕ eeprom_ಕ್ಷೇತ್ರ_update_bin(ರಚನೆ eeprom_ಕ್ಷೇತ್ರ *ಕ್ಷೇತ್ರ, ಅಕ್ಷರ *ಮೌಲ್ಯ)
{
	ಹಿಂತಿರುಗಿ __eeprom_field_update_bin(ಕ್ಷೇತ್ರ, ಮೌಲ್ಯ, false);
}

/**
 * eeprom_field_update_reserved() - Update reserved field with new data in
 *				    binary form
 *
 * @field:	an initialized field
 * @value:	a space delimited string of byte values (i.e. "1 02 3 0x4")
 */
ಪೂರ್ಣಾಂಕ eeprom_ಕ್ಷೇತ್ರ_update_reserved(ರಚನೆ eeprom_ಕ್ಷೇತ್ರ *ಕ್ಷೇತ್ರ, ಅಕ್ಷರ *ಮೌಲ್ಯ)
{
	ಹಿಂತಿರುಗಿ __eeprom_field_update_bin_delim(ಕ್ಷೇತ್ರ, ಮೌಲ್ಯ, " ");
}

/**
 * eeprom_field_print_bin_rev() - print a field which contains binary data in
 *				  reverse order
 *
 * Treat the field data as simple binary data, and print it in reverse order
 * as two digit hexadecimal values.
 *
 * Data in field:
 *                      0102030405060708090a
 * Sample output:
 *      Field Name      0a090807060504030201
 *
 * @field:	an initialized field to print
 */
ಶೂನ್ಯ eeprom_ಕ್ಷೇತ್ರ_print_bin_rev(ಸ್ಥಿರಾಂಕ ರಚನೆ eeprom_ಕ್ಷೇತ್ರ *ಕ್ಷೇತ್ರ)
{
	__eeprom_field_print_bin(ಕ್ಷೇತ್ರ, "", true);
}

/**
 * eeprom_field_update_bin_rev() - Update field with new data in binary form,
 *				   storing it in reverse
 *
 * This function takes a string of byte values, and stores them
 * in the field in the reverse order. i.e. if the input string was "1234",
 * "3412" will be written to the field.
 *
 * @field:	an initialized field
 * @value:	a string of byte values
 */
ಪೂರ್ಣಾಂಕ eeprom_ಕ್ಷೇತ್ರ_update_bin_rev(ರಚನೆ eeprom_ಕ್ಷೇತ್ರ *ಕ್ಷೇತ್ರ, ಅಕ್ಷರ *ಮೌಲ್ಯ)
{
	ಹಿಂತಿರುಗಿ __eeprom_field_update_bin(ಕ್ಷೇತ್ರ, ಮೌಲ್ಯ, true);
}

/**
 * eeprom_field_print_mac_addr() - print a field which contains a mac address
 *
 * Treat the field data as simple binary data, and print it formatted as a MAC
 * address.
 * Sample output:
 *      Field Name     01:02:03:04:05:06
 *
 * @field:	an initialized field to print
 */
ಶೂನ್ಯ eeprom_ಕ್ಷೇತ್ರ_print_mac(ಸ್ಥಿರಾಂಕ ರಚನೆ eeprom_ಕ್ಷೇತ್ರ *ಕ್ಷೇತ್ರ)
{
	__eeprom_field_print_bin(ಕ್ಷೇತ್ರ, ":", false);
}

/**
 * eeprom_field_update_mac() - Update a mac address field which contains binary
 *			       data
 *
 * @field:	an initialized field
 * @value:	a colon delimited string of byte values (i.e. "1:02:3:ff")
 */
ಪೂರ್ಣಾಂಕ eeprom_ಕ್ಷೇತ್ರ_update_mac(ರಚನೆ eeprom_ಕ್ಷೇತ್ರ *ಕ್ಷೇತ್ರ, ಅಕ್ಷರ *ಮೌಲ್ಯ)
{
	ಹಿಂತಿರುಗಿ __eeprom_field_update_bin_delim(ಕ್ಷೇತ್ರ, ಮೌಲ್ಯ, ":");
}

/**
 * eeprom_field_print_ascii() - print a field which contains ASCII data
 * @field:	an initialized field to print
 */
ಶೂನ್ಯ eeprom_ಕ್ಷೇತ್ರ_print_ascii(ಸ್ಥಿರಾಂಕ ರಚನೆ eeprom_ಕ್ಷೇತ್ರ *ಕ್ಷೇತ್ರ)
{
	ಅಕ್ಷರ format[8];

	sprintf(format, "%%.%ds\n", ಕ್ಷೇತ್ರ->ಗಾತ್ರ_2);
	ಬರೆ(PRINT_ಕ್ಷೇತ್ರ_SEGMENT, ಕ್ಷೇತ್ರ->name);
	ಬರೆ(format, ಕ್ಷೇತ್ರ->buf);
}

/**
 * eeprom_field_update_ascii() - Update field with new data in ASCII form
 * @field:	an initialized field
 * @value:	the new string data
 *
 * Returns 0 on success, -1 of failure (new string too long).
 */
ಪೂರ್ಣಾಂಕ eeprom_ಕ್ಷೇತ್ರ_update_ascii(ರಚನೆ eeprom_ಕ್ಷೇತ್ರ *ಕ್ಷೇತ್ರ, ಅಕ್ಷರ *ಮೌಲ್ಯ)
{
	ಆದರೆ (ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(ಮೌಲ್ಯ) >= ಕ್ಷೇತ್ರ->ಗಾತ್ರ_2) {
		ಬರೆ("%s: new data too long\n", ಕ್ಷೇತ್ರ->name);
		ಹಿಂತಿರುಗಿ -1;
	}

	strncpy((ಅಕ್ಷರ *)ಕ್ಷೇತ್ರ->buf, ಮೌಲ್ಯ, ಕ್ಷೇತ್ರ->ಗಾತ್ರ_2 - 1);
	ಕ್ಷೇತ್ರ->buf[ಕ್ಷೇತ್ರ->ಗಾತ್ರ_2 - 1] = '\0';

	ಹಿಂತಿರುಗಿ 0;
}

/**
 * eeprom_field_print_reserved() - print the "Reserved fields" field
 *
 * Print a notice that the following field_size bytes are reserved.
 *
 * Sample output:
 *      Reserved fields              (64 bytes)
 *
 * @field:	an initialized field to print
 */
ಶೂನ್ಯ eeprom_ಕ್ಷೇತ್ರ_print_reserved(ಸ್ಥಿರಾಂಕ ರಚನೆ eeprom_ಕ್ಷೇತ್ರ *ಕ್ಷೇತ್ರ)
{
	ಬರೆ(PRINT_ಕ್ಷೇತ್ರ_SEGMENT, "Reserved fields\t");
	ಬರೆ("(%d bytes)\n", ಕ್ಷೇತ್ರ->ಗಾತ್ರ_2);
}
