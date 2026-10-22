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

ساكن فراغ __eeprom_field_print_bin(ثابت بنية eeprom_ډګر *ډګر,
				     حرف *delimiter, bool سرچپه)
{
	صحيح i;
	صحيح from = سرچپه ? ډګر->اندازه - 1 : 0;
	صحيح منزل = سرچپه ? 0 : ډګر->اندازه - 1;

	چاپ_کړه(PRINT_ډګر_SEGMENT, ډګر->name);
	لكل (i = from; i != منزل; سرچپه ? i-- : i++)
		چاپ_کړه("%02x%s", ډګر->buf[i], delimiter);

	چاپ_کړه("%02x\n", ډګر->buf[i]);
}

ساكن صحيح __eeprom_field_update_bin(بنية eeprom_ډګر *ډګر,
				     ثابت حرف *ارزښت, bool سرچپه)
{
	صحيح len = مزي_اوږدوالی(ارزښت);
	صحيح k, j, i = سرچپه ? len - 1 : 0;
	بینښه حرف byte;
	حرف *endptr;

	/* each two characters in the string fit in one byte */
	إذا (len > ډګر->اندازه * 2)
		أرجع -1;

	memset(ډګر->buf, 0, ډګر->اندازه);

	/* i - string iterator, j - buf iterator */
	لكل (j = 0; j < ډګر->اندازه; j++) {
		byte = 0;
		حرف tmp[3] = { 0, 0, 0 };

		إذا ((سرچپه && i < 0) || (!سرچپه && i >= len))
			اكسر;

		لكل (k = 0; k < 2; k++) {
			إذا (سرچپه && i == 0) {
				tmp[k] = ارزښت[i];
				اكسر;
			}

			tmp[k] = ارزښت[سرچپه ? i - 1 + k : i + k];
		}

		byte = simple_strtoul(tmp, &endptr, 16);
		إذا (*endptr != '\0')
			أرجع -1;

		ډګر->buf[j] = byte;
		i = سرچپه ? i - 2 : i + 2;
	}

	أرجع 0;
}

ساكن صحيح __eeprom_field_update_bin_delim(بنية eeprom_ډګر *ډګر,
					   حرف *ارزښت, حرف *delimiter)
{
	صحيح شمېر = 0;
	صحيح i, val;
	ثابت حرف *tmp = ارزښت;
	حرف *tok;
	حرف *endptr;

	tmp = مزي_پلټنه(tmp, delimiter);
	طالما (tmp != عدم) {
		شمېر++;
		tmp++;
		tmp = مزي_پلټنه(tmp, delimiter);
	}

	إذا (شمېر > ډګر->اندازه)
		أرجع -1;

	tok = strtok(ارزښت, delimiter);
	لكل (i = 0; tok && i < ډګر->اندازه; i++) {
		val = simple_strtoul(tok, &endptr, 0);
		إذا (*endptr != '\0')
			أرجع -1;

		/* here we assume that each tok is no more than byte long */
		ډګر->buf[i] = (بینښه حرف)val;
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
فراغ eeprom_ډګر_print_bin(ثابت بنية eeprom_ډګر *ډګر)
{
	__eeprom_field_print_bin(ډګر, "", false);
}

/**
 * eeprom_field_update_bin() - Update field with new data in binary form
 *
 * @field:	an initialized field
 * @value:	a string of values (i.e. "10b234a")
 */
صحيح eeprom_ډګر_update_bin(بنية eeprom_ډګر *ډګر, حرف *ارزښت)
{
	أرجع __eeprom_field_update_bin(ډګر, ارزښت, false);
}

/**
 * eeprom_field_update_reserved() - Update reserved field with new data in
 *				    binary form
 *
 * @field:	an initialized field
 * @value:	a space delimited string of byte values (i.e. "1 02 3 0x4")
 */
صحيح eeprom_ډګر_update_reserved(بنية eeprom_ډګر *ډګر, حرف *ارزښت)
{
	أرجع __eeprom_field_update_bin_delim(ډګر, ارزښت, " ");
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
فراغ eeprom_ډګر_print_bin_rev(ثابت بنية eeprom_ډګر *ډګر)
{
	__eeprom_field_print_bin(ډګر, "", true);
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
صحيح eeprom_ډګر_update_bin_rev(بنية eeprom_ډګر *ډګر, حرف *ارزښت)
{
	أرجع __eeprom_field_update_bin(ډګر, ارزښت, true);
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
فراغ eeprom_ډګر_print_mac(ثابت بنية eeprom_ډګر *ډګر)
{
	__eeprom_field_print_bin(ډګر, ":", false);
}

/**
 * eeprom_field_update_mac() - Update a mac address field which contains binary
 *			       data
 *
 * @field:	an initialized field
 * @value:	a colon delimited string of byte values (i.e. "1:02:3:ff")
 */
صحيح eeprom_ډګر_update_mac(بنية eeprom_ډګر *ډګر, حرف *ارزښت)
{
	أرجع __eeprom_field_update_bin_delim(ډګر, ارزښت, ":");
}

/**
 * eeprom_field_print_ascii() - print a field which contains ASCII data
 * @field:	an initialized field to print
 */
فراغ eeprom_ډګر_print_ascii(ثابت بنية eeprom_ډګر *ډګر)
{
	حرف format[8];

	sprintf(format, "%%.%ds\n", ډګر->اندازه);
	چاپ_کړه(PRINT_ډګر_SEGMENT, ډګر->name);
	چاپ_کړه(format, ډګر->buf);
}

/**
 * eeprom_field_update_ascii() - Update field with new data in ASCII form
 * @field:	an initialized field
 * @value:	the new string data
 *
 * Returns 0 on success, -1 of failure (new string too long).
 */
صحيح eeprom_ډګر_update_ascii(بنية eeprom_ډګر *ډګر, حرف *ارزښت)
{
	إذا (مزي_اوږدوالی(ارزښت) >= ډګر->اندازه) {
		چاپ_کړه("%s: new data too long\n", ډګر->name);
		أرجع -1;
	}

	strncpy((حرف *)ډګر->buf, ارزښت, ډګر->اندازه - 1);
	ډګر->buf[ډګر->اندازه - 1] = '\0';

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
فراغ eeprom_ډګر_print_reserved(ثابت بنية eeprom_ډګر *ډګر)
{
	چاپ_کړه(PRINT_ډګر_SEGMENT, "Reserved fields\t");
	چاپ_کړه("(%d bytes)\n", ډګر->اندازه);
}
