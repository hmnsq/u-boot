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

কাঠামো eeprom_field layout_unknown[1] = {
	{ NO_LAYOUT_FIELDS, 256, ফাঁকা, eeprom_field_print_bin,
				       eeprom_field_update_bin },
};

/*
 * eeprom_layout_detect() - detect layout based on the contents of the data.
 * @data: Pointer to the data to be analyzed.
 *
 * Returns: the detected layout version.
 */
__weak পূর্ণসংখ্যা eeprom_layout_detect(unsigned অক্ষর *তথ্য)
{
	ফেরত LAYOUT_VERSION_UNRECOGNIZED;
}

/*
 * __eeprom_layout_assign() - set the layout fields
 * @layout:		A pointer to an existing struct layout.
 * @layout_version:	The version number of the desired layout
 */
__weak শূন্য __eeprom_layout_assign(কাঠামো eeprom_layout *layout,
				   পূর্ণসংখ্যা layout_version)
{
	layout->fields = layout_unknown;
	layout->অঙ্ক_of_fields = ARRAY_আকার(layout_unknown);
}
শূন্য eeprom_layout_assign(কাঠামো eeprom_layout *layout, পূর্ণসংখ্যা layout_version) \
		__attribute__((weak, alias("__eeprom_layout_assign")));

/*
 * eeprom_layout_print() - print the layout and the data which is assigned to it
 * @layout: A pointer to an existing struct layout.
 */
স্থির শূন্য eeprom_layout_print(ধ্রুবক কাঠামো eeprom_layout *layout)
{
	পূর্ণসংখ্যা i;
	কাঠামো eeprom_field *fields = layout->fields;

	জন্য (i = 0; i < layout->অঙ্ক_of_fields; i++)
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
কাঠামো eeprom_field *eeprom_layout_find_field(কাঠামো eeprom_layout *layout,
					      অক্ষর *field_name, bool warn)
{
	জন্য (পূর্ণসংখ্যা i = 0; i < layout->অঙ্ক_of_fields; i++)
		যদি (layout->fields[i].name != RESERVED_FIELDS &&
		    !strcmp(layout->fields[i].name, field_name))
			ফেরত &layout->fields[i];

	যদি (warn)
		printf("No such field '%s'\n", field_name);

	ফেরত ফাঁকা;
}

/*
 * eeprom_layout_update_field() - update a single field in the layout data.
 * @layout:	A pointer to an existing struct layout.
 * @field_name:	The name of the field to update.
 * @new_data:	The new field data (a string. Format depends on the field)
 *
 * Returns: 0 on success, negative error value on failure.
 */
স্থির পূর্ণসংখ্যা eeprom_layout_update_field(কাঠামো eeprom_layout *layout,
				      অক্ষর *field_name, অক্ষর *new_তথ্য)
{
	কাঠামো eeprom_field *field;
	পূর্ণসংখ্যা err;

	যদি (new_তথ্য == ফাঁকা)
		ফেরত 0;

	যদি (field_name == ফাঁকা)
		ফেরত -1;

	field = eeprom_layout_find_field(layout, field_name, true);
	যদি (field == ফাঁকা)
		ফেরত -1;

	err = field->update(field, new_তথ্য);
	যদি (err)
		printf("Invalid data for field %s\n", field_name);

	ফেরত err;
}

/*
 * eeprom_layout_setup() - setup layout struct with the layout data and
 *			   metadata as dictated by layout_version
 * @layout:	A pointer to an existing struct layout.
 * @buf:	A buffer initialized with the eeprom data.
 * @buf_size:	Size of buf in bytes.
 * @layout version: The version number of the layout.
 */
শূন্য eeprom_layout_setup(কাঠামো eeprom_layout *layout, unsigned অক্ষর *buf,
			 unsigned পূর্ণসংখ্যা buf_আকার, পূর্ণসংখ্যা layout_version)
{
	পূর্ণসংখ্যা i;

	যদি (layout_version == LAYOUT_VERSION_AUTODETECT)
		layout->layout_version = eeprom_layout_detect(buf);
	অন্যথা
		layout->layout_version = layout_version;

	layout->তথ্য_আকার = buf_আকার;
	layout->print = eeprom_layout_print;
	layout->update = eeprom_layout_update_field;

	eeprom_layout_assign(layout, layout_version);
	layout->তথ্য = buf;
	জন্য (i = 0; i < layout->অঙ্ক_of_fields; i++) {
		layout->fields[i].buf = buf;
		buf += layout->fields[i].আকার_2;
	}
}
