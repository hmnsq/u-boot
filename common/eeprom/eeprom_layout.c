// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2009-2016 CompuLab, Ltd.
 *
 * Authors: Nikita Kiryanov <nikita@compulab.co.il>
 *	    Igor Grinberg <grinberg@compulab.co.il>
 */

#அந்தர்பாவ <linux/kernel.h>
#அந்தர்பாவ <linux/string.h>
#அந்தர்பாவ <eeprom_layout.h>
#அந்தர்பாவ <eeprom_field.h>

#define NO_LAYOUT_FIELDS	"Unknown layout. Dumping raw data\n"

கட்டமைப்பு eeprom_field layout_unknown[1] = {
	{ NO_LAYOUT_FIELDS, 256, வெற்று, eeprom_field_print_bin,
				       eeprom_field_update_bin },
};

/*
 * eeprom_layout_detect() - detect layout based on the contents of the data.
 * @data: Pointer to the data to be analyzed.
 *
 * Returns: the detected layout version.
 */
__weak முழு eeprom_layout_detect(unsigned எழுத்து *data)
{
	திரும்பு LAYOUT_VERSION_UNRECOGNIZED;
}

/*
 * __eeprom_layout_assign() - set the layout fields
 * @layout:		A pointer to an existing struct layout.
 * @layout_version:	The version number of the desired layout
 */
__weak வெறுமை __eeprom_layout_assign(கட்டமைப்பு eeprom_layout *layout,
				   முழு layout_version)
{
	layout->fields = layout_unknown;
	layout->இலக்கம்_of_fields = ARRAY_SIZE(layout_unknown);
}
வெறுமை eeprom_layout_assign(கட்டமைப்பு eeprom_layout *layout, முழு layout_version) \
		__attribute__((weak, alias("__eeprom_layout_assign")));

/*
 * eeprom_layout_print() - print the layout and the data which is assigned to it
 * @layout: A pointer to an existing struct layout.
 */
நிலையான வெறுமை eeprom_layout_print(மாறிலி கட்டமைப்பு eeprom_layout *layout)
{
	முழு i;
	கட்டமைப்பு eeprom_field *fields = layout->fields;

	க்கு (i = 0; i < layout->இலக்கம்_of_fields; i++)
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
கட்டமைப்பு eeprom_field *eeprom_layout_find_field(கட்டமைப்பு eeprom_layout *layout,
					      எழுத்து *field_name, bool warn)
{
	க்கு (முழு i = 0; i < layout->இலக்கம்_of_fields; i++)
		எனில் (layout->fields[i].name != RESERVED_FIELDS &&
		    !strcmp(layout->fields[i].name, field_name))
			திரும்பு &layout->fields[i];

	எனில் (warn)
		printf("No such field '%s'\n", field_name);

	திரும்பு வெற்று;
}

/*
 * eeprom_layout_update_field() - update a single field in the layout data.
 * @layout:	A pointer to an existing struct layout.
 * @field_name:	The name of the field to update.
 * @new_data:	The new field data (a string. Format depends on the field)
 *
 * Returns: 0 on success, negative error value on failure.
 */
நிலையான முழு eeprom_layout_update_field(கட்டமைப்பு eeprom_layout *layout,
				      எழுத்து *field_name, எழுத்து *new_data)
{
	கட்டமைப்பு eeprom_field *field;
	முழு err;

	எனில் (new_data == வெற்று)
		திரும்பு 0;

	எனில் (field_name == வெற்று)
		திரும்பு -1;

	field = eeprom_layout_find_field(layout, field_name, true);
	எனில் (field == வெற்று)
		திரும்பு -1;

	err = field->update(field, new_data);
	எனில் (err)
		printf("Invalid data for field %s\n", field_name);

	திரும்பு err;
}

/*
 * eeprom_layout_setup() - setup layout struct with the layout data and
 *			   metadata as dictated by layout_version
 * @layout:	A pointer to an existing struct layout.
 * @buf:	A buffer initialized with the eeprom data.
 * @buf_size:	Size of buf in bytes.
 * @layout version: The version number of the layout.
 */
வெறுமை eeprom_layout_setup(கட்டமைப்பு eeprom_layout *layout, unsigned எழுத்து *buf,
			 unsigned முழு buf_size, முழு layout_version)
{
	முழு i;

	எனில் (layout_version == LAYOUT_VERSION_AUTODETECT)
		layout->layout_version = eeprom_layout_detect(buf);
	இல்லையெனில்
		layout->layout_version = layout_version;

	layout->data_size = buf_size;
	layout->print = eeprom_layout_print;
	layout->update = eeprom_layout_update_field;

	eeprom_layout_assign(layout, layout_version);
	layout->data = buf;
	க்கு (i = 0; i < layout->இலக்கம்_of_fields; i++) {
		layout->fields[i].buf = buf;
		buf += layout->fields[i].size;
	}
}
