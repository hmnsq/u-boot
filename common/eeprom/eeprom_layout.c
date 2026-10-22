// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2009-2016 CompuLab, Ltd.
 *
 * Authors: Nikita Kiryanov <nikita@compulab.co.il>
 *	    Igor Grinberg <grinberg@compulab.co.il>
 */

#ضمن <linux/kernel.h>
#ضمن <linux/string.h>
#ضمن <eeprom_layout.h>
#ضمن <eeprom_field.h>

#define NO_LAYOUT_FIELDS	"Unknown layout. Dumping raw data\n"

بنية eeprom_field layout_unknown[1] = {
	{ NO_LAYOUT_FIELDS, 256, عدم, eeprom_field_print_bin,
				       eeprom_field_update_bin },
};

/*
 * eeprom_layout_detect() - detect layout based on the contents of the data.
 * @data: Pointer to the data to be analyzed.
 *
 * Returns: the detected layout version.
 */
__weak صحيح eeprom_layout_detect(unsigned حرف *data)
{
	أرجع LAYOUT_VERSION_UNRECOGNIZED;
}

/*
 * __eeprom_layout_assign() - set the layout fields
 * @layout:		A pointer to an existing struct layout.
 * @layout_version:	The version number of the desired layout
 */
__weak فراغ __eeprom_layout_assign(بنية eeprom_layout *layout,
				   صحيح layout_version)
{
	layout->fields = layout_unknown;
	layout->num_of_fields = ARRAY_SIZE(layout_unknown);
}
فراغ eeprom_layout_assign(بنية eeprom_layout *layout, صحيح layout_version) \
		__attribute__((weak, alias("__eeprom_layout_assign")));

/*
 * eeprom_layout_print() - print the layout and the data which is assigned to it
 * @layout: A pointer to an existing struct layout.
 */
ساكن فراغ eeprom_layout_print(ثابت بنية eeprom_layout *layout)
{
	صحيح i;
	بنية eeprom_field *fields = layout->fields;

	لكل (i = 0; i < layout->num_of_fields; i++)
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
بنية eeprom_field *eeprom_layout_find_field(بنية eeprom_layout *layout,
					      حرف *field_name, bool warn)
{
	لكل (صحيح i = 0; i < layout->num_of_fields; i++)
		𐤀𐤌 (layout->fields[i].name != RESERVED_FIELDS &&
		    !strcmp(layout->fields[i].name, field_name))
			أرجع &layout->fields[i];

	𐤀𐤌 (warn)
		printf("No such field '%s'\n", field_name);

	أرجع عدم;
}

/*
 * eeprom_layout_update_field() - update a single field in the layout data.
 * @layout:	A pointer to an existing struct layout.
 * @field_name:	The name of the field to update.
 * @new_data:	The new field data (a string. Format depends on the field)
 *
 * Returns: 0 on success, negative error value on failure.
 */
ساكن صحيح eeprom_layout_update_field(بنية eeprom_layout *layout,
				      حرف *field_name, حرف *new_data)
{
	بنية eeprom_field *field;
	صحيح err;

	𐤀𐤌 (new_data == عدم)
		أرجع 0;

	𐤀𐤌 (field_name == عدم)
		أرجع -1;

	field = eeprom_layout_find_field(layout, field_name, true);
	𐤀𐤌 (field == عدم)
		أرجع -1;

	err = field->update(field, new_data);
	𐤀𐤌 (err)
		printf("Invalid data for field %s\n", field_name);

	أرجع err;
}

/*
 * eeprom_layout_setup() - setup layout struct with the layout data and
 *			   metadata as dictated by layout_version
 * @layout:	A pointer to an existing struct layout.
 * @buf:	A buffer initialized with the eeprom data.
 * @buf_size:	Size of buf in bytes.
 * @layout version: The version number of the layout.
 */
فراغ eeprom_layout_setup(بنية eeprom_layout *layout, unsigned حرف *buf,
			 unsigned صحيح buf_size, صحيح layout_version)
{
	صحيح i;

	𐤀𐤌 (layout_version == LAYOUT_VERSION_AUTODETECT)
		layout->layout_version = eeprom_layout_detect(buf);
	وإلا
		layout->layout_version = layout_version;

	layout->data_size = buf_size;
	layout->print = eeprom_layout_print;
	layout->update = eeprom_layout_update_field;

	eeprom_layout_assign(layout, layout_version);
	layout->data = buf;
	لكل (i = 0; i < layout->num_of_fields; i++) {
		layout->fields[i].buf = buf;
		buf += layout->fields[i].size;
	}
}
