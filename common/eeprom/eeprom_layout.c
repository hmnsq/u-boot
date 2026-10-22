// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2009-2016 CompuLab, Ltd.
 *
 * Authors: Nikita Kiryanov <nikita@compulab.co.il>
 *	    Igor Grinberg <grinberg@compulab.co.il>
 */

#ಅನ್ತರ್ಭಾವ <linux/kernel.h>
#ಅನ್ತರ್ಭಾವ <linux/string.h>
#ಅನ್ತರ್ಭಾವ <eeprom_layout.h>
#ಅನ್ತರ್ಭಾವ <eeprom_field.h>

#define NO_LAYOUT_FIELDS	"Unknown layout. Dumping raw data\n"

ಸಂರಚನಾ eeprom_field layout_unknown[1] = {
	{ NO_LAYOUT_FIELDS, 256, ರಿಕ್ತ, eeprom_field_print_bin,
				       eeprom_field_update_bin },
};

/*
 * eeprom_layout_detect() - detect layout based on the contents of the data.
 * @data: Pointer to the data to be analyzed.
 *
 * Returns: the detected layout version.
 */
__weak ಪೂರ್ಣಾಙ್ಕ eeprom_layout_detect(unsigned ವರ್ಣ *data)
{
	ಪ್ರತ್ಯಾಗಮ LAYOUT_VERSION_UNRECOGNIZED;
}

/*
 * __eeprom_layout_assign() - set the layout fields
 * @layout:		A pointer to an existing struct layout.
 * @layout_version:	The version number of the desired layout
 */
__weak ಶೂನ್ಯ __eeprom_layout_assign(ಸಂರಚನಾ eeprom_layout *layout,
				   ಪೂರ್ಣಾಙ್ಕ layout_version)
{
	layout->fields = layout_unknown;
	layout->ಅಂಕೆ_of_fields = ARRAY_SIZE(layout_unknown);
}
ಶೂನ್ಯ eeprom_layout_assign(ಸಂರಚನಾ eeprom_layout *layout, ಪೂರ್ಣಾಙ್ಕ layout_version) \
		__attribute__((weak, alias("__eeprom_layout_assign")));

/*
 * eeprom_layout_print() - print the layout and the data which is assigned to it
 * @layout: A pointer to an existing struct layout.
 */
ಸ್ಥಿರ ಶೂನ್ಯ eeprom_layout_print(ಧ್ರುವ ಸಂರಚನಾ eeprom_layout *layout)
{
	ಪೂರ್ಣಾಙ್ಕ i;
	ಸಂರಚನಾ eeprom_field *fields = layout->fields;

	ಪ್ರತಿ (i = 0; i < layout->ಅಂಕೆ_of_fields; i++)
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
ಸಂರಚನಾ eeprom_field *eeprom_layout_find_field(ಸಂರಚನಾ eeprom_layout *layout,
					      ವರ್ಣ *field_name, bool warn)
{
	ಪ್ರತಿ (ಪೂರ್ಣಾಙ್ಕ i = 0; i < layout->ಅಂಕೆ_of_fields; i++)
		ಯದಿ (layout->fields[i].name != RESERVED_FIELDS &&
		    !strcmp(layout->fields[i].name, field_name))
			ಪ್ರತ್ಯಾಗಮ &layout->fields[i];

	ಯದಿ (warn)
		printf("No such field '%s'\n", field_name);

	ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
}

/*
 * eeprom_layout_update_field() - update a single field in the layout data.
 * @layout:	A pointer to an existing struct layout.
 * @field_name:	The name of the field to update.
 * @new_data:	The new field data (a string. Format depends on the field)
 *
 * Returns: 0 on success, negative error value on failure.
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ eeprom_layout_update_field(ಸಂರಚನಾ eeprom_layout *layout,
				      ವರ್ಣ *field_name, ವರ್ಣ *new_data)
{
	ಸಂರಚನಾ eeprom_field *field;
	ಪೂರ್ಣಾಙ್ಕ err;

	ಯದಿ (new_data == ರಿಕ್ತ)
		ಪ್ರತ್ಯಾಗಮ 0;

	ಯದಿ (field_name == ರಿಕ್ತ)
		ಪ್ರತ್ಯಾಗಮ -1;

	field = eeprom_layout_find_field(layout, field_name, true);
	ಯದಿ (field == ರಿಕ್ತ)
		ಪ್ರತ್ಯಾಗಮ -1;

	err = field->update(field, new_data);
	ಯದಿ (err)
		printf("Invalid data for field %s\n", field_name);

	ಪ್ರತ್ಯಾಗಮ err;
}

/*
 * eeprom_layout_setup() - setup layout struct with the layout data and
 *			   metadata as dictated by layout_version
 * @layout:	A pointer to an existing struct layout.
 * @buf:	A buffer initialized with the eeprom data.
 * @buf_size:	Size of buf in bytes.
 * @layout version: The version number of the layout.
 */
ಶೂನ್ಯ eeprom_layout_setup(ಸಂರಚನಾ eeprom_layout *layout, unsigned ವರ್ಣ *buf,
			 unsigned ಪೂರ್ಣಾಙ್ಕ buf_size, ಪೂರ್ಣಾಙ್ಕ layout_version)
{
	ಪೂರ್ಣಾಙ್ಕ i;

	ಯದಿ (layout_version == LAYOUT_VERSION_AUTODETECT)
		layout->layout_version = eeprom_layout_detect(buf);
	ಅನ್ಯಥಾ
		layout->layout_version = layout_version;

	layout->data_size = buf_size;
	layout->print = eeprom_layout_print;
	layout->update = eeprom_layout_update_field;

	eeprom_layout_assign(layout, layout_version);
	layout->data = buf;
	ಪ್ರತಿ (i = 0; i < layout->ಅಂಕೆ_of_fields; i++) {
		layout->fields[i].buf = buf;
		buf += layout->fields[i].size;
	}
}
