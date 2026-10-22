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

ساكن فراغ __eeprom_field_print_bin(ثابت بنية eeprom_فیلد *فیلد,
				     حرف *delimiter, bool معکوس)
{
	صحيح i;
	صحيح from = معکوس ? فیلد->اندازه - 1 : 0;
	صحيح مقصد = معکوس ? 0 : فیلد->اندازه - 1;

	چاپ_کن(PRINT_فیلد_SEGMENT, فیلد->name);
	لكل (i = from; i != مقصد; معکوس ? i-- : i++)
		چاپ_کن("%02x%s", فیلد->buf[i], delimiter);

	چاپ_کن("%02x\n", فیلد->buf[i]);
}

ساكن صحيح __eeprom_field_update_bin(بنية eeprom_فیلد *فیلد,
				     ثابت حرف *مقدار, bool معکوس)
{
	صحيح len = طول_رشته(مقدار);
	صحيح k, j, i = معکوس ? len - 1 : 0;
	بیعلامت حرف byte;
	حرف *endptr;

	/* each two characters in the string fit in one byte */
	إذا (len > فیلد->اندازه * 2)
		أرجع -1;

	memset(فیلد->buf, 0, فیلد->اندازه);

	/* i - string iterator, j - buf iterator */
	لكل (j = 0; j < فیلد->اندازه; j++) {
		byte = 0;
		حرف tmp[3] = { 0, 0, 0 };

		إذا ((معکوس && i < 0) || (!معکوس && i >= len))
			اكسر;

		لكل (k = 0; k < 2; k++) {
			إذا (معکوس && i == 0) {
				tmp[k] = مقدار[i];
				اكسر;
			}

			tmp[k] = مقدار[معکوس ? i - 1 + k : i + k];
		}

		byte = simple_strtoul(tmp, &endptr, 16);
		إذا (*endptr != '\0')
			أرجع -1;

		فیلد->buf[j] = byte;
		i = معکوس ? i - 2 : i + 2;
	}

	أرجع 0;
}

ساكن صحيح __eeprom_field_update_bin_delim(بنية eeprom_فیلد *فیلد,
					   حرف *مقدار, حرف *delimiter)
{
	صحيح شمار = 0;
	صحيح i, val;
	ثابت حرف *tmp = مقدار;
	حرف *tok;
	حرف *endptr;

	tmp = جستجوی_رشته(tmp, delimiter);
	طالما (tmp != عدم) {
		شمار++;
		tmp++;
		tmp = جستجوی_رشته(tmp, delimiter);
	}

	إذا (شمار > فیلد->اندازه)
		أرجع -1;

	tok = strtok(مقدار, delimiter);
	لكل (i = 0; tok && i < فیلد->اندازه; i++) {
		val = simple_strtoul(tok, &endptr, 0);
		إذا (*endptr != '\0')
			أرجع -1;

		/* here we assume that each tok is no more than byte long */
		فیلد->buf[i] = (بیعلامت حرف)val;
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
فراغ eeprom_فیلد_print_bin(ثابت بنية eeprom_فیلد *فیلد)
{
	__eeprom_field_print_bin(فیلد, "", false);
}

/**
 * eeprom_field_update_bin() - Update field with new data in binary form
 *
 * @field:	an initialized field
 * @value:	a string of values (i.e. "10b234a")
 */
صحيح eeprom_فیلد_update_bin(بنية eeprom_فیلد *فیلد, حرف *مقدار)
{
	أرجع __eeprom_field_update_bin(فیلد, مقدار, false);
}

/**
 * eeprom_field_update_reserved() - Update reserved field with new data in
 *				    binary form
 *
 * @field:	an initialized field
 * @value:	a space delimited string of byte values (i.e. "1 02 3 0x4")
 */
صحيح eeprom_فیلد_update_reserved(بنية eeprom_فیلد *فیلد, حرف *مقدار)
{
	أرجع __eeprom_field_update_bin_delim(فیلد, مقدار, " ");
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
فراغ eeprom_فیلد_print_bin_rev(ثابت بنية eeprom_فیلد *فیلد)
{
	__eeprom_field_print_bin(فیلد, "", true);
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
صحيح eeprom_فیلد_update_bin_rev(بنية eeprom_فیلد *فیلد, حرف *مقدار)
{
	أرجع __eeprom_field_update_bin(فیلد, مقدار, true);
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
فراغ eeprom_فیلد_print_mac(ثابت بنية eeprom_فیلد *فیلد)
{
	__eeprom_field_print_bin(فیلد, ":", false);
}

/**
 * eeprom_field_update_mac() - Update a mac address field which contains binary
 *			       data
 *
 * @field:	an initialized field
 * @value:	a colon delimited string of byte values (i.e. "1:02:3:ff")
 */
صحيح eeprom_فیلد_update_mac(بنية eeprom_فیلد *فیلد, حرف *مقدار)
{
	أرجع __eeprom_field_update_bin_delim(فیلد, مقدار, ":");
}

/**
 * eeprom_field_print_ascii() - print a field which contains ASCII data
 * @field:	an initialized field to print
 */
فراغ eeprom_فیلد_print_ascii(ثابت بنية eeprom_فیلد *فیلد)
{
	حرف format[8];

	sprintf(format, "%%.%ds\n", فیلد->اندازه);
	چاپ_کن(PRINT_فیلد_SEGMENT, فیلد->name);
	چاپ_کن(format, فیلد->buf);
}

/**
 * eeprom_field_update_ascii() - Update field with new data in ASCII form
 * @field:	an initialized field
 * @value:	the new string data
 *
 * Returns 0 on success, -1 of failure (new string too long).
 */
صحيح eeprom_فیلد_update_ascii(بنية eeprom_فیلد *فیلد, حرف *مقدار)
{
	إذا (طول_رشته(مقدار) >= فیلد->اندازه) {
		چاپ_کن("%s: new data too long\n", فیلد->name);
		أرجع -1;
	}

	strncpy((حرف *)فیلد->buf, مقدار, فیلد->اندازه - 1);
	فیلد->buf[فیلد->اندازه - 1] = '\0';

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
فراغ eeprom_فیلد_print_reserved(ثابت بنية eeprom_فیلد *فیلد)
{
	چاپ_کن(PRINT_فیلد_SEGMENT, "Reserved fields\t");
	چاپ_کن("(%d bytes)\n", فیلد->اندازه);
}
