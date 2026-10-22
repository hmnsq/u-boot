// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2009-2016 CompuLab, Ltd.
 *
 * Authors: Nikita Kiryanov <nikita@compulab.co.il>
 *	    Igor Grinberg <grinberg@compulab.co.il>
 */

#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <vsprintf.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <linux/string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <eeprom_field.h>

ਸਥਿਰ ਖਾਲੀ __eeprom_field_print_bin(ਅਚਲ ਬਣਤਰ eeprom_ਖੇਤਰ *ਖੇਤਰ,
				     ਅੱਖਰ *delimiter, bool ਉਲਟ)
{
	ਪੂਰਨ i;
	ਪੂਰਨ from = ਉਲਟ ? ਖੇਤਰ->ਮਾਪ - 1 : 0;
	ਪੂਰਨ ਮੰਜ਼ਿਲ = ਉਲਟ ? 0 : ਖੇਤਰ->ਮਾਪ - 1;

	ਲਿਖੋ(PRINT_ਖੇਤਰ_SEGMENT, ਖੇਤਰ->name);
	ਲਈ (i = from; i != ਮੰਜ਼ਿਲ; ਉਲਟ ? i-- : i++)
		ਲਿਖੋ("%02x%s", ਖੇਤਰ->buf[i], delimiter);

	ਲਿਖੋ("%02x\n", ਖੇਤਰ->buf[i]);
}

ਸਥਿਰ ਪੂਰਨ __eeprom_field_update_bin(ਬਣਤਰ eeprom_ਖੇਤਰ *ਖੇਤਰ,
				     ਅਚਲ ਅੱਖਰ *ਮੁੱਲ, bool ਉਲਟ)
{
	ਪੂਰਨ len = ਲੜੀ_ਲੰਬਾਈ(ਮੁੱਲ);
	ਪੂਰਨ k, j, i = ਉਲਟ ? len - 1 : 0;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ byte;
	ਅੱਖਰ *endptr;

	/* each two characters in the string fit in one byte */
	ਜੇ (len > ਖੇਤਰ->ਮਾਪ * 2)
		ਵਾਪਸ -1;

	memset(ਖੇਤਰ->buf, 0, ਖੇਤਰ->ਮਾਪ);

	/* i - string iterator, j - buf iterator */
	ਲਈ (j = 0; j < ਖੇਤਰ->ਮਾਪ; j++) {
		byte = 0;
		ਅੱਖਰ tmp[3] = { 0, 0, 0 };

		ਜੇ ((ਉਲਟ && i < 0) || (!ਉਲਟ && i >= len))
			ਰੋਕੋ;

		ਲਈ (k = 0; k < 2; k++) {
			ਜੇ (ਉਲਟ && i == 0) {
				tmp[k] = ਮੁੱਲ[i];
				ਰੋਕੋ;
			}

			tmp[k] = ਮੁੱਲ[ਉਲਟ ? i - 1 + k : i + k];
		}

		byte = simple_strtoul(tmp, &endptr, 16);
		ਜੇ (*endptr != '\0')
			ਵਾਪਸ -1;

		ਖੇਤਰ->buf[j] = byte;
		i = ਉਲਟ ? i - 2 : i + 2;
	}

	ਵਾਪਸ 0;
}

ਸਥਿਰ ਪੂਰਨ __eeprom_field_update_bin_delim(ਬਣਤਰ eeprom_ਖੇਤਰ *ਖੇਤਰ,
					   ਅੱਖਰ *ਮੁੱਲ, ਅੱਖਰ *delimiter)
{
	ਪੂਰਨ ਗਿਣਤੀ_2 = 0;
	ਪੂਰਨ i, val;
	ਅਚਲ ਅੱਖਰ *tmp = ਮੁੱਲ;
	ਅੱਖਰ *tok;
	ਅੱਖਰ *endptr;

	tmp = ਲੜੀ_ਖੋਜ(tmp, delimiter);
	ਜਦੋਂ (tmp != ਨਿਰਾ) {
		ਗਿਣਤੀ_2++;
		tmp++;
		tmp = ਲੜੀ_ਖੋਜ(tmp, delimiter);
	}

	ਜੇ (ਗਿਣਤੀ_2 > ਖੇਤਰ->ਮਾਪ)
		ਵਾਪਸ -1;

	tok = strtok(ਮੁੱਲ, delimiter);
	ਲਈ (i = 0; tok && i < ਖੇਤਰ->ਮਾਪ; i++) {
		val = simple_strtoul(tok, &endptr, 0);
		ਜੇ (*endptr != '\0')
			ਵਾਪਸ -1;

		/* here we assume that each tok is no more than byte long */
		ਖੇਤਰ->buf[i] = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)val;
		tok = strtok(ਨਿਰਾ, delimiter);
	}

	ਵਾਪਸ 0;
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
ਖਾਲੀ eeprom_ਖੇਤਰ_print_bin(ਅਚਲ ਬਣਤਰ eeprom_ਖੇਤਰ *ਖੇਤਰ)
{
	__eeprom_field_print_bin(ਖੇਤਰ, "", false);
}

/**
 * eeprom_field_update_bin() - Update field with new data in binary form
 *
 * @field:	an initialized field
 * @value:	a string of values (i.e. "10b234a")
 */
ਪੂਰਨ eeprom_ਖੇਤਰ_update_bin(ਬਣਤਰ eeprom_ਖੇਤਰ *ਖੇਤਰ, ਅੱਖਰ *ਮੁੱਲ)
{
	ਵਾਪਸ __eeprom_field_update_bin(ਖੇਤਰ, ਮੁੱਲ, false);
}

/**
 * eeprom_field_update_reserved() - Update reserved field with new data in
 *				    binary form
 *
 * @field:	an initialized field
 * @value:	a space delimited string of byte values (i.e. "1 02 3 0x4")
 */
ਪੂਰਨ eeprom_ਖੇਤਰ_update_reserved(ਬਣਤਰ eeprom_ਖੇਤਰ *ਖੇਤਰ, ਅੱਖਰ *ਮੁੱਲ)
{
	ਵਾਪਸ __eeprom_field_update_bin_delim(ਖੇਤਰ, ਮੁੱਲ, " ");
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
ਖਾਲੀ eeprom_ਖੇਤਰ_print_bin_rev(ਅਚਲ ਬਣਤਰ eeprom_ਖੇਤਰ *ਖੇਤਰ)
{
	__eeprom_field_print_bin(ਖੇਤਰ, "", true);
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
ਪੂਰਨ eeprom_ਖੇਤਰ_update_bin_rev(ਬਣਤਰ eeprom_ਖੇਤਰ *ਖੇਤਰ, ਅੱਖਰ *ਮੁੱਲ)
{
	ਵਾਪਸ __eeprom_field_update_bin(ਖੇਤਰ, ਮੁੱਲ, true);
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
ਖਾਲੀ eeprom_ਖੇਤਰ_print_mac(ਅਚਲ ਬਣਤਰ eeprom_ਖੇਤਰ *ਖੇਤਰ)
{
	__eeprom_field_print_bin(ਖੇਤਰ, ":", false);
}

/**
 * eeprom_field_update_mac() - Update a mac address field which contains binary
 *			       data
 *
 * @field:	an initialized field
 * @value:	a colon delimited string of byte values (i.e. "1:02:3:ff")
 */
ਪੂਰਨ eeprom_ਖੇਤਰ_update_mac(ਬਣਤਰ eeprom_ਖੇਤਰ *ਖੇਤਰ, ਅੱਖਰ *ਮੁੱਲ)
{
	ਵਾਪਸ __eeprom_field_update_bin_delim(ਖੇਤਰ, ਮੁੱਲ, ":");
}

/**
 * eeprom_field_print_ascii() - print a field which contains ASCII data
 * @field:	an initialized field to print
 */
ਖਾਲੀ eeprom_ਖੇਤਰ_print_ascii(ਅਚਲ ਬਣਤਰ eeprom_ਖੇਤਰ *ਖੇਤਰ)
{
	ਅੱਖਰ format[8];

	sprintf(format, "%%.%ds\n", ਖੇਤਰ->ਮਾਪ);
	ਲਿਖੋ(PRINT_ਖੇਤਰ_SEGMENT, ਖੇਤਰ->name);
	ਲਿਖੋ(format, ਖੇਤਰ->buf);
}

/**
 * eeprom_field_update_ascii() - Update field with new data in ASCII form
 * @field:	an initialized field
 * @value:	the new string data
 *
 * Returns 0 on success, -1 of failure (new string too long).
 */
ਪੂਰਨ eeprom_ਖੇਤਰ_update_ascii(ਬਣਤਰ eeprom_ਖੇਤਰ *ਖੇਤਰ, ਅੱਖਰ *ਮੁੱਲ)
{
	ਜੇ (ਲੜੀ_ਲੰਬਾਈ(ਮੁੱਲ) >= ਖੇਤਰ->ਮਾਪ) {
		ਲਿਖੋ("%s: new data too long\n", ਖੇਤਰ->name);
		ਵਾਪਸ -1;
	}

	strncpy((ਅੱਖਰ *)ਖੇਤਰ->buf, ਮੁੱਲ, ਖੇਤਰ->ਮਾਪ - 1);
	ਖੇਤਰ->buf[ਖੇਤਰ->ਮਾਪ - 1] = '\0';

	ਵਾਪਸ 0;
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
ਖਾਲੀ eeprom_ਖੇਤਰ_print_reserved(ਅਚਲ ਬਣਤਰ eeprom_ਖੇਤਰ *ਖੇਤਰ)
{
	ਲਿਖੋ(PRINT_ਖੇਤਰ_SEGMENT, "Reserved fields\t");
	ਲਿਖੋ("(%d bytes)\n", ਖੇਤਰ->ਮਾਪ);
}
