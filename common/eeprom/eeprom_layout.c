// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2009-2016 CompuLab, Ltd.
 *
 * Authors: Nikita Kiryanov <nikita@compulab.co.il>
 *	    Igor Grinberg <grinberg@compulab.co.il>
 */

#ଅନ୍ତର୍ଭାଵ <linux/kernel.h>
#ଅନ୍ତର୍ଭାଵ <linux/string.h>
#ଅନ୍ତର୍ଭାଵ <eeprom_layout.h>
#ଅନ୍ତର୍ଭାଵ <eeprom_field.h>

#define NO_LAYOUT_FIELDS	"Unknown layout. Dumping raw data\n"

ସଂରଚନା eeprom_field layout_unknown[1] = {
	{ NO_LAYOUT_FIELDS, 256, ଖାଲି, eeprom_field_print_bin,
				       eeprom_field_update_bin },
};

/*
 * eeprom_layout_detect() - detect layout based on the contents of the data.
 * @data: Pointer to the data to be analyzed.
 *
 * Returns: the detected layout version.
 */
__weak ପୂର୍ଣ୍ଣସଂଖ୍ୟା eeprom_layout_detect(unsigned ଅକ୍ଷର *data)
{
	ଫେରନ୍ତୁ LAYOUT_VERSION_UNRECOGNIZED;
}

/*
 * __eeprom_layout_assign() - set the layout fields
 * @layout:		A pointer to an existing struct layout.
 * @layout_version:	The version number of the desired layout
 */
__weak ଶୂନ୍ୟ __eeprom_layout_assign(ସଂରଚନା eeprom_layout *layout,
				   ପୂର୍ଣ୍ଣସଂଖ୍ୟା layout_version)
{
	layout->fields = layout_unknown;
	layout->ଅଙ୍କ_of_fields = ARRAY_SIZE(layout_unknown);
}
ଶୂନ୍ୟ eeprom_layout_assign(ସଂରଚନା eeprom_layout *layout, ପୂର୍ଣ୍ଣସଂଖ୍ୟା layout_version) \
		__attribute__((weak, alias("__eeprom_layout_assign")));

/*
 * eeprom_layout_print() - print the layout and the data which is assigned to it
 * @layout: A pointer to an existing struct layout.
 */
ସ୍ଥିର ଶୂନ୍ୟ eeprom_layout_print(ଅଚଳ ସଂରଚନା eeprom_layout *layout)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
	ସଂରଚନା eeprom_field *fields = layout->fields;

	ପାଇଁ (i = 0; i < layout->ଅଙ୍କ_of_fields; i++)
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
ସଂରଚନା eeprom_field *eeprom_layout_find_field(ସଂରଚନା eeprom_layout *layout,
					      ଅକ୍ଷର *field_name, bool warn)
{
	ପାଇଁ (ପୂର୍ଣ୍ଣସଂଖ୍ୟା i = 0; i < layout->ଅଙ୍କ_of_fields; i++)
		ଯଦି (layout->fields[i].name != RESERVED_FIELDS &&
		    !strcmp(layout->fields[i].name, field_name))
			ଫେରନ୍ତୁ &layout->fields[i];

	ଯଦି (warn)
		printf("No such field '%s'\n", field_name);

	ଫେରନ୍ତୁ ଖାଲି;
}

/*
 * eeprom_layout_update_field() - update a single field in the layout data.
 * @layout:	A pointer to an existing struct layout.
 * @field_name:	The name of the field to update.
 * @new_data:	The new field data (a string. Format depends on the field)
 *
 * Returns: 0 on success, negative error value on failure.
 */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା eeprom_layout_update_field(ସଂରଚନା eeprom_layout *layout,
				      ଅକ୍ଷର *field_name, ଅକ୍ଷର *new_data)
{
	ସଂରଚନା eeprom_field *field;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା err;

	ଯଦି (new_data == ଖାଲି)
		ଫେରନ୍ତୁ 0;

	ଯଦି (field_name == ଖାଲି)
		ଫେରନ୍ତୁ -1;

	field = eeprom_layout_find_field(layout, field_name, true);
	ଯଦି (field == ଖାଲି)
		ଫେରନ୍ତୁ -1;

	err = field->update(field, new_data);
	ଯଦି (err)
		printf("Invalid data for field %s\n", field_name);

	ଫେରନ୍ତୁ err;
}

/*
 * eeprom_layout_setup() - setup layout struct with the layout data and
 *			   metadata as dictated by layout_version
 * @layout:	A pointer to an existing struct layout.
 * @buf:	A buffer initialized with the eeprom data.
 * @buf_size:	Size of buf in bytes.
 * @layout version: The version number of the layout.
 */
ଶୂନ୍ୟ eeprom_layout_setup(ସଂରଚନା eeprom_layout *layout, unsigned ଅକ୍ଷର *buf,
			 unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା buf_size, ପୂର୍ଣ୍ଣସଂଖ୍ୟା layout_version)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

	ଯଦି (layout_version == LAYOUT_VERSION_AUTODETECT)
		layout->layout_version = eeprom_layout_detect(buf);
	ଅନ୍ୟଥା
		layout->layout_version = layout_version;

	layout->data_size = buf_size;
	layout->print = eeprom_layout_print;
	layout->update = eeprom_layout_update_field;

	eeprom_layout_assign(layout, layout_version);
	layout->data = buf;
	ପାଇଁ (i = 0; i < layout->ଅଙ୍କ_of_fields; i++) {
		layout->fields[i].buf = buf;
		buf += layout->fields[i].size;
	}
}
