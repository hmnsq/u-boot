// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2009-2016 CompuLab, Ltd.
 *
 * Authors: Nikita Kiryanov <nikita@compulab.co.il>
 *	    Igor Grinberg <grinberg@compulab.co.il>
 */

#అన్తర్భావ <stdio.h>
#అన్తర్భావ <vsprintf.h>
#అన్తర్భావ <linux/string.h>
#అన్తర్భావ <eeprom_field.h>

స్థిర శూన్య __eeprom_field_print_bin(స్థిరరాశి నిర్మాణం eeprom_క్షేత్రం *క్షేత్రం,
				     అక్షరం *delimiter, bool తిరగేయి)
{
	పూర్ణాంకం i;
	పూర్ణాంకం from = తిరగేయి ? క్షేత్రం->కొలత - 1 : 0;
	పూర్ణాంకం గమ్యం = తిరగేయి ? 0 : క్షేత్రం->కొలత - 1;

	రాయి(PRINT_క్షేత్రం_SEGMENT, క్షేత్రం->name);
	కొరకు (i = from; i != గమ్యం; తిరగేయి ? i-- : i++)
		రాయి("%02x%s", క్షేత్రం->buf[i], delimiter);

	రాయి("%02x\n", క్షేత్రం->buf[i]);
}

స్థిర పూర్ణాంకం __eeprom_field_update_bin(నిర్మాణం eeprom_క్షేత్రం *క్షేత్రం,
				     స్థిరరాశి అక్షరం *విలువ, bool తిరగేయి)
{
	పూర్ణాంకం len = అక్షరశ్రేణి_పొడవు(విలువ);
	పూర్ణాంకం k, j, i = తిరగేయి ? len - 1 : 0;
	గుర్తులేని అక్షరం byte;
	అక్షరం *endptr;

	/* each two characters in the string fit in one byte */
	ఒకవేళ (len > క్షేత్రం->కొలత * 2)
		తిరిగి -1;

	memset(క్షేత్రం->buf, 0, క్షేత్రం->కొలత);

	/* i - string iterator, j - buf iterator */
	కొరకు (j = 0; j < క్షేత్రం->కొలత; j++) {
		byte = 0;
		అక్షరం tmp[3] = { 0, 0, 0 };

		ఒకవేళ ((తిరగేయి && i < 0) || (!తిరగేయి && i >= len))
			ఆపు;

		కొరకు (k = 0; k < 2; k++) {
			ఒకవేళ (తిరగేయి && i == 0) {
				tmp[k] = విలువ[i];
				ఆపు;
			}

			tmp[k] = విలువ[తిరగేయి ? i - 1 + k : i + k];
		}

		byte = simple_strtoul(tmp, &endptr, 16);
		ఒకవేళ (*endptr != '\0')
			తిరిగి -1;

		క్షేత్రం->buf[j] = byte;
		i = తిరగేయి ? i - 2 : i + 2;
	}

	తిరిగి 0;
}

స్థిర పూర్ణాంకం __eeprom_field_update_bin_delim(నిర్మాణం eeprom_క్షేత్రం *క్షేత్రం,
					   అక్షరం *విలువ, అక్షరం *delimiter)
{
	పూర్ణాంకం లెక్క = 0;
	పూర్ణాంకం i, val;
	స్థిరరాశి అక్షరం *tmp = విలువ;
	అక్షరం *tok;
	అక్షరం *endptr;

	tmp = అక్షరశ్రేణి_శోధన(tmp, delimiter);
	అయితే (tmp != ఖాళీ) {
		లెక్క++;
		tmp++;
		tmp = అక్షరశ్రేణి_శోధన(tmp, delimiter);
	}

	ఒకవేళ (లెక్క > క్షేత్రం->కొలత)
		తిరిగి -1;

	tok = strtok(విలువ, delimiter);
	కొరకు (i = 0; tok && i < క్షేత్రం->కొలత; i++) {
		val = simple_strtoul(tok, &endptr, 0);
		ఒకవేళ (*endptr != '\0')
			తిరిగి -1;

		/* here we assume that each tok is no more than byte long */
		క్షేత్రం->buf[i] = (గుర్తులేని అక్షరం)val;
		tok = strtok(ఖాళీ, delimiter);
	}

	తిరిగి 0;
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
శూన్య eeprom_క్షేత్రం_print_bin(స్థిరరాశి నిర్మాణం eeprom_క్షేత్రం *క్షేత్రం)
{
	__eeprom_field_print_bin(క్షేత్రం, "", false);
}

/**
 * eeprom_field_update_bin() - Update field with new data in binary form
 *
 * @field:	an initialized field
 * @value:	a string of values (i.e. "10b234a")
 */
పూర్ణాంకం eeprom_క్షేత్రం_update_bin(నిర్మాణం eeprom_క్షేత్రం *క్షేత్రం, అక్షరం *విలువ)
{
	తిరిగి __eeprom_field_update_bin(క్షేత్రం, విలువ, false);
}

/**
 * eeprom_field_update_reserved() - Update reserved field with new data in
 *				    binary form
 *
 * @field:	an initialized field
 * @value:	a space delimited string of byte values (i.e. "1 02 3 0x4")
 */
పూర్ణాంకం eeprom_క్షేత్రం_update_reserved(నిర్మాణం eeprom_క్షేత్రం *క్షేత్రం, అక్షరం *విలువ)
{
	తిరిగి __eeprom_field_update_bin_delim(క్షేత్రం, విలువ, " ");
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
శూన్య eeprom_క్షేత్రం_print_bin_rev(స్థిరరాశి నిర్మాణం eeprom_క్షేత్రం *క్షేత్రం)
{
	__eeprom_field_print_bin(క్షేత్రం, "", true);
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
పూర్ణాంకం eeprom_క్షేత్రం_update_bin_rev(నిర్మాణం eeprom_క్షేత్రం *క్షేత్రం, అక్షరం *విలువ)
{
	తిరిగి __eeprom_field_update_bin(క్షేత్రం, విలువ, true);
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
శూన్య eeprom_క్షేత్రం_print_mac(స్థిరరాశి నిర్మాణం eeprom_క్షేత్రం *క్షేత్రం)
{
	__eeprom_field_print_bin(క్షేత్రం, ":", false);
}

/**
 * eeprom_field_update_mac() - Update a mac address field which contains binary
 *			       data
 *
 * @field:	an initialized field
 * @value:	a colon delimited string of byte values (i.e. "1:02:3:ff")
 */
పూర్ణాంకం eeprom_క్షేత్రం_update_mac(నిర్మాణం eeprom_క్షేత్రం *క్షేత్రం, అక్షరం *విలువ)
{
	తిరిగి __eeprom_field_update_bin_delim(క్షేత్రం, విలువ, ":");
}

/**
 * eeprom_field_print_ascii() - print a field which contains ASCII data
 * @field:	an initialized field to print
 */
శూన్య eeprom_క్షేత్రం_print_ascii(స్థిరరాశి నిర్మాణం eeprom_క్షేత్రం *క్షేత్రం)
{
	అక్షరం format[8];

	sprintf(format, "%%.%ds\n", క్షేత్రం->కొలత);
	రాయి(PRINT_క్షేత్రం_SEGMENT, క్షేత్రం->name);
	రాయి(format, క్షేత్రం->buf);
}

/**
 * eeprom_field_update_ascii() - Update field with new data in ASCII form
 * @field:	an initialized field
 * @value:	the new string data
 *
 * Returns 0 on success, -1 of failure (new string too long).
 */
పూర్ణాంకం eeprom_క్షేత్రం_update_ascii(నిర్మాణం eeprom_క్షేత్రం *క్షేత్రం, అక్షరం *విలువ)
{
	ఒకవేళ (అక్షరశ్రేణి_పొడవు(విలువ) >= క్షేత్రం->కొలత) {
		రాయి("%s: new data too long\n", క్షేత్రం->name);
		తిరిగి -1;
	}

	strncpy((అక్షరం *)క్షేత్రం->buf, విలువ, క్షేత్రం->కొలత - 1);
	క్షేత్రం->buf[క్షేత్రం->కొలత - 1] = '\0';

	తిరిగి 0;
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
శూన్య eeprom_క్షేత్రం_print_reserved(స్థిరరాశి నిర్మాణం eeprom_క్షేత్రం *క్షేత్రం)
{
	రాయి(PRINT_క్షేత్రం_SEGMENT, "Reserved fields\t");
	రాయి("(%d bytes)\n", క్షేత్రం->కొలత);
}
