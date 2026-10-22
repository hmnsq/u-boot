// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2009-2016 CompuLab, Ltd.
 *
 * Authors: Nikita Kiryanov <nikita@compulab.co.il>
 *	    Igor Grinberg <grinberg@compulab.co.il>
 */

#समावेश <linux/kernel.h>
#समावेश <linux/string.h>
#समावेश <eeprom_layout.h>
#समावेश <eeprom_field.h>

#घोषणा NO_LAYOUT_FIELDS	"Unknown layout. Dumping raw data\n"

काष्ठा eeprom_field layout_unknown[1] = {
	{ NO_LAYOUT_FIELDS, 256, शून्य, eeprom_field_print_bin,
				       eeprom_field_update_bin },
};

/*
 * eeprom_layout_detect() - detect layout based on the contents of the data.
 * @data: Pointer to the data to be analyzed.
 *
 * Returns: the detected layout version.
 */
__weak पूर्णांक eeprom_layout_detect(अचिन्हित अक्षर *तथ्य)
{
	वापस LAYOUT_VERSION_UNRECOGNIZED;
}

/*
 * __eeprom_layout_assign() - set the layout fields
 * @layout:		A pointer to an existing struct layout.
 * @layout_version:	The version number of the desired layout
 */
__weak व्योम __eeprom_layout_assign(काष्ठा eeprom_layout *layout,
				   पूर्णांक layout_version)
{
	layout->fields = layout_unknown;
	layout->अंक_of_fields = ARRAY_आकार(layout_unknown);
}
व्योम eeprom_layout_assign(काष्ठा eeprom_layout *layout, पूर्णांक layout_version) \
		__attribute__((weak, alias("__eeprom_layout_assign")));

/*
 * eeprom_layout_print() - print the layout and the data which is assigned to it
 * @layout: A pointer to an existing struct layout.
 */
अटल व्योम eeprom_layout_print(स्थिर काष्ठा eeprom_layout *layout)
{
	पूर्णांक i;
	काष्ठा eeprom_field *fields = layout->fields;

	क्रम (i = 0; i < layout->अंक_of_fields; i++)
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
काष्ठा eeprom_field *eeprom_layout_find_field(काष्ठा eeprom_layout *layout,
					      अक्षर *field_name, bool warn)
{
	क्रम (पूर्णांक i = 0; i < layout->अंक_of_fields; i++)
		अगर (layout->fields[i].name != RESERVED_FIELDS &&
		    !म_भेद(layout->fields[i].name, field_name))
			वापस &layout->fields[i];

	अगर (warn)
		म_लिखो("No such field '%s'\n", field_name);

	वापस शून्य;
}

/*
 * eeprom_layout_update_field() - update a single field in the layout data.
 * @layout:	A pointer to an existing struct layout.
 * @field_name:	The name of the field to update.
 * @new_data:	The new field data (a string. Format depends on the field)
 *
 * Returns: 0 on success, negative error value on failure.
 */
अटल पूर्णांक eeprom_layout_update_field(काष्ठा eeprom_layout *layout,
				      अक्षर *field_name, अक्षर *new_तथ्य)
{
	काष्ठा eeprom_field *field;
	पूर्णांक err;

	अगर (new_तथ्य == शून्य)
		वापस 0;

	अगर (field_name == शून्य)
		वापस -1;

	field = eeprom_layout_find_field(layout, field_name, true);
	अगर (field == शून्य)
		वापस -1;

	err = field->update(field, new_तथ्य);
	अगर (err)
		म_लिखो("Invalid data for field %s\n", field_name);

	वापस err;
}

/*
 * eeprom_layout_setup() - setup layout struct with the layout data and
 *			   metadata as dictated by layout_version
 * @layout:	A pointer to an existing struct layout.
 * @buf:	A buffer initialized with the eeprom data.
 * @buf_size:	Size of buf in bytes.
 * @layout version: The version number of the layout.
 */
व्योम eeprom_layout_setup(काष्ठा eeprom_layout *layout, अचिन्हित अक्षर *buf,
			 अचिन्हित पूर्णांक buf_आकार, पूर्णांक layout_version)
{
	पूर्णांक i;

	अगर (layout_version == LAYOUT_VERSION_AUTODETECT)
		layout->layout_version = eeprom_layout_detect(buf);
	अन्यथा
		layout->layout_version = layout_version;

	layout->तथ्य_आकार = buf_आकार;
	layout->print = eeprom_layout_print;
	layout->update = eeprom_layout_update_field;

	eeprom_layout_assign(layout, layout_version);
	layout->तथ्य = buf;
	क्रम (i = 0; i < layout->अंक_of_fields; i++) {
		layout->fields[i].buf = buf;
		buf += layout->fields[i].आकार;
	}
}
