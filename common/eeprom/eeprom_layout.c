// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2009-2016 CompuLab, Ltd.
 *
 * Authors: Nikita Kiryanov <nikita@compulab.co.il>
 *	    Igor Grinberg <grinberg@compulab.co.il>
 */

#ਅਨ੍ਤਰ੍ਭਾਵ <linux/kernel.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <linux/string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <eeprom_layout.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <eeprom_field.h>

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ NO_LAYOUT_FIELDS	"Unknown layout. Dumping raw data\n"

ਬਣਤਰ eeprom_ਖੇਤਰ layout_unknown[1] = {
	{ NO_LAYOUT_FIELDS, 256, ਨਿਰਾ, eeprom_ਖੇਤਰ_print_bin,
				       eeprom_ਖੇਤਰ_update_bin },
};

/*
 * eeprom_layout_detect() - detect layout based on the contents of the data.
 * @data: Pointer to the data to be analyzed.
 *
 * Returns: the detected layout version.
 */
__weak ਪੂਰਨ eeprom_layout_detect(ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *ਅੰਕੜੇ)
{
	ਵਾਪਸ LAYOUT_ਸੰਸਕਰਣ_UNRECOGNIZED;
}

/*
 * __eeprom_layout_assign() - set the layout fields
 * @layout:		A pointer to an existing struct layout.
 * @layout_version:	The version number of the desired layout
 */
__weak ਖਾਲੀ __eeprom_layout_assign(ਬਣਤਰ eeprom_layout *layout,
				   ਪੂਰਨ layout_ਸੰਸਕਰਣ)
{
	layout->fields = layout_unknown;
	layout->ਅੰਕ_ਦਾ_fields = ARRAY_ਮਾਪ(layout_unknown);
}
ਖਾਲੀ eeprom_layout_ਸੌਂਪੋ(ਬਣਤਰ eeprom_layout *layout, ਪੂਰਨ layout_ਸੰਸਕਰਣ) \
		__attribute__((weak, alias("__eeprom_layout_assign")));

/*
 * eeprom_layout_print() - print the layout and the data which is assigned to it
 * @layout: A pointer to an existing struct layout.
 */
ਸਥਿਰ ਖਾਲੀ eeprom_layout_print(ਅਚਲ ਬਣਤਰ eeprom_layout *layout)
{
	ਪੂਰਨ i;
	ਬਣਤਰ eeprom_ਖੇਤਰ *fields = layout->fields;

	ਲਈ (i = 0; i < layout->ਅੰਕ_ਦਾ_fields; i++)
		fields[i].print(&fields[i]);
}

/*
 * eeprom_layout_find_field() - finds a layout field by name
 * @layout:	A pointer to an existing struct layout.
 * @field_name:	The name of the field to update.
 * @warn:	Whether to print a warning if the field is not found.
 *
 * Returns: a pointer to the found field or NULL on failure.
 */
ਬਣਤਰ eeprom_ਖੇਤਰ *eeprom_layout_ਲੱਭੋ_ਖੇਤਰ(ਬਣਤਰ eeprom_layout *layout,
					      ਅੱਖਰ *ਖੇਤਰ_name, bool warn)
{
	ਲਈ (ਪੂਰਨ i = 0; i < layout->ਅੰਕ_ਦਾ_fields; i++)
		ਜੇ (layout->fields[i].name != RESERVED_FIELDS &&
		    !strcmp(layout->fields[i].name, ਖੇਤਰ_name))
			ਵਾਪਸ &layout->fields[i];

	ਜੇ (warn)
		ਲਿਖੋ("No such field '%s'\n", ਖੇਤਰ_name);

	ਵਾਪਸ ਨਿਰਾ;
}

/*
 * eeprom_layout_update_field() - update a single field in the layout data.
 * @layout:	A pointer to an existing struct layout.
 * @field_name:	The name of the field to update.
 * @new_data:	The new field data (a string. Format depends on the field)
 *
 * Returns: 0 on success, negative error value on failure.
 */
ਸਥਿਰ ਪੂਰਨ eeprom_layout_update_ਖੇਤਰ(ਬਣਤਰ eeprom_layout *layout,
				      ਅੱਖਰ *ਖੇਤਰ_name, ਅੱਖਰ *new_ਅੰਕੜੇ)
{
	ਬਣਤਰ eeprom_ਖੇਤਰ *ਖੇਤਰ;
	ਪੂਰਨ err;

	ਜੇ (new_ਅੰਕੜੇ == ਨਿਰਾ)
		ਵਾਪਸ 0;

	ਜੇ (ਖੇਤਰ_name == ਨਿਰਾ)
		ਵਾਪਸ -1;

	ਖੇਤਰ = eeprom_layout_ਲੱਭੋ_ਖੇਤਰ(layout, ਖੇਤਰ_name, true);
	ਜੇ (ਖੇਤਰ == ਨਿਰਾ)
		ਵਾਪਸ -1;

	err = ਖੇਤਰ->update(ਖੇਤਰ, new_ਅੰਕੜੇ);
	ਜੇ (err)
		ਲਿਖੋ("Invalid data for field %s\n", ਖੇਤਰ_name);

	ਵਾਪਸ err;
}

/*
 * eeprom_layout_setup() - setup layout struct with the layout data and
 *			   metadata as dictated by layout_version
 * @layout:	A pointer to an existing struct layout.
 * @buf:	A buffer initialized with the eeprom data.
 * @buf_size:	Size of buf in bytes.
 * @layout version: The version number of the layout.
 */
ਖਾਲੀ eeprom_layout_ਤਿਆਰੀ(ਬਣਤਰ eeprom_layout *layout, ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *buf,
			 ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ buf_ਮਾਪ, ਪੂਰਨ layout_ਸੰਸਕਰਣ)
{
	ਪੂਰਨ i;

	ਜੇ (layout_ਸੰਸਕਰਣ == LAYOUT_ਸੰਸਕਰਣ_AUTODETECT)
		layout->layout_ਸੰਸਕਰਣ = eeprom_layout_detect(buf);
	ਨਹੀਂਤਾਂ
		layout->layout_ਸੰਸਕਰਣ = layout_ਸੰਸਕਰਣ;

	layout->ਅੰਕੜੇ_ਮਾਪ = buf_ਮਾਪ;
	layout->print = eeprom_layout_print;
	layout->update = eeprom_layout_update_ਖੇਤਰ;

	eeprom_layout_ਸੌਂਪੋ(layout, layout_ਸੰਸਕਰਣ);
	layout->ਅੰਕੜੇ = buf;
	ਲਈ (i = 0; i < layout->ਅੰਕ_ਦਾ_fields; i++) {
		layout->fields[i].buf = buf;
		buf += layout->fields[i].ਮਾਪ;
	}
}
