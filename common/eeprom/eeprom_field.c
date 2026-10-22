// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2009-2016 CompuLab, Ltd.
 *
 * Authors: Nikita Kiryanov <nikita@compulab.co.il>
 *	    Igor Grinberg <grinberg@compulab.co.il>
 */

#അന്തര്ഭാവ <stdio.h>
#അന്തര്ഭാവ <vsprintf.h>
#അന്തര്ഭാവ <linux/string.h>
#അന്തര്ഭാവ <eeprom_field.h>

സ്ഥിതം ശൂന്യം __eeprom_field_print_bin(സ്ഥിരം ഘടന eeprom_field *field,
				     അക്ഷരം *delimiter, bool reverse)
{
	പൂർണ്ണസംഖ്യ i;
	പൂർണ്ണസംഖ്യ from = reverse ? field->size - 1 : 0;
	പൂർണ്ണസംഖ്യ to = reverse ? 0 : field->size - 1;

	printf(PRINT_FIELD_SEGMENT, field->name);
	വേണ്ടി (i = from; i != to; reverse ? i-- : i++)
		printf("%02x%s", field->buf[i], delimiter);

	printf("%02x\n", field->buf[i]);
}

സ്ഥിതം പൂർണ്ണസംഖ്യ __eeprom_field_update_bin(ഘടന eeprom_field *field,
				     സ്ഥിരം അക്ഷരം *മൂല്യം, bool reverse)
{
	പൂർണ്ണസംഖ്യ len = strlen(മൂല്യം);
	പൂർണ്ണസംഖ്യ k, j, i = reverse ? len - 1 : 0;
	unsigned അക്ഷരം byte;
	അക്ഷരം *endptr;

	/* each two characters in the string fit in one byte */
	എങ്കിൽ (len > field->size * 2)
		തിരികെ -1;

	memset(field->buf, 0, field->size);

	/* i - string iterator, j - buf iterator */
	വേണ്ടി (j = 0; j < field->size; j++) {
		byte = 0;
		അക്ഷരം tmp[3] = { 0, 0, 0 };

		എങ്കിൽ ((reverse && i < 0) || (!reverse && i >= len))
			നിർത്തുക;

		വേണ്ടി (k = 0; k < 2; k++) {
			എങ്കിൽ (reverse && i == 0) {
				tmp[k] = മൂല്യം[i];
				നിർത്തുക;
			}

			tmp[k] = മൂല്യം[reverse ? i - 1 + k : i + k];
		}

		byte = simple_strtoul(tmp, &endptr, 16);
		എങ്കിൽ (*endptr != '\0')
			തിരികെ -1;

		field->buf[j] = byte;
		i = reverse ? i - 2 : i + 2;
	}

	തിരികെ 0;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ __eeprom_field_update_bin_delim(ഘടന eeprom_field *field,
					   അക്ഷരം *മൂല്യം, അക്ഷരം *delimiter)
{
	പൂർണ്ണസംഖ്യ എണ്ണം = 0;
	പൂർണ്ണസംഖ്യ i, val;
	സ്ഥിരം അക്ഷരം *tmp = മൂല്യം;
	അക്ഷരം *tok;
	അക്ഷരം *endptr;

	tmp = strstr(tmp, delimiter);
	ആകുമ്പോൾ (tmp != ശൂന്യ) {
		എണ്ണം++;
		tmp++;
		tmp = strstr(tmp, delimiter);
	}

	എങ്കിൽ (എണ്ണം > field->size)
		തിരികെ -1;

	tok = strtok(മൂല്യം, delimiter);
	വേണ്ടി (i = 0; tok && i < field->size; i++) {
		val = simple_strtoul(tok, &endptr, 0);
		എങ്കിൽ (*endptr != '\0')
			തിരികെ -1;

		/* here we assume that each tok is no more than byte long */
		field->buf[i] = (unsigned അക്ഷരം)val;
		tok = strtok(ശൂന്യ, delimiter);
	}

	തിരികെ 0;
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
ശൂന്യം eeprom_field_print_bin(സ്ഥിരം ഘടന eeprom_field *field)
{
	__eeprom_field_print_bin(field, "", false);
}

/**
 * eeprom_field_update_bin() - Update field with new data in binary form
 *
 * @field:	an initialized field
 * @value:	a string of values (i.e. "10b234a")
 */
പൂർണ്ണസംഖ്യ eeprom_field_update_bin(ഘടന eeprom_field *field, അക്ഷരം *മൂല്യം)
{
	തിരികെ __eeprom_field_update_bin(field, മൂല്യം, false);
}

/**
 * eeprom_field_update_reserved() - Update reserved field with new data in
 *				    binary form
 *
 * @field:	an initialized field
 * @value:	a space delimited string of byte values (i.e. "1 02 3 0x4")
 */
പൂർണ്ണസംഖ്യ eeprom_field_update_reserved(ഘടന eeprom_field *field, അക്ഷരം *മൂല്യം)
{
	തിരികെ __eeprom_field_update_bin_delim(field, മൂല്യം, " ");
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
ശൂന്യം eeprom_field_print_bin_rev(സ്ഥിരം ഘടന eeprom_field *field)
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
പൂർണ്ണസംഖ്യ eeprom_field_update_bin_rev(ഘടന eeprom_field *field, അക്ഷരം *മൂല്യം)
{
	തിരികെ __eeprom_field_update_bin(field, മൂല്യം, true);
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
ശൂന്യം eeprom_field_print_mac(സ്ഥിരം ഘടന eeprom_field *field)
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
പൂർണ്ണസംഖ്യ eeprom_field_update_mac(ഘടന eeprom_field *field, അക്ഷരം *മൂല്യം)
{
	തിരികെ __eeprom_field_update_bin_delim(field, മൂല്യം, ":");
}

/**
 * eeprom_field_print_ascii() - print a field which contains ASCII data
 * @field:	an initialized field to print
 */
ശൂന്യം eeprom_field_print_ascii(സ്ഥിരം ഘടന eeprom_field *field)
{
	അക്ഷരം format[8];

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
പൂർണ്ണസംഖ്യ eeprom_field_update_ascii(ഘടന eeprom_field *field, അക്ഷരം *മൂല്യം)
{
	എങ്കിൽ (strlen(മൂല്യം) >= field->size) {
		printf("%s: new data too long\n", field->name);
		തിരികെ -1;
	}

	strncpy((അക്ഷരം *)field->buf, മൂല്യം, field->size - 1);
	field->buf[field->size - 1] = '\0';

	തിരികെ 0;
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
ശൂന്യം eeprom_field_print_reserved(സ്ഥിരം ഘടന eeprom_field *field)
{
	printf(PRINT_FIELD_SEGMENT, "Reserved fields\t");
	printf("(%d bytes)\n", field->size);
}
