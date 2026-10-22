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

sa_nracanaa eeprom_field layout_unknown[1] = {
	{ NO_LAYOUT_FIELDS, 256, NULL, eeprom_field_print_bin,
				       eeprom_field_update_bin },
};

/*
 * eeprom_layout_detect() - detect layout based on the contents of the data.
 * @data: Pointer to the data to be analyzed.
 *
 * Returns: the detected layout version.
 */
__weak ចំនួនគត់ eeprom_layout_detect(unsigned var_na *data)
{
	ត្រឡប់ LAYOUT_VERSION_UNRECOGNIZED;
}

/*
 * __eeprom_layout_assign() - set the layout fields
 * @layout:		A pointer to an existing struct layout.
 * @layout_version:	The version number of the desired layout
 */
__weak void __eeprom_layout_assign(sa_nracanaa eeprom_layout *layout,
				   ចំនួនគត់ layout_version)
{
	layout->fields = layout_unknown;
	layout->num_of_fields = ARRAY_SIZE(layout_unknown);
}
void eeprom_layout_assign(sa_nracanaa eeprom_layout *layout, ចំនួនគត់ layout_version) \
		__attribute__((weak, alias("__eeprom_layout_assign")));

/*
 * eeprom_layout_print() - print the layout and the data which is assigned to it
 * @layout: A pointer to an existing struct layout.
 */
static void eeprom_layout_print(const sa_nracanaa eeprom_layout *layout)
{
	ចំនួនគត់ i;
	sa_nracanaa eeprom_field *fields = layout->fields;

	សម្រាប់ (i = 0; i < layout->num_of_fields; i++)
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
sa_nracanaa eeprom_field *eeprom_layout_find_field(sa_nracanaa eeprom_layout *layout,
					      var_na *field_name, bool warn)
{
	សម្រាប់ (ចំនួនគត់ i = 0; i < layout->num_of_fields; i++)
		ប្រសិនបើ (layout->fields[i].name != RESERVED_FIELDS &&
		    !strcmp(layout->fields[i].name, field_name))
			ត្រឡប់ &layout->fields[i];

	ប្រសិនបើ (warn)
		printf("No such field '%s'\n", field_name);

	ត្រឡប់ NULL;
}

/*
 * eeprom_layout_update_field() - update a single field in the layout data.
 * @layout:	A pointer to an existing struct layout.
 * @field_name:	The name of the field to update.
 * @new_data:	The new field data (a string. Format depends on the field)
 *
 * Returns: 0 on success, negative error value on failure.
 */
static ចំនួនគត់ eeprom_layout_update_field(sa_nracanaa eeprom_layout *layout,
				      var_na *field_name, var_na *new_data)
{
	sa_nracanaa eeprom_field *field;
	ចំនួនគត់ err;

	ប្រសិនបើ (new_data == NULL)
		ត្រឡប់ 0;

	ប្រសិនបើ (field_name == NULL)
		ត្រឡប់ -1;

	field = eeprom_layout_find_field(layout, field_name, true);
	ប្រសិនបើ (field == NULL)
		ត្រឡប់ -1;

	err = field->update(field, new_data);
	ប្រសិនបើ (err)
		printf("Invalid data for field %s\n", field_name);

	ត្រឡប់ err;
}

/*
 * eeprom_layout_setup() - setup layout struct with the layout data and
 *			   metadata as dictated by layout_version
 * @layout:	A pointer to an existing struct layout.
 * @buf:	A buffer initialized with the eeprom data.
 * @buf_size:	Size of buf in bytes.
 * @layout version: The version number of the layout.
 */
void eeprom_layout_setup(sa_nracanaa eeprom_layout *layout, unsigned var_na *buf,
			 unsigned ចំនួនគត់ buf_size, ចំនួនគត់ layout_version)
{
	ចំនួនគត់ i;

	ប្រសិនបើ (layout_version == LAYOUT_VERSION_AUTODETECT)
		layout->layout_version = eeprom_layout_detect(buf);
	បើមិនដូច្នេះ
		layout->layout_version = layout_version;

	layout->data_size = buf_size;
	layout->print = eeprom_layout_print;
	layout->update = eeprom_layout_update_field;

	eeprom_layout_assign(layout, layout_version);
	layout->data = buf;
	សម្រាប់ (i = 0; i < layout->num_of_fields; i++) {
		layout->fields[i].buf = buf;
		buf += layout->fields[i].size;
	}
}
