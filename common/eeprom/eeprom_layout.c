// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2009-2016 CompuLab, Ltd.
 *
 * Authors: Nikita Kiryanov <nikita@compulab.co.il>
 *	    Igor Grinberg <grinberg@compulab.co.il>
 */

#شامل <linux/kernel.h>
#شامل <linux/string.h>
#شامل <eeprom_layout.h>
#شامل <eeprom_field.h>

#define NO_LAYOUT_FIELDS	"Unknown layout. Dumping raw data\n"

ساختار eeprom_field layout_unknown[1] = {
	{ NO_LAYOUT_FIELDS, 256, پوچ, eeprom_field_print_bin,
				       eeprom_field_update_bin },
};

/*
 * eeprom_layout_detect() - detect layout based on the contents of the data.
 * @data: Pointer to the data to be analyzed.
 *
 * Returns: the detected layout version.
 */
__weak صحیح eeprom_layout_detect(unsigned نویسه *data)
{
	بازگشت LAYOUT_VERSION_UNRECOGNIZED;
}

/*
 * __eeprom_layout_assign() - set the layout fields
 * @layout:		A pointer to an existing struct layout.
 * @layout_version:	The version number of the desired layout
 */
__weak تهی __eeprom_layout_assign(ساختار eeprom_layout *layout,
				   صحیح layout_version)
{
	layout->fields = layout_unknown;
	layout->رقم_of_fields = ARRAY_SIZE(layout_unknown);
}
تهی eeprom_layout_assign(ساختار eeprom_layout *layout, صحیح layout_version) \
		__attribute__((weak, alias("__eeprom_layout_assign")));

/*
 * eeprom_layout_print() - print the layout and the data which is assigned to it
 * @layout: A pointer to an existing struct layout.
 */
ایستا تهی eeprom_layout_print(ثابت ساختار eeprom_layout *layout)
{
	صحیح i;
	ساختار eeprom_field *fields = layout->fields;

	برای (i = 0; i < layout->رقم_of_fields; i++)
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
ساختار eeprom_field *eeprom_layout_find_field(ساختار eeprom_layout *layout,
					      نویسه *field_name, bool warn)
{
	برای (صحیح i = 0; i < layout->رقم_of_fields; i++)
		اگر (layout->fields[i].name != RESERVED_FIELDS &&
		    !strcmp(layout->fields[i].name, field_name))
			بازگشت &layout->fields[i];

	اگر (warn)
		printf("No such field '%s'\n", field_name);

	بازگشت پوچ;
}

/*
 * eeprom_layout_update_field() - update a single field in the layout data.
 * @layout:	A pointer to an existing struct layout.
 * @field_name:	The name of the field to update.
 * @new_data:	The new field data (a string. Format depends on the field)
 *
 * Returns: 0 on success, negative error value on failure.
 */
ایستا صحیح eeprom_layout_update_field(ساختار eeprom_layout *layout,
				      نویسه *field_name, نویسه *new_data)
{
	ساختار eeprom_field *field;
	صحیح err;

	اگر (new_data == پوچ)
		بازگشت 0;

	اگر (field_name == پوچ)
		بازگشت -1;

	field = eeprom_layout_find_field(layout, field_name, true);
	اگر (field == پوچ)
		بازگشت -1;

	err = field->update(field, new_data);
	اگر (err)
		printf("Invalid data for field %s\n", field_name);

	بازگشت err;
}

/*
 * eeprom_layout_setup() - setup layout struct with the layout data and
 *			   metadata as dictated by layout_version
 * @layout:	A pointer to an existing struct layout.
 * @buf:	A buffer initialized with the eeprom data.
 * @buf_size:	Size of buf in bytes.
 * @layout version: The version number of the layout.
 */
تهی eeprom_layout_setup(ساختار eeprom_layout *layout, unsigned نویسه *buf,
			 unsigned صحیح buf_size, صحیح layout_version)
{
	صحیح i;

	اگر (layout_version == LAYOUT_VERSION_AUTODETECT)
		layout->layout_version = eeprom_layout_detect(buf);
	وگرنه
		layout->layout_version = layout_version;

	layout->data_size = buf_size;
	layout->print = eeprom_layout_print;
	layout->update = eeprom_layout_update_field;

	eeprom_layout_assign(layout, layout_version);
	layout->data = buf;
	برای (i = 0; i < layout->رقم_of_fields; i++) {
		layout->fields[i].buf = buf;
		buf += layout->fields[i].size;
	}
}
