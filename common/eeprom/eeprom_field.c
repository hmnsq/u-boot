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

ساكن فراغ __eeprom_field_print_bin(ثابت بنية eeprom_ميدان *ميدان,
				     حرف *delimiter, bool ابتو)
{
	صحيح i;
	صحيح from = ابتو ? ميدان->سائيز - 1 : 0;
	صحيح منزل = ابتو ? 0 : ميدان->سائيز - 1;

	لکو(PRINT_ميدان_SEGMENT, ميدان->name);
	لكل (i = from; i != منزل; ابتو ? i-- : i++)
		لکو("%02x%s", ميدان->buf[i], delimiter);

	لکو("%02x\n", ميدان->buf[i]);
}

ساكن صحيح __eeprom_field_update_bin(بنية eeprom_ميدان *ميدان,
				     ثابت حرف *قدر, bool ابتو)
{
	صحيح len = لڙي_ڊيگهه(قدر);
	صحيح k, j, i = ابتو ? len - 1 : 0;
	بنانشان حرف byte;
	حرف *endptr;

	/* each two characters in the string fit in one byte */
	إذا (len > ميدان->سائيز * 2)
		أرجع -1;

	memset(ميدان->buf, 0, ميدان->سائيز);

	/* i - string iterator, j - buf iterator */
	لكل (j = 0; j < ميدان->سائيز; j++) {
		byte = 0;
		حرف tmp[3] = { 0, 0, 0 };

		إذا ((ابتو && i < 0) || (!ابتو && i >= len))
			اكسر;

		لكل (k = 0; k < 2; k++) {
			إذا (ابتو && i == 0) {
				tmp[k] = قدر[i];
				اكسر;
			}

			tmp[k] = قدر[ابتو ? i - 1 + k : i + k];
		}

		byte = simple_strtoul(tmp, &endptr, 16);
		إذا (*endptr != '\0')
			أرجع -1;

		ميدان->buf[j] = byte;
		i = ابتو ? i - 2 : i + 2;
	}

	أرجع 0;
}

ساكن صحيح __eeprom_field_update_bin_delim(بنية eeprom_ميدان *ميدان,
					   حرف *قدر, حرف *delimiter)
{
	صحيح ڳڻپ = 0;
	صحيح i, val;
	ثابت حرف *tmp = قدر;
	حرف *tok;
	حرف *endptr;

	tmp = لڙي_ڳولا(tmp, delimiter);
	طالما (tmp != عدم) {
		ڳڻپ++;
		tmp++;
		tmp = لڙي_ڳولا(tmp, delimiter);
	}

	إذا (ڳڻپ > ميدان->سائيز)
		أرجع -1;

	tok = strtok(قدر, delimiter);
	لكل (i = 0; tok && i < ميدان->سائيز; i++) {
		val = simple_strtoul(tok, &endptr, 0);
		إذا (*endptr != '\0')
			أرجع -1;

		/* here we assume that each tok is no more than byte long */
		ميدان->buf[i] = (بنانشان حرف)val;
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
فراغ eeprom_ميدان_print_bin(ثابت بنية eeprom_ميدان *ميدان)
{
	__eeprom_field_print_bin(ميدان, "", false);
}

/**
 * eeprom_field_update_bin() - Update field with new data in binary form
 *
 * @field:	an initialized field
 * @value:	a string of values (i.e. "10b234a")
 */
صحيح eeprom_ميدان_update_bin(بنية eeprom_ميدان *ميدان, حرف *قدر)
{
	أرجع __eeprom_field_update_bin(ميدان, قدر, false);
}

/**
 * eeprom_field_update_reserved() - Update reserved field with new data in
 *				    binary form
 *
 * @field:	an initialized field
 * @value:	a space delimited string of byte values (i.e. "1 02 3 0x4")
 */
صحيح eeprom_ميدان_update_reserved(بنية eeprom_ميدان *ميدان, حرف *قدر)
{
	أرجع __eeprom_field_update_bin_delim(ميدان, قدر, " ");
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
فراغ eeprom_ميدان_print_bin_rev(ثابت بنية eeprom_ميدان *ميدان)
{
	__eeprom_field_print_bin(ميدان, "", true);
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
صحيح eeprom_ميدان_update_bin_rev(بنية eeprom_ميدان *ميدان, حرف *قدر)
{
	أرجع __eeprom_field_update_bin(ميدان, قدر, true);
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
فراغ eeprom_ميدان_print_mac(ثابت بنية eeprom_ميدان *ميدان)
{
	__eeprom_field_print_bin(ميدان, ":", false);
}

/**
 * eeprom_field_update_mac() - Update a mac address field which contains binary
 *			       data
 *
 * @field:	an initialized field
 * @value:	a colon delimited string of byte values (i.e. "1:02:3:ff")
 */
صحيح eeprom_ميدان_update_mac(بنية eeprom_ميدان *ميدان, حرف *قدر)
{
	أرجع __eeprom_field_update_bin_delim(ميدان, قدر, ":");
}

/**
 * eeprom_field_print_ascii() - print a field which contains ASCII data
 * @field:	an initialized field to print
 */
فراغ eeprom_ميدان_print_ascii(ثابت بنية eeprom_ميدان *ميدان)
{
	حرف format[8];

	sprintf(format, "%%.%ds\n", ميدان->سائيز);
	لکو(PRINT_ميدان_SEGMENT, ميدان->name);
	لکو(format, ميدان->buf);
}

/**
 * eeprom_field_update_ascii() - Update field with new data in ASCII form
 * @field:	an initialized field
 * @value:	the new string data
 *
 * Returns 0 on success, -1 of failure (new string too long).
 */
صحيح eeprom_ميدان_update_ascii(بنية eeprom_ميدان *ميدان, حرف *قدر)
{
	إذا (لڙي_ڊيگهه(قدر) >= ميدان->سائيز) {
		لکو("%s: new data too long\n", ميدان->name);
		أرجع -1;
	}

	strncpy((حرف *)ميدان->buf, قدر, ميدان->سائيز - 1);
	ميدان->buf[ميدان->سائيز - 1] = '\0';

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
فراغ eeprom_ميدان_print_reserved(ثابت بنية eeprom_ميدان *ميدان)
{
	لکو(PRINT_ميدان_SEGMENT, "Reserved fields\t");
	لکو("(%d bytes)\n", ميدان->سائيز);
}
