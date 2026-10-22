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
__weak पूर्णांक eeprom_layout_detect(अचिन्हित अक्षर *data)
{
	परत LAYOUT_VERSION_UNRECOGNIZED;
}

/*
 * __eeprom_layout_assign() - set the layout fields
 * @layout:		A pointer to an existing struct layout.
 * @layout_version:	The version number of the desired layout
 */
__weak शून्य __eeprom_layout_assign(संरचना eeprom_layout *layout,
				   पूर्णांक layout_version)
{
	layout->fields = layout_unknown;
	layout->अंक_of_fields = ARRAY_SIZE(layout_unknown);
}
शून्य eeprom_layout_assign(संरचना eeprom_layout *layout, पूर्णांक layout_version) \
		__attribute__((weak, alias("__eeprom_layout_assign")));

/*
 * eeprom_layout_print() - print the layout and the data which is assigned to it
 * @layout: A pointer to an existing struct layout.
 */
स्थिर शून्य eeprom_layout_print(स्थिरांक संरचना eeprom_layout *layout)
{
	पूर्णांक i;
	संरचना eeprom_field *fields = layout->fields;

	साठी (i = 0; i < layout->अंक_of_fields; i++)
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
					      अक्षर *field_name, bool warn)
{
	साठी (पूर्णांक i = 0; i < layout->अंक_of_fields; i++)
		जर (layout->fields[i].name != RESERVED_FIELDS &&
		    !म_भेद(layout->fields[i].name, field_name))
			परत &layout->fields[i];

	जर (warn)
		म_लिखो("No such field '%s'\n", field_name);

	परत रिक्त;
}

/*
 * eeprom_layout_update_field() - update a single field in the layout data.
 * @layout:	A pointer to an existing struct layout.
 * @field_name:	The name of the field to update.
 * @new_data:	The new field data (a string. Format depends on the field)
 *
 * Returns: 0 on success, negative error value on failure.
 */
स्थिर पूर्णांक eeprom_layout_update_field(संरचना eeprom_layout *layout,
				      अक्षर *field_name, अक्षर *new_data)
{
	संरचना eeprom_field *field;
	पूर्णांक err;

	जर (new_data == रिक्त)
		परत 0;

	जर (field_name == रिक्त)
		परत -1;

	field = eeprom_layout_find_field(layout, field_name, true);
	जर (field == रिक्त)
		परत -1;

	err = field->update(field, new_data);
	जर (err)
		म_लिखो("Invalid data for field %s\n", field_name);

	परत err;
}

/*
 * eeprom_layout_setup() - setup layout struct with the layout data and
 *			   metadata as dictated by layout_version
 * @layout:	A pointer to an existing struct layout.
 * @buf:	A buffer initialized with the eeprom data.
 * @buf_size:	Size of buf in bytes.
 * @layout version: The version number of the layout.
 */
शून्य eeprom_layout_setup(संरचना eeprom_layout *layout, अचिन्हित अक्षर *buf,
			 अचिन्हित पूर्णांक buf_size, पूर्णांक layout_version)
{
	पूर्णांक i;

	जर (layout_version == LAYOUT_VERSION_AUTODETECT)
		layout->layout_version = eeprom_layout_detect(buf);
	अन्यथा
		layout->layout_version = layout_version;

	layout->data_size = buf_size;
	layout->print = eeprom_layout_print;
	layout->update = eeprom_layout_update_field;

	eeprom_layout_assign(layout, layout_version);
	layout->data = buf;
	साठी (i = 0; i < layout->अंक_of_fields; i++) {
		layout->fields[i].buf = buf;
		buf += layout->fields[i].size;
	}
}
