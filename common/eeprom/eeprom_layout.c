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

#परिभाषय NO_LAYOUT_FIELDS	"Unknown layout. Dumping raw data\n"

संरचना eeprom_क्षेत्रम् layout_unknown[1] = {
	{ NO_LAYOUT_FIELDS, 256, रिक्त, eeprom_क्षेत्रम्_print_bin,
				       eeprom_क्षेत्रम्_update_bin },
};

/*
 * eeprom_layout_detect() - detect layout based on the contents of the data.
 * @data: Pointer to the data to be analyzed.
 *
 * Returns: the detected layout version.
 */
__weak पूर्णाङ्क eeprom_layout_detect(अचिह्नित वर्ण *दत्त)
{
	प्रत्यागम LAYOUT_संस्करणम्_UNRECOGNIZED;
}

/*
 * __eeprom_layout_assign() - set the layout fields
 * @layout:		A pointer to an existing struct layout.
 * @layout_version:	The version number of the desired layout
 */
__weak शून्य __eeprom_layout_assign(संरचना eeprom_layout *layout,
				   पूर्णाङ्क layout_संस्करणम्)
{
	layout->fields = layout_unknown;
	layout->अङ्कः_सम्बन्धि_fields = ARRAY_आकार(layout_unknown);
}
शून्य eeprom_layout_नियोजय(संरचना eeprom_layout *layout, पूर्णाङ्क layout_संस्करणम्) \
		__attribute__((weak, alias("__eeprom_layout_assign")));

/*
 * eeprom_layout_print() - print the layout and the data which is assigned to it
 * @layout: A pointer to an existing struct layout.
 */
स्थिर शून्य eeprom_layout_print(ध्रुव संरचना eeprom_layout *layout)
{
	पूर्णाङ्क i;
	संरचना eeprom_क्षेत्रम् *fields = layout->fields;

	प्रति (i = 0; i < layout->अङ्कः_सम्बन्धि_fields; i++)
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
संरचना eeprom_क्षेत्रम् *eeprom_layout_find_क्षेत्रम्(संरचना eeprom_layout *layout,
					      वर्ण *क्षेत्रम्_name, bool warn)
{
	प्रति (पूर्णाङ्क i = 0; i < layout->अङ्कः_सम्बन्धि_fields; i++)
		यदि (layout->fields[i].name != RESERVED_FIELDS &&
		    !वाक्यं_तुलय(layout->fields[i].name, क्षेत्रम्_name))
			प्रत्यागम &layout->fields[i];

	यदि (warn)
		प्रारूप_लिख("No such field '%s'\n", क्षेत्रम्_name);

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
स्थिर पूर्णाङ्क eeprom_layout_update_क्षेत्रम्(संरचना eeprom_layout *layout,
				      वर्ण *क्षेत्रम्_name, वर्ण *new_दत्त)
{
	संरचना eeprom_क्षेत्रम् *क्षेत्रम्;
	पूर्णाङ्क err;

	यदि (new_दत्त == रिक्त)
		प्रत्यागम 0;

	यदि (क्षेत्रम्_name == रिक्त)
		प्रत्यागम -1;

	क्षेत्रम् = eeprom_layout_find_क्षेत्रम्(layout, क्षेत्रम्_name, true);
	यदि (क्षेत्रम् == रिक्त)
		प्रत्यागम -1;

	err = क्षेत्रम्->update(क्षेत्रम्, new_दत्त);
	यदि (err)
		प्रारूप_लिख("Invalid data for field %s\n", क्षेत्रम्_name);

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
शून्य eeprom_layout_सज्जता(संरचना eeprom_layout *layout, अचिह्नित वर्ण *buf,
			 अचिह्नित पूर्णाङ्क buf_आकार, पूर्णाङ्क layout_संस्करणम्)
{
	पूर्णाङ्क i;

	यदि (layout_संस्करणम् == LAYOUT_संस्करणम्_AUTODETECT)
		layout->layout_संस्करणम् = eeprom_layout_detect(buf);
	अन्यथा
		layout->layout_संस्करणम् = layout_संस्करणम्;

	layout->दत्त_आकार = buf_आकार;
	layout->print = eeprom_layout_print;
	layout->update = eeprom_layout_update_क्षेत्रम्;

	eeprom_layout_नियोजय(layout, layout_संस्करणम्);
	layout->दत्त = buf;
	प्रति (i = 0; i < layout->अङ्कः_सम्बन्धि_fields; i++) {
		layout->fields[i].buf = buf;
		buf += layout->fields[i].आकार;
	}
}
