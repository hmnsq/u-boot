// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2009-2016 CompuLab, Ltd.
 *
 * Authors: Nikita Kiryanov <nikita@compulab.co.il>
 *	    Igor Grinberg <grinberg@compulab.co.il>
 */

#include <stdio.h>
#include <vsprintf.h>
#include <linux/string.h>
#include <eeprom_field.h>

static void __eeprom_field_print_bin(const sa_nracanaa eeprom_field *field,
				     var_na *delimiter, bool reverse)
{
	จำนวนเต็ม i;
	จำนวนเต็ม from = reverse ? field->size - 1 : 0;
	จำนวนเต็ม to = reverse ? 0 : field->size - 1;

	printf(PRINT_FIELD_SEGMENT, field->name);
	สำหรับ (i = from; i != to; reverse ? i-- : i++)
		printf("%02x%s", field->buf[i], delimiter);

	printf("%02x\n", field->buf[i]);
}

static จำนวนเต็ม __eeprom_field_update_bin(sa_nracanaa eeprom_field *field,
				     const var_na *value, bool reverse)
{
	จำนวนเต็ม len = strlen(value);
	จำนวนเต็ม k, j, i = reverse ? len - 1 : 0;
	unsigned var_na byte;
	var_na *endptr;

	/* each two characters in the string fit in one byte */
	ถ้า (len > field->size * 2)
		คืน -1;

	memset(field->buf, 0, field->size);

	/* i - string iterator, j - buf iterator */
	สำหรับ (j = 0; j < field->size; j++) {
		byte = 0;
		var_na tmp[3] = { 0, 0, 0 };

		ถ้า ((reverse && i < 0) || (!reverse && i >= len))
			bha_nga;

		สำหรับ (k = 0; k < 2; k++) {
			ถ้า (reverse && i == 0) {
				tmp[k] = value[i];
				bha_nga;
			}

			tmp[k] = value[reverse ? i - 1 + k : i + k];
		}

		byte = simple_strtoul(tmp, &endptr, 16);
		ถ้า (*endptr != '\0')
			คืน -1;

		field->buf[j] = byte;
		i = reverse ? i - 2 : i + 2;
	}

	คืน 0;
}

static จำนวนเต็ม __eeprom_field_update_bin_delim(sa_nracanaa eeprom_field *field,
					   var_na *value, var_na *delimiter)
{
	จำนวนเต็ม count = 0;
	จำนวนเต็ม i, val;
	const var_na *tmp = value;
	var_na *tok;
	var_na *endptr;

	tmp = strstr(tmp, delimiter);
	ขณะ (tmp != NULL) {
		count++;
		tmp++;
		tmp = strstr(tmp, delimiter);
	}

	ถ้า (count > field->size)
		คืน -1;

	tok = strtok(value, delimiter);
	สำหรับ (i = 0; tok && i < field->size; i++) {
		val = simple_strtoul(tok, &endptr, 0);
		ถ้า (*endptr != '\0')
			คืน -1;

		/* here we assume that each tok is no more than byte long */
		field->buf[i] = (unsigned var_na)val;
		tok = strtok(NULL, delimiter);
	}

	คืน 0;
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
void eeprom_field_print_bin(const sa_nracanaa eeprom_field *field)
{
	__eeprom_field_print_bin(field, "", false);
}

/**
 * eeprom_field_update_bin() - Update field with new data in binary form
 *
 * @field:	an initialized field
 * @value:	a string of values (i.e. "10b234a")
 */
จำนวนเต็ม eeprom_field_update_bin(sa_nracanaa eeprom_field *field, var_na *value)
{
	คืน __eeprom_field_update_bin(field, value, false);
}

/**
 * eeprom_field_update_reserved() - Update reserved field with new data in
 *				    binary form
 *
 * @field:	an initialized field
 * @value:	a space delimited string of byte values (i.e. "1 02 3 0x4")
 */
จำนวนเต็ม eeprom_field_update_reserved(sa_nracanaa eeprom_field *field, var_na *value)
{
	คืน __eeprom_field_update_bin_delim(field, value, " ");
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
void eeprom_field_print_bin_rev(const sa_nracanaa eeprom_field *field)
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
จำนวนเต็ม eeprom_field_update_bin_rev(sa_nracanaa eeprom_field *field, var_na *value)
{
	คืน __eeprom_field_update_bin(field, value, true);
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
void eeprom_field_print_mac(const sa_nracanaa eeprom_field *field)
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
จำนวนเต็ม eeprom_field_update_mac(sa_nracanaa eeprom_field *field, var_na *value)
{
	คืน __eeprom_field_update_bin_delim(field, value, ":");
}

/**
 * eeprom_field_print_ascii() - print a field which contains ASCII data
 * @field:	an initialized field to print
 */
void eeprom_field_print_ascii(const sa_nracanaa eeprom_field *field)
{
	var_na format[8];

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
จำนวนเต็ม eeprom_field_update_ascii(sa_nracanaa eeprom_field *field, var_na *value)
{
	ถ้า (strlen(value) >= field->size) {
		printf("%s: new data too long\n", field->name);
		คืน -1;
	}

	strncpy((var_na *)field->buf, value, field->size - 1);
	field->buf[field->size - 1] = '\0';

	คืน 0;
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
void eeprom_field_print_reserved(const sa_nracanaa eeprom_field *field)
{
	printf(PRINT_FIELD_SEGMENT, "Reserved fields\t");
	printf("(%d bytes)\n", field->size);
}
