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

ಸ್ಥಿರ ಶೂನ್ಯ __eeprom_field_print_bin(ಧ್ರುವ ಸಂರಚನಾ eeprom_field *field,
				     ವರ್ಣ *delimiter, bool reverse)
{
	ಪೂರ್ಣಾಙ್ಕ i;
	ಪೂರ್ಣಾಙ್ಕ from = reverse ? field->size - 1 : 0;
	ಪೂರ್ಣಾಙ್ಕ to = reverse ? 0 : field->size - 1;

	printf(PRINT_FIELD_SEGMENT, field->name);
	ಪ್ರತಿ (i = from; i != to; reverse ? i-- : i++)
		printf("%02x%s", field->buf[i], delimiter);

	printf("%02x\n", field->buf[i]);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ __eeprom_field_update_bin(ಸಂರಚನಾ eeprom_field *field,
				     ಧ್ರುವ ವರ್ಣ *ಮೌಲ್ಯ, bool reverse)
{
	ಪೂರ್ಣಾಙ್ಕ len = strlen(ಮೌಲ್ಯ);
	ಪೂರ್ಣಾಙ್ಕ k, j, i = reverse ? len - 1 : 0;
	unsigned ವರ್ಣ byte;
	ವರ್ಣ *endptr;

	/* each two characters in the string fit in one byte */
	ಯದಿ (len > field->size * 2)
		ಪ್ರತ್ಯಾಗಮ -1;

	memset(field->buf, 0, field->size);

	/* i - string iterator, j - buf iterator */
	ಪ್ರತಿ (j = 0; j < field->size; j++) {
		byte = 0;
		ವರ್ಣ tmp[3] = { 0, 0, 0 };

		ಯದಿ ((reverse && i < 0) || (!reverse && i >= len))
			ಭಙ್ಗ;

		ಪ್ರತಿ (k = 0; k < 2; k++) {
			ಯದಿ (reverse && i == 0) {
				tmp[k] = ಮೌಲ್ಯ[i];
				ಭಙ್ಗ;
			}

			tmp[k] = ಮೌಲ್ಯ[reverse ? i - 1 + k : i + k];
		}

		byte = simple_strtoul(tmp, &endptr, 16);
		ಯದಿ (*endptr != '\0')
			ಪ್ರತ್ಯಾಗಮ -1;

		field->buf[j] = byte;
		i = reverse ? i - 2 : i + 2;
	}

	ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ __eeprom_field_update_bin_delim(ಸಂರಚನಾ eeprom_field *field,
					   ವರ್ಣ *ಮೌಲ್ಯ, ವರ್ಣ *delimiter)
{
	ಪೂರ್ಣಾಙ್ಕ ಎಣಿಕೆ = 0;
	ಪೂರ್ಣಾಙ್ಕ i, val;
	ಧ್ರುವ ವರ್ಣ *tmp = ಮೌಲ್ಯ;
	ವರ್ಣ *tok;
	ವರ್ಣ *endptr;

	tmp = strstr(tmp, delimiter);
	ಯಾವತ್ (tmp != ರಿಕ್ತ) {
		ಎಣಿಕೆ++;
		tmp++;
		tmp = strstr(tmp, delimiter);
	}

	ಯದಿ (ಎಣಿಕೆ > field->size)
		ಪ್ರತ್ಯಾಗಮ -1;

	tok = strtok(ಮೌಲ್ಯ, delimiter);
	ಪ್ರತಿ (i = 0; tok && i < field->size; i++) {
		val = simple_strtoul(tok, &endptr, 0);
		ಯದಿ (*endptr != '\0')
			ಪ್ರತ್ಯಾಗಮ -1;

		/* here we assume that each tok is no more than byte long */
		field->buf[i] = (unsigned ವರ್ಣ)val;
		tok = strtok(ರಿಕ್ತ, delimiter);
	}

	ಪ್ರತ್ಯಾಗಮ 0;
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
ಶೂನ್ಯ eeprom_field_print_bin(ಧ್ರುವ ಸಂರಚನಾ eeprom_field *field)
{
	__eeprom_field_print_bin(field, "", false);
}

/**
 * eeprom_field_update_bin() - Update field with new data in binary form
 *
 * @field:	an initialized field
 * @value:	a string of values (i.e. "10b234a")
 */
ಪೂರ್ಣಾಙ್ಕ eeprom_field_update_bin(ಸಂರಚನಾ eeprom_field *field, ವರ್ಣ *ಮೌಲ್ಯ)
{
	ಪ್ರತ್ಯಾಗಮ __eeprom_field_update_bin(field, ಮೌಲ್ಯ, false);
}

/**
 * eeprom_field_update_reserved() - Update reserved field with new data in
 *				    binary form
 *
 * @field:	an initialized field
 * @value:	a space delimited string of byte values (i.e. "1 02 3 0x4")
 */
ಪೂರ್ಣಾಙ್ಕ eeprom_field_update_reserved(ಸಂರಚನಾ eeprom_field *field, ವರ್ಣ *ಮೌಲ್ಯ)
{
	ಪ್ರತ್ಯಾಗಮ __eeprom_field_update_bin_delim(field, ಮೌಲ್ಯ, " ");
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
ಶೂನ್ಯ eeprom_field_print_bin_rev(ಧ್ರುವ ಸಂರಚನಾ eeprom_field *field)
{
	__eeprom_field_print_bin(field, "", true);
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
ಪೂರ್ಣಾಙ್ಕ eeprom_field_update_bin_rev(ಸಂರಚನಾ eeprom_field *field, ವರ್ಣ *ಮೌಲ್ಯ)
{
	ಪ್ರತ್ಯಾಗಮ __eeprom_field_update_bin(field, ಮೌಲ್ಯ, true);
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
ಶೂನ್ಯ eeprom_field_print_mac(ಧ್ರುವ ಸಂರಚನಾ eeprom_field *field)
{
	__eeprom_field_print_bin(field, ":", false);
}

/**
 * eeprom_field_update_mac() - Update a mac address field which contains binary
 *			       data
 *
 * @field:	an initialized field
 * @value:	a colon delimited string of byte values (i.e. "1:02:3:ff")
 */
ಪೂರ್ಣಾಙ್ಕ eeprom_field_update_mac(ಸಂರಚನಾ eeprom_field *field, ವರ್ಣ *ಮೌಲ್ಯ)
{
	ಪ್ರತ್ಯಾಗಮ __eeprom_field_update_bin_delim(field, ಮೌಲ್ಯ, ":");
}

/**
 * eeprom_field_print_ascii() - print a field which contains ASCII data
 * @field:	an initialized field to print
 */
ಶೂನ್ಯ eeprom_field_print_ascii(ಧ್ರುವ ಸಂರಚನಾ eeprom_field *field)
{
	ವರ್ಣ format[8];

	sprintf(format, "%%.%ds\n", field->size);
	printf(PRINT_FIELD_SEGMENT, field->name);
	printf(format, field->buf);
}

/**
 * eeprom_field_update_ascii() - Update field with new data in ASCII form
 * @field:	an initialized field
 * @value:	the new string data
 *
 * Returns 0 on success, -1 of failure (new string too long).
 */
ಪೂರ್ಣಾಙ್ಕ eeprom_field_update_ascii(ಸಂರಚನಾ eeprom_field *field, ವರ್ಣ *ಮೌಲ್ಯ)
{
	ಯದಿ (strlen(ಮೌಲ್ಯ) >= field->size) {
		printf("%s: new data too long\n", field->name);
		ಪ್ರತ್ಯಾಗಮ -1;
	}

	strncpy((ವರ್ಣ *)field->buf, ಮೌಲ್ಯ, field->size - 1);
	field->buf[field->size - 1] = '\0';

	ಪ್ರತ್ಯಾಗಮ 0;
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
ಶೂನ್ಯ eeprom_field_print_reserved(ಧ್ರುವ ಸಂರಚನಾ eeprom_field *field)
{
	printf(PRINT_FIELD_SEGMENT, "Reserved fields\t");
	printf("(%d bytes)\n", field->size);
}
