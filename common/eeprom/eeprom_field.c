// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2009-2016 CompuLab, Ltd.
 *
 * Authors: Nikita Kiryanov <nikita@compulab.co.il>
 *	    Igor Grinberg <grinberg@compulab.co.il>
 */

#ଅନ୍ତର୍ଭାଵ <stdio.h>
#ଅନ୍ତର୍ଭାଵ <vsprintf.h>
#ଅନ୍ତର୍ଭାଵ <linux/string.h>
#ଅନ୍ତର୍ଭାଵ <eeprom_field.h>

ସ୍ଥିର ଶୂନ୍ୟ __eeprom_field_print_bin(ଅଚଳ ସଂରଚନା eeprom_field *field,
				     ଅକ୍ଷର *delimiter, bool reverse)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା from = reverse ? field->size - 1 : 0;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା to = reverse ? 0 : field->size - 1;

	printf(PRINT_FIELD_SEGMENT, field->name);
	ପାଇଁ (i = from; i != to; reverse ? i-- : i++)
		printf("%02x%s", field->buf[i], delimiter);

	printf("%02x\n", field->buf[i]);
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା __eeprom_field_update_bin(ସଂରଚନା eeprom_field *field,
				     ଅଚଳ ଅକ୍ଷର *ମୂଲ୍ୟ, bool reverse)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା len = strlen(ମୂଲ୍ୟ);
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା k, j, i = reverse ? len - 1 : 0;
	unsigned ଅକ୍ଷର byte;
	ଅକ୍ଷର *endptr;

	/* each two characters in the string fit in one byte */
	ଯଦି (len > field->size * 2)
		ଫେରନ୍ତୁ -1;

	memset(field->buf, 0, field->size);

	/* i - string iterator, j - buf iterator */
	ପାଇଁ (j = 0; j < field->size; j++) {
		byte = 0;
		ଅକ୍ଷର tmp[3] = { 0, 0, 0 };

		ଯଦି ((reverse && i < 0) || (!reverse && i >= len))
			ଭାଙ୍ଗ;

		ପାଇଁ (k = 0; k < 2; k++) {
			ଯଦି (reverse && i == 0) {
				tmp[k] = ମୂଲ୍ୟ[i];
				ଭାଙ୍ଗ;
			}

			tmp[k] = ମୂଲ୍ୟ[reverse ? i - 1 + k : i + k];
		}

		byte = simple_strtoul(tmp, &endptr, 16);
		ଯଦି (*endptr != '\0')
			ଫେରନ୍ତୁ -1;

		field->buf[j] = byte;
		i = reverse ? i - 2 : i + 2;
	}

	ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା __eeprom_field_update_bin_delim(ସଂରଚନା eeprom_field *field,
					   ଅକ୍ଷର *ମୂଲ୍ୟ, ଅକ୍ଷର *delimiter)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଗଣନା_2 = 0;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, val;
	ଅଚଳ ଅକ୍ଷର *tmp = ମୂଲ୍ୟ;
	ଅକ୍ଷର *tok;
	ଅକ୍ଷର *endptr;

	tmp = strstr(tmp, delimiter);
	ଯେତେବେଳେ (tmp != ଖାଲି) {
		ଗଣନା_2++;
		tmp++;
		tmp = strstr(tmp, delimiter);
	}

	ଯଦି (ଗଣନା_2 > field->size)
		ଫେରନ୍ତୁ -1;

	tok = strtok(ମୂଲ୍ୟ, delimiter);
	ପାଇଁ (i = 0; tok && i < field->size; i++) {
		val = simple_strtoul(tok, &endptr, 0);
		ଯଦି (*endptr != '\0')
			ଫେରନ୍ତୁ -1;

		/* here we assume that each tok is no more than byte long */
		field->buf[i] = (unsigned ଅକ୍ଷର)val;
		tok = strtok(ଖାଲି, delimiter);
	}

	ଫେରନ୍ତୁ 0;
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
ଶୂନ୍ୟ eeprom_field_print_bin(ଅଚଳ ସଂରଚନା eeprom_field *field)
{
	__eeprom_field_print_bin(field, "", false);
}

/**
 * eeprom_field_update_bin() - Update field with new data in binary form
 *
 * @field:	an initialized field
 * @value:	a string of values (i.e. "10b234a")
 */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା eeprom_field_update_bin(ସଂରଚନା eeprom_field *field, ଅକ୍ଷର *ମୂଲ୍ୟ)
{
	ଫେରନ୍ତୁ __eeprom_field_update_bin(field, ମୂଲ୍ୟ, false);
}

/**
 * eeprom_field_update_reserved() - Update reserved field with new data in
 *				    binary form
 *
 * @field:	an initialized field
 * @value:	a space delimited string of byte values (i.e. "1 02 3 0x4")
 */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା eeprom_field_update_reserved(ସଂରଚନା eeprom_field *field, ଅକ୍ଷର *ମୂଲ୍ୟ)
{
	ଫେରନ୍ତୁ __eeprom_field_update_bin_delim(field, ମୂଲ୍ୟ, " ");
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
ଶୂନ୍ୟ eeprom_field_print_bin_rev(ଅଚଳ ସଂରଚନା eeprom_field *field)
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
ପୂର୍ଣ୍ଣସଂଖ୍ୟା eeprom_field_update_bin_rev(ସଂରଚନା eeprom_field *field, ଅକ୍ଷର *ମୂଲ୍ୟ)
{
	ଫେରନ୍ତୁ __eeprom_field_update_bin(field, ମୂଲ୍ୟ, true);
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
ଶୂନ୍ୟ eeprom_field_print_mac(ଅଚଳ ସଂରଚନା eeprom_field *field)
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
ପୂର୍ଣ୍ଣସଂଖ୍ୟା eeprom_field_update_mac(ସଂରଚନା eeprom_field *field, ଅକ୍ଷର *ମୂଲ୍ୟ)
{
	ଫେରନ୍ତୁ __eeprom_field_update_bin_delim(field, ମୂଲ୍ୟ, ":");
}

/**
 * eeprom_field_print_ascii() - print a field which contains ASCII data
 * @field:	an initialized field to print
 */
ଶୂନ୍ୟ eeprom_field_print_ascii(ଅଚଳ ସଂରଚନା eeprom_field *field)
{
	ଅକ୍ଷର format[8];

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
ପୂର୍ଣ୍ଣସଂଖ୍ୟା eeprom_field_update_ascii(ସଂରଚନା eeprom_field *field, ଅକ୍ଷର *ମୂଲ୍ୟ)
{
	ଯଦି (strlen(ମୂଲ୍ୟ) >= field->size) {
		printf("%s: new data too long\n", field->name);
		ଫେରନ୍ତୁ -1;
	}

	strncpy((ଅକ୍ଷର *)field->buf, ମୂଲ୍ୟ, field->size - 1);
	field->buf[field->size - 1] = '\0';

	ଫେରନ୍ତୁ 0;
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
ଶୂନ୍ୟ eeprom_field_print_reserved(ଅଚଳ ସଂରଚନା eeprom_field *field)
{
	printf(PRINT_FIELD_SEGMENT, "Reserved fields\t");
	printf("(%d bytes)\n", field->size);
}
