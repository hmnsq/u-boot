// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2009-2016 CompuLab, Ltd.
 *
 * Authors: Nikita Kiryanov <nikita@compulab.co.il>
 *	    Igor Grinberg <grinberg@compulab.co.il>
 */

#অন্তর্ভাব <linux/kernel.h>
#অন্তর্ভাব <linux/string.h>
#অন্তর্ভাব <eeprom_layout.h>
#অন্তর্ভাব <eeprom_field.h>

#define NO_LAYOUT_FIELDS	"Unknown layout. Dumping raw data\n"

সংরচনা eeprom_field layout_unknown[1] = {
	{ NO_LAYOUT_FIELDS, 256, রিক্ত, eeprom_field_print_bin,
				       eeprom_field_update_bin },
};

/*
 * eeprom_layout_detect() - detect layout based on the contents of the data.
 * @data: Pointer to the data to be analyzed.
 *
 * Returns: the detected layout version.
 */
__weak পূর্ণাঙ্ক eeprom_layout_detect(unsigned বর্ণ *data)
{
	প্রত্যাগম LAYOUT_VERSION_UNRECOGNIZED;
}

/*
 * __eeprom_layout_assign() - set the layout fields
 * @layout:		A pointer to an existing struct layout.
 * @layout_version:	The version number of the desired layout
 */
__weak শূন্য __eeprom_layout_assign(সংরচনা eeprom_layout *layout,
				   পূর্ণাঙ্ক layout_version)
{
	layout->fields = layout_unknown;
	layout->অঙ্ক_of_fields = ARRAY_SIZE(layout_unknown);
}
শূন্য eeprom_layout_assign(সংরচনা eeprom_layout *layout, পূর্ণাঙ্ক layout_version) \
		__attribute__((weak, alias("__eeprom_layout_assign")));

/*
 * eeprom_layout_print() - print the layout and the data which is assigned to it
 * @layout: A pointer to an existing struct layout.
 */
স্থির শূন্য eeprom_layout_print(ধ্রুব সংরচনা eeprom_layout *layout)
{
	পূর্ণাঙ্ক i;
	সংরচনা eeprom_field *fields = layout->fields;

	প্রতি (i = 0; i < layout->অঙ্ক_of_fields; i++)
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
সংরচনা eeprom_field *eeprom_layout_find_field(সংরচনা eeprom_layout *layout,
					      বর্ণ *field_name, bool warn)
{
	প্রতি (পূর্ণাঙ্ক i = 0; i < layout->অঙ্ক_of_fields; i++)
		যদি (layout->fields[i].name != RESERVED_FIELDS &&
		    !strcmp(layout->fields[i].name, field_name))
			প্রত্যাগম &layout->fields[i];

	যদি (warn)
		printf("No such field '%s'\n", field_name);

	প্রত্যাগম রিক্ত;
}

/*
 * eeprom_layout_update_field() - update a single field in the layout data.
 * @layout:	A pointer to an existing struct layout.
 * @field_name:	The name of the field to update.
 * @new_data:	The new field data (a string. Format depends on the field)
 *
 * Returns: 0 on success, negative error value on failure.
 */
স্থির পূর্ণাঙ্ক eeprom_layout_update_field(সংরচনা eeprom_layout *layout,
				      বর্ণ *field_name, বর্ণ *new_data)
{
	সংরচনা eeprom_field *field;
	পূর্ণাঙ্ক err;

	যদি (new_data == রিক্ত)
		প্রত্যাগম 0;

	যদি (field_name == রিক্ত)
		প্রত্যাগম -1;

	field = eeprom_layout_find_field(layout, field_name, true);
	যদি (field == রিক্ত)
		প্রত্যাগম -1;

	err = field->update(field, new_data);
	যদি (err)
		printf("Invalid data for field %s\n", field_name);

	প্রত্যাগম err;
}

/*
 * eeprom_layout_setup() - setup layout struct with the layout data and
 *			   metadata as dictated by layout_version
 * @layout:	A pointer to an existing struct layout.
 * @buf:	A buffer initialized with the eeprom data.
 * @buf_size:	Size of buf in bytes.
 * @layout version: The version number of the layout.
 */
শূন্য eeprom_layout_setup(সংরচনা eeprom_layout *layout, unsigned বর্ণ *buf,
			 unsigned পূর্ণাঙ্ক buf_size, পূর্ণাঙ্ক layout_version)
{
	পূর্ণাঙ্ক i;

	যদি (layout_version == LAYOUT_VERSION_AUTODETECT)
		layout->layout_version = eeprom_layout_detect(buf);
	অন্যথা
		layout->layout_version = layout_version;

	layout->data_size = buf_size;
	layout->print = eeprom_layout_print;
	layout->update = eeprom_layout_update_field;

	eeprom_layout_assign(layout, layout_version);
	layout->data = buf;
	প্রতি (i = 0; i < layout->অঙ্ক_of_fields; i++) {
		layout->fields[i].buf = buf;
		buf += layout->fields[i].size;
	}
}
