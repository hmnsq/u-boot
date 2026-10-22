// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2009-2016 CompuLab, Ltd.
 *
 * Authors: Nikita Kiryanov <nikita@compulab.co.il>
 *	    Igor Grinberg <grinberg@compulab.co.il>
 */

#अन्तर्भाव <linux/kernel.h>
#अन्तर्भाव <linux/string.h>
#अन्तर्भाव <eeprom_layout.h>
#अन्तर्भाव <eeprom_field.h>

#define NO_LAYOUT_FIELDS	"Unknown layout. Dumping raw data\n"

संरचना eeprom_field layout_unknown[1] = {
	{ NO_LAYOUT_FIELDS, 256, रिक्त, eeprom_field_print_bin,
				       eeprom_field_update_bin },
};

/*
 * eeprom_layout_detect() - detect layout based on the contents of the data.
 * @data: Pointer to the data to be analyzed.
 *
 * Returns: the detected layout version.
 */
__weak पूर्णाङ्क eeprom_layout_detect(unsigned वर्ण *data)
{
	प्रत्यागम LAYOUT_VERSION_UNRECOGNIZED;
}

/*
 * __eeprom_layout_assign() - set the layout fields
 * @layout:		A pointer to an existing struct layout.
 * @layout_version:	The version number of the desired layout
 */
__weak शून्य __eeprom_layout_assign(संरचना eeprom_layout *layout,
				   पूर्णाङ्क layout_version)
{
	layout->fields = layout_unknown;
	layout->अंक_of_fields = ARRAY_SIZE(layout_unknown);
}
शून्य eeprom_layout_assign(संरचना eeprom_layout *layout, पूर्णाङ्क layout_version) \
		__attribute__((weak, alias("__eeprom_layout_assign")));

/*
 * eeprom_layout_print() - print the layout and the data which is assigned to it
 * @layout: A pointer to an existing struct layout.
 */
स्थिर शून्य eeprom_layout_print(ध्रुव संरचना eeprom_layout *layout)
{
	पूर्णाङ्क i;
	संरचना eeprom_field *fields = layout->fields;

	प्रति (i = 0; i < layout->अंक_of_fields; i++)
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
संरचना eeprom_field *eeprom_layout_find_field(संरचना eeprom_layout *layout,
					      वर्ण *field_name, bool warn)
{
	प्रति (पूर्णाङ्क i = 0; i < layout->अंक_of_fields; i++)
		यदि (layout->fields[i].name != RESERVED_FIELDS &&
		    !strcmp(layout->fields[i].name, field_name))
			प्रत्यागम &layout->fields[i];

	यदि (warn)
		printf("No such field '%s'\n", field_name);

	प्रत्यागम रिक्त;
}

/*
 * eeprom_layout_update_field() - update a single field in the layout data.
 * @layout:	A pointer to an existing struct layout.
 * @field_name:	The name of the field to update.
 * @new_data:	The new field data (a string. Format depends on the field)
 *
 * Returns: 0 on success, negative error value on failure.
 */
स्थिर पूर्णाङ्क eeprom_layout_update_field(संरचना eeprom_layout *layout,
				      वर्ण *field_name, वर्ण *new_data)
{
	संरचना eeprom_field *field;
	पूर्णाङ्क err;

	यदि (new_data == रिक्त)
		प्रत्यागम 0;

	यदि (field_name == रिक्त)
		प्रत्यागम -1;

	field = eeprom_layout_find_field(layout, field_name, true);
	यदि (field == रिक्त)
		प्रत्यागम -1;

	err = field->update(field, new_data);
	यदि (err)
		printf("Invalid data for field %s\n", field_name);

	प्रत्यागम err;
}

/*
 * eeprom_layout_setup() - setup layout struct with the layout data and
 *			   metadata as dictated by layout_version
 * @layout:	A pointer to an existing struct layout.
 * @buf:	A buffer initialized with the eeprom data.
 * @buf_size:	Size of buf in bytes.
 * @layout version: The version number of the layout.
 */
शून्य eeprom_layout_setup(संरचना eeprom_layout *layout, unsigned वर्ण *buf,
			 unsigned पूर्णाङ्क buf_size, पूर्णाङ्क layout_version)
{
	पूर्णाङ्क i;

	यदि (layout_version == LAYOUT_VERSION_AUTODETECT)
		layout->layout_version = eeprom_layout_detect(buf);
	अन्यथा
		layout->layout_version = layout_version;

	layout->data_size = buf_size;
	layout->print = eeprom_layout_print;
	layout->update = eeprom_layout_update_field;

	eeprom_layout_assign(layout, layout_version);
	layout->data = buf;
	प्रति (i = 0; i < layout->अंक_of_fields; i++) {
		layout->fields[i].buf = buf;
		buf += layout->fields[i].size;
	}
}
