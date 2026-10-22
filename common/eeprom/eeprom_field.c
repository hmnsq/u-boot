// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2009-2016 CompuLab, Ltd.
 *
 * Authors: Nikita Kiryanov <nikita@compulab.co.il>
 *	    Igor Grinberg <grinberg@compulab.co.il>
 */

#अन्तर्भाव <stdio.h>
#अन्तर्भाव <vsprintf.h>
#अन्तर्भाव <linux/string.h>
#अन्तर्भाव <eeprom_field.h>

स्थिर शून्य __eeprom_field_print_bin(अचर संरचना eeprom_field *field,
				     वर्ण *delimiter, bool reverse)
{
	पूर्णाङ्क i;
	पूर्णाङ्क from = reverse ? field->size - 1 : 0;
	पूर्णाङ्क to = reverse ? 0 : field->size - 1;

	म_लिखो(PRINT_FIELD_SEGMENT, field->name);
	लागि (i = from; i != to; reverse ? i-- : i++)
		म_लिखो("%02x%s", field->buf[i], delimiter);

	म_लिखो("%02x\n", field->buf[i]);
}

स्थिर पूर्णाङ्क __eeprom_field_update_bin(संरचना eeprom_field *field,
				     अचर वर्ण *मान, bool reverse)
{
	पूर्णाङ्क len = म_माप(मान);
	पूर्णाङ्क k, j, i = reverse ? len - 1 : 0;
	अचिन्हित वर्ण byte;
	वर्ण *endptr;

	/* each two characters in the string fit in one byte */
	यदि (len > field->size * 2)
		फर्काऊ -1;

	स_रखो(field->buf, 0, field->size);

	/* i - string iterator, j - buf iterator */
	लागि (j = 0; j < field->size; j++) {
		byte = 0;
		वर्ण tmp[3] = { 0, 0, 0 };

		यदि ((reverse && i < 0) || (!reverse && i >= len))
			रोक;

		लागि (k = 0; k < 2; k++) {
			यदि (reverse && i == 0) {
				tmp[k] = मान[i];
				रोक;
			}

			tmp[k] = मान[reverse ? i - 1 + k : i + k];
		}

		byte = simple_strtoul(tmp, &endptr, 16);
		यदि (*endptr != '\0')
			फर्काऊ -1;

		field->buf[j] = byte;
		i = reverse ? i - 2 : i + 2;
	}

	फर्काऊ 0;
}

स्थिर पूर्णाङ्क __eeprom_field_update_bin_delim(संरचना eeprom_field *field,
					   वर्ण *मान, वर्ण *delimiter)
{
	पूर्णाङ्क गन्ती = 0;
	पूर्णाङ्क i, val;
	अचर वर्ण *tmp = मान;
	वर्ण *tok;
	वर्ण *endptr;

	tmp = म_माला(tmp, delimiter);
	जबसम्म (tmp != रिक्त) {
		गन्ती++;
		tmp++;
		tmp = म_माला(tmp, delimiter);
	}

	यदि (गन्ती > field->size)
		फर्काऊ -1;

	tok = म_मोहर(मान, delimiter);
	लागि (i = 0; tok && i < field->size; i++) {
		val = simple_strtoul(tok, &endptr, 0);
		यदि (*endptr != '\0')
			फर्काऊ -1;

		/* here we assume that each tok is no more than byte long */
		field->buf[i] = (अचिन्हित वर्ण)val;
		tok = म_मोहर(रिक्त, delimiter);
	}

	फर्काऊ 0;
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
शून्य eeprom_field_print_bin(अचर संरचना eeprom_field *field)
{
	__eeprom_field_print_bin(field, "", false);
}

/**
 * eeprom_field_update_bin() - Update field with new data in binary form
 *
 * @field:	an initialized field
 * @value:	a string of values (i.e. "10b234a")
 */
पूर्णाङ्क eeprom_field_update_bin(संरचना eeprom_field *field, वर्ण *मान)
{
	फर्काऊ __eeprom_field_update_bin(field, मान, false);
}

/**
 * eeprom_field_update_reserved() - Update reserved field with new data in
 *				    binary form
 *
 * @field:	an initialized field
 * @value:	a space delimited string of byte values (i.e. "1 02 3 0x4")
 */
पूर्णाङ्क eeprom_field_update_reserved(संरचना eeprom_field *field, वर्ण *मान)
{
	फर्काऊ __eeprom_field_update_bin_delim(field, मान, " ");
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
शून्य eeprom_field_print_bin_rev(अचर संरचना eeprom_field *field)
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
पूर्णाङ्क eeprom_field_update_bin_rev(संरचना eeprom_field *field, वर्ण *मान)
{
	फर्काऊ __eeprom_field_update_bin(field, मान, true);
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
शून्य eeprom_field_print_mac(अचर संरचना eeprom_field *field)
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
पूर्णाङ्क eeprom_field_update_mac(संरचना eeprom_field *field, वर्ण *मान)
{
	फर्काऊ __eeprom_field_update_bin_delim(field, मान, ":");
}

/**
 * eeprom_field_print_ascii() - print a field which contains ASCII data
 * @field:	an initialized field to print
 */
शून्य eeprom_field_print_ascii(अचर संरचना eeprom_field *field)
{
	वर्ण format[8];

	प्र_लिखो(format, "%%.%ds\n", field->size);
	म_लिखो(PRINT_FIELD_SEGMENT, field->name);
	म_लिखो(format, field->buf);
}

/**
 * eeprom_field_update_ascii() - Update field with new data in ASCII form
 * @field:	an initialized field
 * @value:	the new string data
 *
 * Returns 0 on success, -1 of failure (new string too long).
 */
पूर्णाङ्क eeprom_field_update_ascii(संरचना eeprom_field *field, वर्ण *मान)
{
	यदि (म_माप(मान) >= field->size) {
		म_लिखो("%s: new data too long\n", field->name);
		फर्काऊ -1;
	}

	म_नकलन((वर्ण *)field->buf, मान, field->size - 1);
	field->buf[field->size - 1] = '\0';

	फर्काऊ 0;
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
शून्य eeprom_field_print_reserved(अचर संरचना eeprom_field *field)
{
	म_लिखो(PRINT_FIELD_SEGMENT, "Reserved fields\t");
	म_लिखो("(%d bytes)\n", field->size);
}
