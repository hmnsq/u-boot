// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2009-2016 CompuLab, Ltd.
 *
 * Authors: Nikita Kiryanov <nikita@compulab.co.il>
 *	    Igor Grinberg <grinberg@compulab.co.il>
 */

#অন্তর্ভাব <stdio.h>
#অন্তর্ভাব <vsprintf.h>
#অন্তর্ভাব <linux/string.h>
#অন্তর্ভাব <eeprom_field.h>

স্থির শূন্য __eeprom_field_print_bin(ধ্রুব সংরচনা eeprom_field *field,
				     বর্ণ *delimiter, bool reverse)
{
	পূর্ণাঙ্ক i;
	পূর্ণাঙ্ক from = reverse ? field->size - 1 : 0;
	পূর্ণাঙ্ক to = reverse ? 0 : field->size - 1;

	printf(PRINT_FIELD_SEGMENT, field->name);
	প্রতি (i = from; i != to; reverse ? i-- : i++)
		printf("%02x%s", field->buf[i], delimiter);

	printf("%02x\n", field->buf[i]);
}

স্থির পূর্ণাঙ্ক __eeprom_field_update_bin(সংরচনা eeprom_field *field,
				     ধ্রুব বর্ণ *মান, bool reverse)
{
	পূর্ণাঙ্ক len = strlen(মান);
	পূর্ণাঙ্ক k, j, i = reverse ? len - 1 : 0;
	unsigned বর্ণ byte;
	বর্ণ *endptr;

	/* each two characters in the string fit in one byte */
	যদি (len > field->size * 2)
		প্রত্যাগম -1;

	memset(field->buf, 0, field->size);

	/* i - string iterator, j - buf iterator */
	প্রতি (j = 0; j < field->size; j++) {
		byte = 0;
		বর্ণ tmp[3] = { 0, 0, 0 };

		যদি ((reverse && i < 0) || (!reverse && i >= len))
			ভঙ্গ;

		প্রতি (k = 0; k < 2; k++) {
			যদি (reverse && i == 0) {
				tmp[k] = মান[i];
				ভঙ্গ;
			}

			tmp[k] = মান[reverse ? i - 1 + k : i + k];
		}

		byte = simple_strtoul(tmp, &endptr, 16);
		যদি (*endptr != '\0')
			প্রত্যাগম -1;

		field->buf[j] = byte;
		i = reverse ? i - 2 : i + 2;
	}

	প্রত্যাগম 0;
}

স্থির পূর্ণাঙ্ক __eeprom_field_update_bin_delim(সংরচনা eeprom_field *field,
					   বর্ণ *মান, বর্ণ *delimiter)
{
	পূর্ণাঙ্ক গুনতি = 0;
	পূর্ণাঙ্ক i, val;
	ধ্রুব বর্ণ *tmp = মান;
	বর্ণ *tok;
	বর্ণ *endptr;

	tmp = strstr(tmp, delimiter);
	যাবত্ (tmp != রিক্ত) {
		গুনতি++;
		tmp++;
		tmp = strstr(tmp, delimiter);
	}

	যদি (গুনতি > field->size)
		প্রত্যাগম -1;

	tok = strtok(মান, delimiter);
	প্রতি (i = 0; tok && i < field->size; i++) {
		val = simple_strtoul(tok, &endptr, 0);
		যদি (*endptr != '\0')
			প্রত্যাগম -1;

		/* here we assume that each tok is no more than byte long */
		field->buf[i] = (unsigned বর্ণ)val;
		tok = strtok(রিক্ত, delimiter);
	}

	প্রত্যাগম 0;
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
শূন্য eeprom_field_print_bin(ধ্রুব সংরচনা eeprom_field *field)
{
	__eeprom_field_print_bin(field, "", false);
}

/**
 * eeprom_field_update_bin() - Update field with new data in binary form
 *
 * @field:	an initialized field
 * @value:	a string of values (i.e. "10b234a")
 */
পূর্ণাঙ্ক eeprom_field_update_bin(সংরচনা eeprom_field *field, বর্ণ *মান)
{
	প্রত্যাগম __eeprom_field_update_bin(field, মান, false);
}

/**
 * eeprom_field_update_reserved() - Update reserved field with new data in
 *				    binary form
 *
 * @field:	an initialized field
 * @value:	a space delimited string of byte values (i.e. "1 02 3 0x4")
 */
পূর্ণাঙ্ক eeprom_field_update_reserved(সংরচনা eeprom_field *field, বর্ণ *মান)
{
	প্রত্যাগম __eeprom_field_update_bin_delim(field, মান, " ");
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
শূন্য eeprom_field_print_bin_rev(ধ্রুব সংরচনা eeprom_field *field)
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
পূর্ণাঙ্ক eeprom_field_update_bin_rev(সংরচনা eeprom_field *field, বর্ণ *মান)
{
	প্রত্যাগম __eeprom_field_update_bin(field, মান, true);
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
শূন্য eeprom_field_print_mac(ধ্রুব সংরচনা eeprom_field *field)
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
পূর্ণাঙ্ক eeprom_field_update_mac(সংরচনা eeprom_field *field, বর্ণ *মান)
{
	প্রত্যাগম __eeprom_field_update_bin_delim(field, মান, ":");
}

/**
 * eeprom_field_print_ascii() - print a field which contains ASCII data
 * @field:	an initialized field to print
 */
শূন্য eeprom_field_print_ascii(ধ্রুব সংরচনা eeprom_field *field)
{
	বর্ণ format[8];

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
পূর্ণাঙ্ক eeprom_field_update_ascii(সংরচনা eeprom_field *field, বর্ণ *মান)
{
	যদি (strlen(মান) >= field->size) {
		printf("%s: new data too long\n", field->name);
		প্রত্যাগম -1;
	}

	strncpy((বর্ণ *)field->buf, মান, field->size - 1);
	field->buf[field->size - 1] = '\0';

	প্রত্যাগম 0;
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
শূন্য eeprom_field_print_reserved(ধ্রুব সংরচনা eeprom_field *field)
{
	printf(PRINT_FIELD_SEGMENT, "Reserved fields\t");
	printf("(%d bytes)\n", field->size);
}
