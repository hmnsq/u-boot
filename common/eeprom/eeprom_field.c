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
	puur_naa_nka i;
	puur_naa_nka from = reverse ? field->size - 1 : 0;
	puur_naa_nka to = reverse ? 0 : field->size - 1;

	printf(PRINT_FIELD_SEGMENT, field->name);
	ꦏꦁꦒꦺꦴ (i = from; i != to; reverse ? i-- : i++)
		printf("%02x%s", field->buf[i], delimiter);

	printf("%02x\n", field->buf[i]);
}

static puur_naa_nka __eeprom_field_update_bin(sa_nracanaa eeprom_field *field,
				     const var_na *value, bool reverse)
{
	puur_naa_nka len = strlen(value);
	puur_naa_nka k, j, i = reverse ? len - 1 : 0;
	unsigned var_na byte;
	var_na *endptr;

	/* each two characters in the string fit in one byte */
	ꦪꦼꦤ꧀ (len > field->size * 2)
		ꦮꦭꦶ -1;

	memset(field->buf, 0, field->size);

	/* i - string iterator, j - buf iterator */
	ꦏꦁꦒꦺꦴ (j = 0; j < field->size; j++) {
		byte = 0;
		var_na tmp[3] = { 0, 0, 0 };

		ꦪꦼꦤ꧀ ((reverse && i < 0) || (!reverse && i >= len))
			bha_nga;

		ꦏꦁꦒꦺꦴ (k = 0; k < 2; k++) {
			ꦪꦼꦤ꧀ (reverse && i == 0) {
				tmp[k] = value[i];
				bha_nga;
			}

			tmp[k] = value[reverse ? i - 1 + k : i + k];
		}

		byte = simple_strtoul(tmp, &endptr, 16);
		ꦪꦼꦤ꧀ (*endptr != '\0')
			ꦮꦭꦶ -1;

		field->buf[j] = byte;
		i = reverse ? i - 2 : i + 2;
	}

	ꦮꦭꦶ 0;
}

static puur_naa_nka __eeprom_field_update_bin_delim(sa_nracanaa eeprom_field *field,
					   var_na *value, var_na *delimiter)
{
	puur_naa_nka count = 0;
	puur_naa_nka i, val;
	const var_na *tmp = value;
	var_na *tok;
	var_na *endptr;

	tmp = strstr(tmp, delimiter);
	ꦱꦼꦢꦼꦭ (tmp != NULL) {
		count++;
		tmp++;
		tmp = strstr(tmp, delimiter);
	}

	ꦪꦼꦤ꧀ (count > field->size)
		ꦮꦭꦶ -1;

	tok = strtok(value, delimiter);
	ꦏꦁꦒꦺꦴ (i = 0; tok && i < field->size; i++) {
		val = simple_strtoul(tok, &endptr, 0);
		ꦪꦼꦤ꧀ (*endptr != '\0')
			ꦮꦭꦶ -1;

		/* here we assume that each tok is no more than byte long */
		field->buf[i] = (unsigned var_na)val;
		tok = strtok(NULL, delimiter);
	}

	ꦮꦭꦶ 0;
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
puur_naa_nka eeprom_field_update_bin(sa_nracanaa eeprom_field *field, var_na *value)
{
	ꦮꦭꦶ __eeprom_field_update_bin(field, value, false);
}

/**
 * eeprom_field_update_reserved() - Update reserved field with new data in
 *				    binary form
 *
 * @field:	an initialized field
 * @value:	a space delimited string of byte values (i.e. "1 02 3 0x4")
 */
puur_naa_nka eeprom_field_update_reserved(sa_nracanaa eeprom_field *field, var_na *value)
{
	ꦮꦭꦶ __eeprom_field_update_bin_delim(field, value, " ");
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
puur_naa_nka eeprom_field_update_bin_rev(sa_nracanaa eeprom_field *field, var_na *value)
{
	ꦮꦭꦶ __eeprom_field_update_bin(field, value, true);
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
puur_naa_nka eeprom_field_update_mac(sa_nracanaa eeprom_field *field, var_na *value)
{
	ꦮꦭꦶ __eeprom_field_update_bin_delim(field, value, ":");
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
puur_naa_nka eeprom_field_update_ascii(sa_nracanaa eeprom_field *field, var_na *value)
{
	ꦪꦼꦤ꧀ (strlen(value) >= field->size) {
		printf("%s: new data too long\n", field->name);
		ꦮꦭꦶ -1;
	}

	strncpy((var_na *)field->buf, value, field->size - 1);
	field->buf[field->size - 1] = '\0';

	ꦮꦭꦶ 0;
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
