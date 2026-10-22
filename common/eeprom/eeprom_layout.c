// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2009-2016 CompuLab, Ltd.
 *
 * Authors: Nikita Kiryanov <nikita@compulab.co.il>
 *	    Igor Grinberg <grinberg@compulab.co.il>
 */

#include <linux/kernel.h>
#include <linux/string.h>
#include <eeprom_layout.h>
#include <eeprom_field.h>

#define NO_LAYOUT_FIELDS	"Unknown layout. Dumping raw data\n"

struct eeprom_field layout_unknown[1] = {
	{ NO_LAYOUT_FIELDS, 256, NULL, eeprom_field_print_bin,
				       eeprom_field_update_bin },
};

/*
 * eeprom_layout_detect() - detect layout based on the contents of the data.
 * @data: Pointer to the data to be analyzed.
 *
 * Returns: the detected layout version.
 */
__weak 整数 eeprom_layout_detect(unsigned char *data)
{
	返回 LAYOUT_VERSION_UNRECOGNIZED;
}

/*
 * __eeprom_layout_assign() - set the layout fields
 * @layout:		A pointer to an existing struct layout.
 * @layout_version:	The version number of the desired layout
 */
__weak void __eeprom_layout_assign(struct eeprom_layout *layout,
				   整数 layout_version)
{
	layout->fields = layout_unknown;
	layout->num_of_fields = ARRAY_SIZE(layout_unknown);
}
void eeprom_layout_assign(struct eeprom_layout *layout, 整数 layout_version) \
		__attribute__((weak, alias("__eeprom_layout_assign")));

/*
 * eeprom_layout_print() - print the layout and the data which is assigned to it
 * @layout: A pointer to an existing struct layout.
 */
static void eeprom_layout_print(const struct eeprom_layout *layout)
{
	整数 i;
	struct eeprom_field *fields = layout->fields;

	为 (i = 0; i < layout->num_of_fields; i++)
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
struct eeprom_field *eeprom_layout_find_field(struct eeprom_layout *layout,
					      char *field_name, bool warn)
{
	为 (整数 i = 0; i < layout->num_of_fields; i++)
		如果 (layout->fields[i].name != RESERVED_FIELDS &&
		    !strcmp(layout->fields[i].name, field_name))
			返回 &layout->fields[i];

	如果 (warn)
		printf("No such field '%s'\n", field_name);

	返回 NULL;
}

/*
 * eeprom_layout_update_field() - update a single field in the layout data.
 * @layout:	A pointer to an existing struct layout.
 * @field_name:	The name of the field to update.
 * @new_data:	The new field data (a string. Format depends on the field)
 *
 * Returns: 0 on success, negative error value on failure.
 */
static 整数 eeprom_layout_update_field(struct eeprom_layout *layout,
				      char *field_name, char *new_data)
{
	struct eeprom_field *field;
	整数 err;

	如果 (new_data == NULL)
		返回 0;

	如果 (field_name == NULL)
		返回 -1;

	field = eeprom_layout_find_field(layout, field_name, true);
	如果 (field == NULL)
		返回 -1;

	err = field->update(field, new_data);
	如果 (err)
		printf("Invalid data for field %s\n", field_name);

	返回 err;
}

/*
 * eeprom_layout_setup() - setup layout struct with the layout data and
 *			   metadata as dictated by layout_version
 * @layout:	A pointer to an existing struct layout.
 * @buf:	A buffer initialized with the eeprom data.
 * @buf_size:	Size of buf in bytes.
 * @layout version: The version number of the layout.
 */
void eeprom_layout_setup(struct eeprom_layout *layout, unsigned char *buf,
			 unsigned 整数 buf_size, 整数 layout_version)
{
	整数 i;

	如果 (layout_version == LAYOUT_VERSION_AUTODETECT)
		layout->layout_version = eeprom_layout_detect(buf);
	否则
		layout->layout_version = layout_version;

	layout->data_size = buf_size;
	layout->print = eeprom_layout_print;
	layout->update = eeprom_layout_update_field;

	eeprom_layout_assign(layout, layout_version);
	layout->data = buf;
	为 (i = 0; i < layout->num_of_fields; i++) {
		layout->fields[i].buf = buf;
		buf += layout->fields[i].size;
	}
}
