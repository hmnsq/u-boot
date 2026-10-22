// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2009-2016 CompuLab, Ltd.
 *
 * Authors: Nikita Kiryanov <nikita@compulab.co.il>
 *	    Igor Grinberg <grinberg@compulab.co.il>
 */

#ضمن <stdio.h>
#ضمن <vsprintf.h>
#ضمن <linux/string.h>
#ضمن <eeprom_field.h>

ساكن فراغ __eeprom_field_print_bin(ثابت بنية eeprom_field *field,
				     حرف *delimiter, bool reverse)
{
	صحيح i;
	صحيح from = reverse ? field->size - 1 : 0;
	صحيح to = reverse ? 0 : field->size - 1;

	printf(PRINT_FIELD_SEGMENT, field->name);
	لكل (i = from; i != to; reverse ? i-- : i++)
		printf("%02x%s", field->buf[i], delimiter);

	printf("%02x\n", field->buf[i]);
}

ساكن صحيح __eeprom_field_update_bin(بنية eeprom_field *field,
				     ثابت حرف *نيلاي, bool reverse)
{
	صحيح len = strlen(نيلاي);
	صحيح k, j, i = reverse ? len - 1 : 0;
	unsigned حرف byte;
	حرف *endptr;

	/* each two characters in the string fit in one byte */
	إذا (len > field->size * 2)
		أرجع -1;

	memset(field->buf, 0, field->size);

	/* i - string iterator, j - buf iterator */
	لكل (j = 0; j < field->size; j++) {
		byte = 0;
		حرف tmp[3] = { 0, 0, 0 };

		إذا ((reverse && i < 0) || (!reverse && i >= len))
			اكسر;

		لكل (k = 0; k < 2; k++) {
			إذا (reverse && i == 0) {
				tmp[k] = نيلاي[i];
				اكسر;
			}

			tmp[k] = نيلاي[reverse ? i - 1 + k : i + k];
		}

		byte = simple_strtoul(tmp, &endptr, 16);
		إذا (*endptr != '\0')
			أرجع -1;

		field->buf[j] = byte;
		i = reverse ? i - 2 : i + 2;
	}

	أرجع 0;
}

ساكن صحيح __eeprom_field_update_bin_delim(بنية eeprom_field *field,
					   حرف *نيلاي, حرف *delimiter)
{
	صحيح کيراءن = 0;
	صحيح i, val;
	ثابت حرف *tmp = نيلاي;
	حرف *tok;
	حرف *endptr;

	tmp = strstr(tmp, delimiter);
	طالما (tmp != عدم) {
		کيراءن++;
		tmp++;
		tmp = strstr(tmp, delimiter);
	}

	إذا (کيراءن > field->size)
		أرجع -1;

	tok = strtok(نيلاي, delimiter);
	لكل (i = 0; tok && i < field->size; i++) {
		val = simple_strtoul(tok, &endptr, 0);
		إذا (*endptr != '\0')
			أرجع -1;

		/* here we assume that each tok is no more than byte long */
		field->buf[i] = (unsigned حرف)val;
		tok = strtok(عدم, delimiter);
	}

	أرجع 0;
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
فراغ eeprom_field_print_bin(ثابت بنية eeprom_field *field)
{
	__eeprom_field_print_bin(field, "", false);
}

/**
 * eeprom_field_update_bin() - Update field with new data in binary form
 *
 * @field:	an initialized field
 * @value:	a string of values (i.e. "10b234a")
 */
صحيح eeprom_field_update_bin(بنية eeprom_field *field, حرف *نيلاي)
{
	أرجع __eeprom_field_update_bin(field, نيلاي, false);
}

/**
 * eeprom_field_update_reserved() - Update reserved field with new data in
 *				    binary form
 *
 * @field:	an initialized field
 * @value:	a space delimited string of byte values (i.e. "1 02 3 0x4")
 */
صحيح eeprom_field_update_reserved(بنية eeprom_field *field, حرف *نيلاي)
{
	أرجع __eeprom_field_update_bin_delim(field, نيلاي, " ");
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
فراغ eeprom_field_print_bin_rev(ثابت بنية eeprom_field *field)
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
صحيح eeprom_field_update_bin_rev(بنية eeprom_field *field, حرف *نيلاي)
{
	أرجع __eeprom_field_update_bin(field, نيلاي, true);
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
فراغ eeprom_field_print_mac(ثابت بنية eeprom_field *field)
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
صحيح eeprom_field_update_mac(بنية eeprom_field *field, حرف *نيلاي)
{
	أرجع __eeprom_field_update_bin_delim(field, نيلاي, ":");
}

/**
 * eeprom_field_print_ascii() - print a field which contains ASCII data
 * @field:	an initialized field to print
 */
فراغ eeprom_field_print_ascii(ثابت بنية eeprom_field *field)
{
	حرف format[8];

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
صحيح eeprom_field_update_ascii(بنية eeprom_field *field, حرف *نيلاي)
{
	إذا (strlen(نيلاي) >= field->size) {
		printf("%s: new data too long\n", field->name);
		أرجع -1;
	}

	strncpy((حرف *)field->buf, نيلاي, field->size - 1);
	field->buf[field->size - 1] = '\0';

	أرجع 0;
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
فراغ eeprom_field_print_reserved(ثابت بنية eeprom_field *field)
{
	printf(PRINT_FIELD_SEGMENT, "Reserved fields\t");
	printf("(%d bytes)\n", field->size);
}
