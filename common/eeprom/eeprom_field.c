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

স্থির শূন্য __eeprom_field_print_bin(ধ্রুবক কাঠামো eeprom_field *field,
				     অক্ষর *delimiter, bool reverse)
{
	পূর্ণসংখ্যা i;
	পূর্ণসংখ্যা from = reverse ? field->আকার_2 - 1 : 0;
	পূর্ণসংখ্যা to = reverse ? 0 : field->আকার_2 - 1;

	printf(PRINT_FIELD_SEGMENT, field->name);
	জন্য (i = from; i != to; reverse ? i-- : i++)
		printf("%02x%s", field->buf[i], delimiter);

	printf("%02x\n", field->buf[i]);
}

স্থির পূর্ণসংখ্যা __eeprom_field_update_bin(কাঠামো eeprom_field *field,
				     ধ্রুবক অক্ষর *মান, bool reverse)
{
	পূর্ণসংখ্যা len = strlen(মান);
	পূর্ণসংখ্যা k, j, i = reverse ? len - 1 : 0;
	unsigned অক্ষর byte;
	অক্ষর *endptr;

	/* each two characters in the string fit in one byte */
	যদি (len > field->আকার_2 * 2)
		ফেরত -1;

	memset(field->buf, 0, field->আকার_2);

	/* i - string iterator, j - buf iterator */
	জন্য (j = 0; j < field->আকার_2; j++) {
		byte = 0;
		অক্ষর tmp[3] = { 0, 0, 0 };

		যদি ((reverse && i < 0) || (!reverse && i >= len))
			বিরতি;

		জন্য (k = 0; k < 2; k++) {
			যদি (reverse && i == 0) {
				tmp[k] = মান[i];
				বিরতি;
			}

			tmp[k] = মান[reverse ? i - 1 + k : i + k];
		}

		byte = simple_strtoul(tmp, &endptr, 16);
		যদি (*endptr != '\0')
			ফেরত -1;

		field->buf[j] = byte;
		i = reverse ? i - 2 : i + 2;
	}

	ফেরত 0;
}

স্থির পূর্ণসংখ্যা __eeprom_field_update_bin_delim(কাঠামো eeprom_field *field,
					   অক্ষর *মান, অক্ষর *delimiter)
{
	পূর্ণসংখ্যা গুনতি = 0;
	পূর্ণসংখ্যা i, val;
	ধ্রুবক অক্ষর *tmp = মান;
	অক্ষর *tok;
	অক্ষর *endptr;

	tmp = strstr(tmp, delimiter);
	যখন (tmp != ফাঁকা) {
		গুনতি++;
		tmp++;
		tmp = strstr(tmp, delimiter);
	}

	যদি (গুনতি > field->আকার_2)
		ফেরত -1;

	tok = strtok(মান, delimiter);
	জন্য (i = 0; tok && i < field->আকার_2; i++) {
		val = simple_strtoul(tok, &endptr, 0);
		যদি (*endptr != '\0')
			ফেরত -1;

		/* here we assume that each tok is no more than byte long */
		field->buf[i] = (unsigned অক্ষর)val;
		tok = strtok(ফাঁকা, delimiter);
	}

	ফেরত 0;
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
শূন্য eeprom_field_print_bin(ধ্রুবক কাঠামো eeprom_field *field)
{
	__eeprom_field_print_bin(field, "", false);
}

/**
 * eeprom_field_update_bin() - Update field with new data in binary form
 *
 * @field:	an initialized field
 * @value:	a string of values (i.e. "10b234a")
 */
পূর্ণসংখ্যা eeprom_field_update_bin(কাঠামো eeprom_field *field, অক্ষর *মান)
{
	ফেরত __eeprom_field_update_bin(field, মান, false);
}

/**
 * eeprom_field_update_reserved() - Update reserved field with new data in
 *				    binary form
 *
 * @field:	an initialized field
 * @value:	a space delimited string of byte values (i.e. "1 02 3 0x4")
 */
পূর্ণসংখ্যা eeprom_field_update_reserved(কাঠামো eeprom_field *field, অক্ষর *মান)
{
	ফেরত __eeprom_field_update_bin_delim(field, মান, " ");
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
শূন্য eeprom_field_print_bin_rev(ধ্রুবক কাঠামো eeprom_field *field)
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
পূর্ণসংখ্যা eeprom_field_update_bin_rev(কাঠামো eeprom_field *field, অক্ষর *মান)
{
	ফেরত __eeprom_field_update_bin(field, মান, true);
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
শূন্য eeprom_field_print_mac(ধ্রুবক কাঠামো eeprom_field *field)
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
পূর্ণসংখ্যা eeprom_field_update_mac(কাঠামো eeprom_field *field, অক্ষর *মান)
{
	ফেরত __eeprom_field_update_bin_delim(field, মান, ":");
}

/**
 * eeprom_field_print_ascii() - print a field which contains ASCII data
 * @field:	an initialized field to print
 */
শূন্য eeprom_field_print_ascii(ধ্রুবক কাঠামো eeprom_field *field)
{
	অক্ষর format[8];

	sprintf(format, "%%.%ds\n", field->আকার_2);
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
পূর্ণসংখ্যা eeprom_field_update_ascii(কাঠামো eeprom_field *field, অক্ষর *মান)
{
	যদি (strlen(মান) >= field->আকার_2) {
		printf("%s: new data too long\n", field->name);
		ফেরত -1;
	}

	strncpy((অক্ষর *)field->buf, মান, field->আকার_2 - 1);
	field->buf[field->আকার_2 - 1] = '\0';

	ফেরত 0;
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
শূন্য eeprom_field_print_reserved(ধ্রুবক কাঠামো eeprom_field *field)
{
	printf(PRINT_FIELD_SEGMENT, "Reserved fields\t");
	printf("(%d bytes)\n", field->আকার_2);
}
