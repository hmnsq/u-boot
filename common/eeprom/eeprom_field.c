// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2009-2016 CompuLab, Ltd.
 *
 * Authors: Nikita Kiryanov <nikita@compulab.co.il>
 *	    Igor Grinberg <grinberg@compulab.co.il>
 */

#અન્તર્ભાવ <stdio.h>
#અન્તર્ભાવ <vsprintf.h>
#અન્તર્ભાવ <linux/string.h>
#અન્તર્ભાવ <eeprom_field.h>

સ્થિર શૂન્ય __eeprom_field_print_bin(અચલ સંરચના eeprom_field *field,
				     અક્ષર *delimiter, bool reverse)
{
	પૂર્ણાંક i;
	પૂર્ણાંક from = reverse ? field->size - 1 : 0;
	પૂર્ણાંક to = reverse ? 0 : field->size - 1;

	printf(PRINT_FIELD_SEGMENT, field->name);
	માટે (i = from; i != to; reverse ? i-- : i++)
		printf("%02x%s", field->buf[i], delimiter);

	printf("%02x\n", field->buf[i]);
}

સ્થિર પૂર્ણાંક __eeprom_field_update_bin(સંરચના eeprom_field *field,
				     અચલ અક્ષર *કિંમત, bool reverse)
{
	પૂર્ણાંક len = strlen(કિંમત);
	પૂર્ણાંક k, j, i = reverse ? len - 1 : 0;
	unsigned અક્ષર byte;
	અક્ષર *endptr;

	/* each two characters in the string fit in one byte */
	જો (len > field->size * 2)
		પાછા -1;

	memset(field->buf, 0, field->size);

	/* i - string iterator, j - buf iterator */
	માટે (j = 0; j < field->size; j++) {
		byte = 0;
		અક્ષર tmp[3] = { 0, 0, 0 };

		જો ((reverse && i < 0) || (!reverse && i >= len))
			તોડો;

		માટે (k = 0; k < 2; k++) {
			જો (reverse && i == 0) {
				tmp[k] = કિંમત[i];
				તોડો;
			}

			tmp[k] = કિંમત[reverse ? i - 1 + k : i + k];
		}

		byte = simple_strtoul(tmp, &endptr, 16);
		જો (*endptr != '\0')
			પાછા -1;

		field->buf[j] = byte;
		i = reverse ? i - 2 : i + 2;
	}

	પાછા 0;
}

સ્થિર પૂર્ણાંક __eeprom_field_update_bin_delim(સંરચના eeprom_field *field,
					   અક્ષર *કિંમત, અક્ષર *delimiter)
{
	પૂર્ણાંક ગણતરી = 0;
	પૂર્ણાંક i, val;
	અચલ અક્ષર *tmp = કિંમત;
	અક્ષર *tok;
	અક્ષર *endptr;

	tmp = strstr(tmp, delimiter);
	જ્યારે (tmp != ખાલી) {
		ગણતરી++;
		tmp++;
		tmp = strstr(tmp, delimiter);
	}

	જો (ગણતરી > field->size)
		પાછા -1;

	tok = strtok(કિંમત, delimiter);
	માટે (i = 0; tok && i < field->size; i++) {
		val = simple_strtoul(tok, &endptr, 0);
		જો (*endptr != '\0')
			પાછા -1;

		/* here we assume that each tok is no more than byte long */
		field->buf[i] = (unsigned અક્ષર)val;
		tok = strtok(ખાલી, delimiter);
	}

	પાછા 0;
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
શૂન્ય eeprom_field_print_bin(અચલ સંરચના eeprom_field *field)
{
	__eeprom_field_print_bin(field, "", false);
}

/**
 * eeprom_field_update_bin() - Update field with new data in binary form
 *
 * @field:	an initialized field
 * @value:	a string of values (i.e. "10b234a")
 */
પૂર્ણાંક eeprom_field_update_bin(સંરચના eeprom_field *field, અક્ષર *કિંમત)
{
	પાછા __eeprom_field_update_bin(field, કિંમત, false);
}

/**
 * eeprom_field_update_reserved() - Update reserved field with new data in
 *				    binary form
 *
 * @field:	an initialized field
 * @value:	a space delimited string of byte values (i.e. "1 02 3 0x4")
 */
પૂર્ણાંક eeprom_field_update_reserved(સંરચના eeprom_field *field, અક્ષર *કિંમત)
{
	પાછા __eeprom_field_update_bin_delim(field, કિંમત, " ");
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
શૂન્ય eeprom_field_print_bin_rev(અચલ સંરચના eeprom_field *field)
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
પૂર્ણાંક eeprom_field_update_bin_rev(સંરચના eeprom_field *field, અક્ષર *કિંમત)
{
	પાછા __eeprom_field_update_bin(field, કિંમત, true);
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
શૂન્ય eeprom_field_print_mac(અચલ સંરચના eeprom_field *field)
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
પૂર્ણાંક eeprom_field_update_mac(સંરચના eeprom_field *field, અક્ષર *કિંમત)
{
	પાછા __eeprom_field_update_bin_delim(field, કિંમત, ":");
}

/**
 * eeprom_field_print_ascii() - print a field which contains ASCII data
 * @field:	an initialized field to print
 */
શૂન્ય eeprom_field_print_ascii(અચલ સંરચના eeprom_field *field)
{
	અક્ષર format[8];

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
પૂર્ણાંક eeprom_field_update_ascii(સંરચના eeprom_field *field, અક્ષર *કિંમત)
{
	જો (strlen(કિંમત) >= field->size) {
		printf("%s: new data too long\n", field->name);
		પાછા -1;
	}

	strncpy((અક્ષર *)field->buf, કિંમત, field->size - 1);
	field->buf[field->size - 1] = '\0';

	પાછા 0;
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
શૂન્ય eeprom_field_print_reserved(અચલ સંરચના eeprom_field *field)
{
	printf(PRINT_FIELD_SEGMENT, "Reserved fields\t");
	printf("(%d bytes)\n", field->size);
}
