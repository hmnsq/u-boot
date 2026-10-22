// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2009-2016 CompuLab, Ltd.
 *
 * Authors: Nikita Kiryanov <nikita@compulab.co.il>
 *	    Igor Grinberg <grinberg@compulab.co.il>
 */

#அந்தர்பாவ <stdio.h>
#அந்தர்பாவ <vsprintf.h>
#அந்தர்பாவ <linux/string.h>
#அந்தர்பாவ <eeprom_field.h>

நிலையான வெறுமை __eeprom_field_print_bin(மாறிலி கட்டமைப்பு eeprom_field *field,
				     எழுத்து *delimiter, bool reverse)
{
	முழு i;
	முழு from = reverse ? field->size - 1 : 0;
	முழு to = reverse ? 0 : field->size - 1;

	printf(PRINT_FIELD_SEGMENT, field->name);
	க்கு (i = from; i != to; reverse ? i-- : i++)
		printf("%02x%s", field->buf[i], delimiter);

	printf("%02x\n", field->buf[i]);
}

நிலையான முழு __eeprom_field_update_bin(கட்டமைப்பு eeprom_field *field,
				     மாறிலி எழுத்து *மதிப்பு, bool reverse)
{
	முழு len = strlen(மதிப்பு);
	முழு k, j, i = reverse ? len - 1 : 0;
	unsigned எழுத்து byte;
	எழுத்து *endptr;

	/* each two characters in the string fit in one byte */
	எனில் (len > field->size * 2)
		திரும்பு -1;

	memset(field->buf, 0, field->size);

	/* i - string iterator, j - buf iterator */
	க்கு (j = 0; j < field->size; j++) {
		byte = 0;
		எழுத்து tmp[3] = { 0, 0, 0 };

		எனில் ((reverse && i < 0) || (!reverse && i >= len))
			நிறுத்து;

		க்கு (k = 0; k < 2; k++) {
			எனில் (reverse && i == 0) {
				tmp[k] = மதிப்பு[i];
				நிறுத்து;
			}

			tmp[k] = மதிப்பு[reverse ? i - 1 + k : i + k];
		}

		byte = simple_strtoul(tmp, &endptr, 16);
		எனில் (*endptr != '\0')
			திரும்பு -1;

		field->buf[j] = byte;
		i = reverse ? i - 2 : i + 2;
	}

	திரும்பு 0;
}

நிலையான முழு __eeprom_field_update_bin_delim(கட்டமைப்பு eeprom_field *field,
					   எழுத்து *மதிப்பு, எழுத்து *delimiter)
{
	முழு மொத்தம் = 0;
	முழு i, val;
	மாறிலி எழுத்து *tmp = மதிப்பு;
	எழுத்து *tok;
	எழுத்து *endptr;

	tmp = strstr(tmp, delimiter);
	போது (tmp != வெற்று) {
		மொத்தம்++;
		tmp++;
		tmp = strstr(tmp, delimiter);
	}

	எனில் (மொத்தம் > field->size)
		திரும்பு -1;

	tok = strtok(மதிப்பு, delimiter);
	க்கு (i = 0; tok && i < field->size; i++) {
		val = simple_strtoul(tok, &endptr, 0);
		எனில் (*endptr != '\0')
			திரும்பு -1;

		/* here we assume that each tok is no more than byte long */
		field->buf[i] = (unsigned எழுத்து)val;
		tok = strtok(வெற்று, delimiter);
	}

	திரும்பு 0;
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
வெறுமை eeprom_field_print_bin(மாறிலி கட்டமைப்பு eeprom_field *field)
{
	__eeprom_field_print_bin(field, "", false);
}

/**
 * eeprom_field_update_bin() - Update field with new data in binary form
 *
 * @field:	an initialized field
 * @value:	a string of values (i.e. "10b234a")
 */
முழு eeprom_field_update_bin(கட்டமைப்பு eeprom_field *field, எழுத்து *மதிப்பு)
{
	திரும்பு __eeprom_field_update_bin(field, மதிப்பு, false);
}

/**
 * eeprom_field_update_reserved() - Update reserved field with new data in
 *				    binary form
 *
 * @field:	an initialized field
 * @value:	a space delimited string of byte values (i.e. "1 02 3 0x4")
 */
முழு eeprom_field_update_reserved(கட்டமைப்பு eeprom_field *field, எழுத்து *மதிப்பு)
{
	திரும்பு __eeprom_field_update_bin_delim(field, மதிப்பு, " ");
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
வெறுமை eeprom_field_print_bin_rev(மாறிலி கட்டமைப்பு eeprom_field *field)
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
முழு eeprom_field_update_bin_rev(கட்டமைப்பு eeprom_field *field, எழுத்து *மதிப்பு)
{
	திரும்பு __eeprom_field_update_bin(field, மதிப்பு, true);
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
வெறுமை eeprom_field_print_mac(மாறிலி கட்டமைப்பு eeprom_field *field)
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
முழு eeprom_field_update_mac(கட்டமைப்பு eeprom_field *field, எழுத்து *மதிப்பு)
{
	திரும்பு __eeprom_field_update_bin_delim(field, மதிப்பு, ":");
}

/**
 * eeprom_field_print_ascii() - print a field which contains ASCII data
 * @field:	an initialized field to print
 */
வெறுமை eeprom_field_print_ascii(மாறிலி கட்டமைப்பு eeprom_field *field)
{
	எழுத்து format[8];

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
முழு eeprom_field_update_ascii(கட்டமைப்பு eeprom_field *field, எழுத்து *மதிப்பு)
{
	எனில் (strlen(மதிப்பு) >= field->size) {
		printf("%s: new data too long\n", field->name);
		திரும்பு -1;
	}

	strncpy((எழுத்து *)field->buf, மதிப்பு, field->size - 1);
	field->buf[field->size - 1] = '\0';

	திரும்பு 0;
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
வெறுமை eeprom_field_print_reserved(மாறிலி கட்டமைப்பு eeprom_field *field)
{
	printf(PRINT_FIELD_SEGMENT, "Reserved fields\t");
	printf("(%d bytes)\n", field->size);
}
