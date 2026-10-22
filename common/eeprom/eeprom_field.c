// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2009-2016 CompuLab, Ltd.
 *
 * Authors: Nikita Kiryanov <nikita@compulab.co.il>
 *	    Igor Grinberg <grinberg@compulab.co.il>
 */

#समावेश <stdio.h>
#समावेश <vsprintf.h>
#समावेश <linux/string.h>
#समावेश <eeprom_field.h>

अटल व्योम __eeprom_field_print_bin(स्थिर काष्ठा eeprom_field *field,
				     अक्षर *delimiter, bool reverse)
{
	पूर्णांक i;
	पूर्णांक from = reverse ? field->आकार - 1 : 0;
	पूर्णांक to = reverse ? 0 : field->आकार - 1;

	म_लिखो(PRINT_FIELD_SEGMENT, field->name);
	क्रम (i = from; i != to; reverse ? i-- : i++)
		म_लिखो("%02x%s", field->buf[i], delimiter);

	म_लिखो("%02x\n", field->buf[i]);
}

अटल पूर्णांक __eeprom_field_update_bin(काष्ठा eeprom_field *field,
				     स्थिर अक्षर *मूल्य, bool reverse)
{
	पूर्णांक len = म_माप(मूल्य);
	पूर्णांक k, j, i = reverse ? len - 1 : 0;
	अचिन्हित अक्षर byte;
	अक्षर *endptr;

	/* each two characters in the string fit in one byte */
	अगर (len > field->आकार * 2)
		वापस -1;

	स_रखो(field->buf, 0, field->आकार);

	/* i - string iterator, j - buf iterator */
	क्रम (j = 0; j < field->आकार; j++) {
		byte = 0;
		अक्षर tmp[3] = { 0, 0, 0 };

		अगर ((reverse && i < 0) || (!reverse && i >= len))
			अवरोध;

		क्रम (k = 0; k < 2; k++) {
			अगर (reverse && i == 0) {
				tmp[k] = मूल्य[i];
				अवरोध;
			}

			tmp[k] = मूल्य[reverse ? i - 1 + k : i + k];
		}

		byte = simple_strtoul(tmp, &endptr, 16);
		अगर (*endptr != '\0')
			वापस -1;

		field->buf[j] = byte;
		i = reverse ? i - 2 : i + 2;
	}

	वापस 0;
}

अटल पूर्णांक __eeprom_field_update_bin_delim(काष्ठा eeprom_field *field,
					   अक्षर *मूल्य, अक्षर *delimiter)
{
	पूर्णांक गिनती = 0;
	पूर्णांक i, val;
	स्थिर अक्षर *tmp = मूल्य;
	अक्षर *tok;
	अक्षर *endptr;

	tmp = म_माला(tmp, delimiter);
	जबतक (tmp != शून्य) {
		गिनती++;
		tmp++;
		tmp = म_माला(tmp, delimiter);
	}

	अगर (गिनती > field->आकार)
		वापस -1;

	tok = म_मोहर(मूल्य, delimiter);
	क्रम (i = 0; tok && i < field->आकार; i++) {
		val = simple_strtoul(tok, &endptr, 0);
		अगर (*endptr != '\0')
			वापस -1;

		/* here we assume that each tok is no more than byte long */
		field->buf[i] = (अचिन्हित अक्षर)val;
		tok = म_मोहर(शून्य, delimiter);
	}

	वापस 0;
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
व्योम eeprom_field_print_bin(स्थिर काष्ठा eeprom_field *field)
{
	__eeprom_field_print_bin(field, "", false);
}

/**
 * eeprom_field_update_bin() - Update field with new data in binary form
 *
 * @field:	an initialized field
 * @value:	a string of values (i.e. "10b234a")
 */
पूर्णांक eeprom_field_update_bin(काष्ठा eeprom_field *field, अक्षर *मूल्य)
{
	वापस __eeprom_field_update_bin(field, मूल्य, false);
}

/**
 * eeprom_field_update_reserved() - Update reserved field with new data in
 *				    binary form
 *
 * @field:	an initialized field
 * @value:	a space delimited string of byte values (i.e. "1 02 3 0x4")
 */
पूर्णांक eeprom_field_update_reserved(काष्ठा eeprom_field *field, अक्षर *मूल्य)
{
	वापस __eeprom_field_update_bin_delim(field, मूल्य, " ");
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
व्योम eeprom_field_print_bin_rev(स्थिर काष्ठा eeprom_field *field)
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
पूर्णांक eeprom_field_update_bin_rev(काष्ठा eeprom_field *field, अक्षर *मूल्य)
{
	वापस __eeprom_field_update_bin(field, मूल्य, true);
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
व्योम eeprom_field_print_mac(स्थिर काष्ठा eeprom_field *field)
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
पूर्णांक eeprom_field_update_mac(काष्ठा eeprom_field *field, अक्षर *मूल्य)
{
	वापस __eeprom_field_update_bin_delim(field, मूल्य, ":");
}

/**
 * eeprom_field_print_ascii() - print a field which contains ASCII data
 * @field:	an initialized field to print
 */
व्योम eeprom_field_print_ascii(स्थिर काष्ठा eeprom_field *field)
{
	अक्षर format[8];

	प्र_लिखो(format, "%%.%ds\n", field->आकार);
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
पूर्णांक eeprom_field_update_ascii(काष्ठा eeprom_field *field, अक्षर *मूल्य)
{
	अगर (म_माप(मूल्य) >= field->आकार) {
		म_लिखो("%s: new data too long\n", field->name);
		वापस -1;
	}

	म_नकलन((अक्षर *)field->buf, मूल्य, field->आकार - 1);
	field->buf[field->आकार - 1] = '\0';

	वापस 0;
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
व्योम eeprom_field_print_reserved(स्थिर काष्ठा eeprom_field *field)
{
	म_लिखो(PRINT_FIELD_SEGMENT, "Reserved fields\t");
	म_लिखो("(%d bytes)\n", field->आकार);
}
