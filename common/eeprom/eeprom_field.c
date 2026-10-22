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

स्थिर शून्य __eeprom_field_print_bin(ध्रुव संरचना eeprom_क्षेत्रम् *क्षेत्रम्,
				     वर्ण *delimiter, bool reverse)
{
	पूर्णाङ्क i;
	पूर्णाङ्क from = reverse ? क्षेत्रम्->आकार - 1 : 0;
	पूर्णाङ्क गन्तव्यम् = reverse ? 0 : क्षेत्रम्->आकार - 1;

	प्रारूप_लिख(PRINT_क्षेत्रम्_SEGMENT, क्षेत्रम्->name);
	प्रति (i = from; i != गन्तव्यम्; reverse ? i-- : i++)
		प्रारूप_लिख("%02x%s", क्षेत्रम्->buf[i], delimiter);

	प्रारूप_लिख("%02x\n", क्षेत्रम्->buf[i]);
}

स्थिर पूर्णाङ्क __eeprom_field_update_bin(संरचना eeprom_क्षेत्रम् *क्षेत्रम्,
				     ध्रुव वर्ण *मानम्, bool reverse)
{
	पूर्णाङ्क len = वाक्यदैर्घ्यम्(मानम्);
	पूर्णाङ्क k, j, i = reverse ? len - 1 : 0;
	अचिह्नित वर्ण byte;
	वर्ण *endptr;

	/* each two characters in the string fit in one byte */
	यदि (len > क्षेत्रम्->आकार * 2)
		प्रत्यागम -1;

	स्मृतिं_पूरय(क्षेत्रम्->buf, 0, क्षेत्रम्->आकार);

	/* i - string iterator, j - buf iterator */
	प्रति (j = 0; j < क्षेत्रम्->आकार; j++) {
		byte = 0;
		वर्ण tmp[3] = { 0, 0, 0 };

		यदि ((reverse && i < 0) || (!reverse && i >= len))
			भङ्ग;

		प्रति (k = 0; k < 2; k++) {
			यदि (reverse && i == 0) {
				tmp[k] = मानम्[i];
				भङ्ग;
			}

			tmp[k] = मानम्[reverse ? i - 1 + k : i + k];
		}

		byte = simple_strtoul(tmp, &endptr, 16);
		यदि (*endptr != '\0')
			प्रत्यागम -1;

		क्षेत्रम्->buf[j] = byte;
		i = reverse ? i - 2 : i + 2;
	}

	प्रत्यागम 0;
}

स्थिर पूर्णाङ्क __eeprom_field_update_bin_delim(संरचना eeprom_क्षेत्रम् *क्षेत्रम्,
					   वर्ण *मानम्, वर्ण *delimiter)
{
	पूर्णाङ्क गणनम् = 0;
	पूर्णाङ्क i, val;
	ध्रुव वर्ण *tmp = मानम्;
	वर्ण *tok;
	वर्ण *endptr;

	tmp = वाक्ये_अन्विष(tmp, delimiter);
	यावत् (tmp != रिक्त) {
		गणनम्++;
		tmp++;
		tmp = वाक्ये_अन्विष(tmp, delimiter);
	}

	यदि (गणनम् > क्षेत्रम्->आकार)
		प्रत्यागम -1;

	tok = strtok(मानम्, delimiter);
	प्रति (i = 0; tok && i < क्षेत्रम्->आकार; i++) {
		val = simple_strtoul(tok, &endptr, 0);
		यदि (*endptr != '\0')
			प्रत्यागम -1;

		/* here we assume that each tok is no more than byte long */
		क्षेत्रम्->buf[i] = (अचिह्नित वर्ण)val;
		tok = strtok(रिक्त, delimiter);
	}

	प्रत्यागम 0;
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
शून्य eeprom_क्षेत्रम्_print_bin(ध्रुव संरचना eeprom_क्षेत्रम् *क्षेत्रम्)
{
	__eeprom_field_print_bin(क्षेत्रम्, "", false);
}

/**
 * eeprom_field_update_bin() - Update field with new data in binary form
 *
 * @field:	an initialized field
 * @value:	a string of values (i.e. "10b234a")
 */
पूर्णाङ्क eeprom_क्षेत्रम्_update_bin(संरचना eeprom_क्षेत्रम् *क्षेत्रम्, वर्ण *मानम्)
{
	प्रत्यागम __eeprom_field_update_bin(क्षेत्रम्, मानम्, false);
}

/**
 * eeprom_field_update_reserved() - Update reserved field with new data in
 *				    binary form
 *
 * @field:	an initialized field
 * @value:	a space delimited string of byte values (i.e. "1 02 3 0x4")
 */
पूर्णाङ्क eeprom_क्षेत्रम्_update_reserved(संरचना eeprom_क्षेत्रम् *क्षेत्रम्, वर्ण *मानम्)
{
	प्रत्यागम __eeprom_field_update_bin_delim(क्षेत्रम्, मानम्, " ");
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
शून्य eeprom_क्षेत्रम्_print_bin_rev(ध्रुव संरचना eeprom_क्षेत्रम् *क्षेत्रम्)
{
	__eeprom_field_print_bin(क्षेत्रम्, "", true);
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
पूर्णाङ्क eeprom_क्षेत्रम्_update_bin_rev(संरचना eeprom_क्षेत्रम् *क्षेत्रम्, वर्ण *मानम्)
{
	प्रत्यागम __eeprom_field_update_bin(क्षेत्रम्, मानम्, true);
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
शून्य eeprom_क्षेत्रम्_print_mac(ध्रुव संरचना eeprom_क्षेत्रम् *क्षेत्रम्)
{
	__eeprom_field_print_bin(क्षेत्रम्, ":", false);
}

/**
 * eeprom_field_update_mac() - Update a mac address field which contains binary
 *			       data
 *
 * @field:	an initialized field
 * @value:	a colon delimited string of byte values (i.e. "1:02:3:ff")
 */
पूर्णाङ्क eeprom_क्षेत्रम्_update_mac(संरचना eeprom_क्षेत्रम् *क्षेत्रम्, वर्ण *मानम्)
{
	प्रत्यागम __eeprom_field_update_bin_delim(क्षेत्रम्, मानम्, ":");
}

/**
 * eeprom_field_print_ascii() - print a field which contains ASCII data
 * @field:	an initialized field to print
 */
शून्य eeprom_क्षेत्रम्_print_ascii(ध्रुव संरचना eeprom_क्षेत्रम् *क्षेत्रम्)
{
	वर्ण format[8];

	sprintf(format, "%%.%ds\n", क्षेत्रम्->आकार);
	प्रारूप_लिख(PRINT_क्षेत्रम्_SEGMENT, क्षेत्रम्->name);
	प्रारूप_लिख(format, क्षेत्रम्->buf);
}

/**
 * eeprom_field_update_ascii() - Update field with new data in ASCII form
 * @field:	an initialized field
 * @value:	the new string data
 *
 * Returns 0 on success, -1 of failure (new string too long).
 */
पूर्णाङ्क eeprom_क्षेत्रम्_update_ascii(संरचना eeprom_क्षेत्रम् *क्षेत्रम्, वर्ण *मानम्)
{
	यदि (वाक्यदैर्घ्यम्(मानम्) >= क्षेत्रम्->आकार) {
		प्रारूप_लिख("%s: new data too long\n", क्षेत्रम्->name);
		प्रत्यागम -1;
	}

	strncpy((वर्ण *)क्षेत्रम्->buf, मानम्, क्षेत्रम्->आकार - 1);
	क्षेत्रम्->buf[क्षेत्रम्->आकार - 1] = '\0';

	प्रत्यागम 0;
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
शून्य eeprom_क्षेत्रम्_print_reserved(ध्रुव संरचना eeprom_क्षेत्रम् *क्षेत्रम्)
{
	प्रारूप_लिख(PRINT_क्षेत्रम्_SEGMENT, "Reserved fields\t");
	प्रारूप_लिख("(%d bytes)\n", क्षेत्रम्->आकार);
}
