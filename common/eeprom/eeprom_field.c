// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2009-2016 CompuLab, Ltd.
 *
 * Authors: Nikita Kiryanov <nikita@compulab.co.il>
 *	    Igor Grinberg <grinberg@compulab.co.il>
 */

#شامل <stdio.h>
#شامل <vsprintf.h>
#شامل <linux/string.h>
#شامل <eeprom_field.h>

ایستا تهی __eeprom_field_print_bin(ثابت ساختار eeprom_field *field,
				     نویسه *delimiter, bool reverse)
{
	صحیح i;
	صحیح from = reverse ? field->size - 1 : 0;
	صحیح to = reverse ? 0 : field->size - 1;

	printf(PRINT_FIELD_SEGMENT, field->name);
	برای (i = from; i != to; reverse ? i-- : i++)
		printf("%02x%s", field->buf[i], delimiter);

	printf("%02x\n", field->buf[i]);
}

ایستا صحیح __eeprom_field_update_bin(ساختار eeprom_field *field,
				     ثابت نویسه *مقدار, bool reverse)
{
	صحیح len = strlen(مقدار);
	صحیح k, j, i = reverse ? len - 1 : 0;
	unsigned نویسه byte;
	نویسه *endptr;

	/* each two characters in the string fit in one byte */
	اگر (len > field->size * 2)
		بازگشت -1;

	memset(field->buf, 0, field->size);

	/* i - string iterator, j - buf iterator */
	برای (j = 0; j < field->size; j++) {
		byte = 0;
		نویسه tmp[3] = { 0, 0, 0 };

		اگر ((reverse && i < 0) || (!reverse && i >= len))
			بشکن;

		برای (k = 0; k < 2; k++) {
			اگر (reverse && i == 0) {
				tmp[k] = مقدار[i];
				بشکن;
			}

			tmp[k] = مقدار[reverse ? i - 1 + k : i + k];
		}

		byte = simple_strtoul(tmp, &endptr, 16);
		اگر (*endptr != '\0')
			بازگشت -1;

		field->buf[j] = byte;
		i = reverse ? i - 2 : i + 2;
	}

	بازگشت 0;
}

ایستا صحیح __eeprom_field_update_bin_delim(ساختار eeprom_field *field,
					   نویسه *مقدار, نویسه *delimiter)
{
	صحیح شمار = 0;
	صحیح i, val;
	ثابت نویسه *tmp = مقدار;
	نویسه *tok;
	نویسه *endptr;

	tmp = strstr(tmp, delimiter);
	وقتی (tmp != پوچ) {
		شمار++;
		tmp++;
		tmp = strstr(tmp, delimiter);
	}

	اگر (شمار > field->size)
		بازگشت -1;

	tok = strtok(مقدار, delimiter);
	برای (i = 0; tok && i < field->size; i++) {
		val = simple_strtoul(tok, &endptr, 0);
		اگر (*endptr != '\0')
			بازگشت -1;

		/* here we assume that each tok is no more than byte long */
		field->buf[i] = (unsigned نویسه)val;
		tok = strtok(پوچ, delimiter);
	}

	بازگشت 0;
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
تهی eeprom_field_print_bin(ثابت ساختار eeprom_field *field)
{
	__eeprom_field_print_bin(field, "", false);
}

/**
 * eeprom_field_update_bin() - Update field with new data in binary form
 *
 * @field:	an initialized field
 * @value:	a string of values (i.e. "10b234a")
 */
صحیح eeprom_field_update_bin(ساختار eeprom_field *field, نویسه *مقدار)
{
	بازگشت __eeprom_field_update_bin(field, مقدار, false);
}

/**
 * eeprom_field_update_reserved() - Update reserved field with new data in
 *				    binary form
 *
 * @field:	an initialized field
 * @value:	a space delimited string of byte values (i.e. "1 02 3 0x4")
 */
صحیح eeprom_field_update_reserved(ساختار eeprom_field *field, نویسه *مقدار)
{
	بازگشت __eeprom_field_update_bin_delim(field, مقدار, " ");
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
تهی eeprom_field_print_bin_rev(ثابت ساختار eeprom_field *field)
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
صحیح eeprom_field_update_bin_rev(ساختار eeprom_field *field, نویسه *مقدار)
{
	بازگشت __eeprom_field_update_bin(field, مقدار, true);
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
تهی eeprom_field_print_mac(ثابت ساختار eeprom_field *field)
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
صحیح eeprom_field_update_mac(ساختار eeprom_field *field, نویسه *مقدار)
{
	بازگشت __eeprom_field_update_bin_delim(field, مقدار, ":");
}

/**
 * eeprom_field_print_ascii() - print a field which contains ASCII data
 * @field:	an initialized field to print
 */
تهی eeprom_field_print_ascii(ثابت ساختار eeprom_field *field)
{
	نویسه format[8];

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
صحیح eeprom_field_update_ascii(ساختار eeprom_field *field, نویسه *مقدار)
{
	اگر (strlen(مقدار) >= field->size) {
		printf("%s: new data too long\n", field->name);
		بازگشت -1;
	}

	strncpy((نویسه *)field->buf, مقدار, field->size - 1);
	field->buf[field->size - 1] = '\0';

	بازگشت 0;
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
تهی eeprom_field_print_reserved(ثابت ساختار eeprom_field *field)
{
	printf(PRINT_FIELD_SEGMENT, "Reserved fields\t");
	printf("(%d bytes)\n", field->size);
}
