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

ساکن خالی __eeprom_field_print_bin(ثابت ساخت eeprom_میدان *میدان,
				     حرف *delimiter, bool الٹ)
{
	صحیح i;
	صحیح from = الٹ ? میدان->سائز - 1 : 0;
	صحیح مقصد = الٹ ? 0 : میدان->سائز - 1;

	لکھو(PRINT_میدان_SEGMENT, میدان->name);
	برائے (i = from; i != مقصد; الٹ ? i-- : i++)
		لکھو("%02x%s", میدان->buf[i], delimiter);

	لکھو("%02x\n", میدان->buf[i]);
}

ساکن صحیح __eeprom_field_update_bin(ساخت eeprom_میدان *میدان,
				     ثابت حرف *قدر, bool الٹ)
{
	صحیح len = لڑی_لمبائی(قدر);
	صحیح k, j, i = الٹ ? len - 1 : 0;
	غیرعلامتی حرف byte;
	حرف *endptr;

	/* each two characters in the string fit in one byte */
	اگر (len > میدان->سائز * 2)
		واپس -1;

	memset(میدان->buf, 0, میدان->سائز);

	/* i - string iterator, j - buf iterator */
	برائے (j = 0; j < میدان->سائز; j++) {
		byte = 0;
		حرف tmp[3] = { 0, 0, 0 };

		اگر ((الٹ && i < 0) || (!الٹ && i >= len))
			توڑو;

		برائے (k = 0; k < 2; k++) {
			اگر (الٹ && i == 0) {
				tmp[k] = قدر[i];
				توڑو;
			}

			tmp[k] = قدر[الٹ ? i - 1 + k : i + k];
		}

		byte = simple_strtoul(tmp, &endptr, 16);
		اگر (*endptr != '\0')
			واپس -1;

		میدان->buf[j] = byte;
		i = الٹ ? i - 2 : i + 2;
	}

	واپس 0;
}

ساکن صحیح __eeprom_field_update_bin_delim(ساخت eeprom_میدان *میدان,
					   حرف *قدر, حرف *delimiter)
{
	صحیح گنتی = 0;
	صحیح i, val;
	ثابت حرف *tmp = قدر;
	حرف *tok;
	حرف *endptr;

	tmp = لڑی_تلاش(tmp, delimiter);
	جب (tmp != تہی) {
		گنتی++;
		tmp++;
		tmp = لڑی_تلاش(tmp, delimiter);
	}

	اگر (گنتی > میدان->سائز)
		واپس -1;

	tok = strtok(قدر, delimiter);
	برائے (i = 0; tok && i < میدان->سائز; i++) {
		val = simple_strtoul(tok, &endptr, 0);
		اگر (*endptr != '\0')
			واپس -1;

		/* here we assume that each tok is no more than byte long */
		میدان->buf[i] = (غیرعلامتی حرف)val;
		tok = strtok(تہی, delimiter);
	}

	واپس 0;
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
خالی eeprom_میدان_print_bin(ثابت ساخت eeprom_میدان *میدان)
{
	__eeprom_field_print_bin(میدان, "", false);
}

/**
 * eeprom_field_update_bin() - Update field with new data in binary form
 *
 * @field:	an initialized field
 * @value:	a string of values (i.e. "10b234a")
 */
صحیح eeprom_میدان_update_bin(ساخت eeprom_میدان *میدان, حرف *قدر)
{
	واپس __eeprom_field_update_bin(میدان, قدر, false);
}

/**
 * eeprom_field_update_reserved() - Update reserved field with new data in
 *				    binary form
 *
 * @field:	an initialized field
 * @value:	a space delimited string of byte values (i.e. "1 02 3 0x4")
 */
صحیح eeprom_میدان_update_reserved(ساخت eeprom_میدان *میدان, حرف *قدر)
{
	واپس __eeprom_field_update_bin_delim(میدان, قدر, " ");
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
خالی eeprom_میدان_print_bin_rev(ثابت ساخت eeprom_میدان *میدان)
{
	__eeprom_field_print_bin(میدان, "", true);
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
صحیح eeprom_میدان_update_bin_rev(ساخت eeprom_میدان *میدان, حرف *قدر)
{
	واپس __eeprom_field_update_bin(میدان, قدر, true);
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
خالی eeprom_میدان_print_mac(ثابت ساخت eeprom_میدان *میدان)
{
	__eeprom_field_print_bin(میدان, ":", false);
}

/**
 * eeprom_field_update_mac() - Update a mac address field which contains binary
 *			       data
 *
 * @field:	an initialized field
 * @value:	a colon delimited string of byte values (i.e. "1:02:3:ff")
 */
صحیح eeprom_میدان_update_mac(ساخت eeprom_میدان *میدان, حرف *قدر)
{
	واپس __eeprom_field_update_bin_delim(میدان, قدر, ":");
}

/**
 * eeprom_field_print_ascii() - print a field which contains ASCII data
 * @field:	an initialized field to print
 */
خالی eeprom_میدان_print_ascii(ثابت ساخت eeprom_میدان *میدان)
{
	حرف format[8];

	sprintf(format, "%%.%ds\n", میدان->سائز);
	لکھو(PRINT_میدان_SEGMENT, میدان->name);
	لکھو(format, میدان->buf);
}

/**
 * eeprom_field_update_ascii() - Update field with new data in ASCII form
 * @field:	an initialized field
 * @value:	the new string data
 *
 * Returns 0 on success, -1 of failure (new string too long).
 */
صحیح eeprom_میدان_update_ascii(ساخت eeprom_میدان *میدان, حرف *قدر)
{
	اگر (لڑی_لمبائی(قدر) >= میدان->سائز) {
		لکھو("%s: new data too long\n", میدان->name);
		واپس -1;
	}

	strncpy((حرف *)میدان->buf, قدر, میدان->سائز - 1);
	میدان->buf[میدان->سائز - 1] = '\0';

	واپس 0;
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
خالی eeprom_میدان_print_reserved(ثابت ساخت eeprom_میدان *میدان)
{
	لکھو(PRINT_میدان_SEGMENT, "Reserved fields\t");
	لکھو("(%d bytes)\n", میدان->سائز);
}
