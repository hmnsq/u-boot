// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2009-2016 CompuLab, Ltd.
 *
 * Authors: Nikita Kiryanov <nikita@compulab.co.il>
 *	    Igor Grinberg <grinberg@compulab.co.il>
 */

#અન્તર્ભાવ <linux/kernel.h>
#અન્તર્ભાવ <linux/string.h>
#અન્તર્ભાવ <eeprom_layout.h>
#અન્તર્ભાવ <eeprom_field.h>

#define NO_LAYOUT_FIELDS	"Unknown layout. Dumping raw data\n"

સંરચના eeprom_field layout_unknown[1] = {
	{ NO_LAYOUT_FIELDS, 256, ખાલી, eeprom_field_print_bin,
				       eeprom_field_update_bin },
};

/*
 * eeprom_layout_detect() - detect layout based on the contents of the data.
 * @data: Pointer to the data to be analyzed.
 *
 * Returns: the detected layout version.
 */
__weak પૂર્ણાંક eeprom_layout_detect(unsigned અક્ષર *data)
{
	પાછા LAYOUT_VERSION_UNRECOGNIZED;
}

/*
 * __eeprom_layout_assign() - set the layout fields
 * @layout:		A pointer to an existing struct layout.
 * @layout_version:	The version number of the desired layout
 */
__weak શૂન્ય __eeprom_layout_assign(સંરચના eeprom_layout *layout,
				   પૂર્ણાંક layout_version)
{
	layout->fields = layout_unknown;
	layout->અંક_of_fields = ARRAY_SIZE(layout_unknown);
}
શૂન્ય eeprom_layout_assign(સંરચના eeprom_layout *layout, પૂર્ણાંક layout_version) \
		__attribute__((weak, alias("__eeprom_layout_assign")));

/*
 * eeprom_layout_print() - print the layout and the data which is assigned to it
 * @layout: A pointer to an existing struct layout.
 */
સ્થિર શૂન્ય eeprom_layout_print(અચલ સંરચના eeprom_layout *layout)
{
	પૂર્ણાંક i;
	સંરચના eeprom_field *fields = layout->fields;

	માટે (i = 0; i < layout->અંક_of_fields; i++)
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
સંરચના eeprom_field *eeprom_layout_find_field(સંરચના eeprom_layout *layout,
					      અક્ષર *field_name, bool warn)
{
	માટે (પૂર્ણાંક i = 0; i < layout->અંક_of_fields; i++)
		જો (layout->fields[i].name != RESERVED_FIELDS &&
		    !strcmp(layout->fields[i].name, field_name))
			પાછા &layout->fields[i];

	જો (warn)
		printf("No such field '%s'\n", field_name);

	પાછા ખાલી;
}

/*
 * eeprom_layout_update_field() - update a single field in the layout data.
 * @layout:	A pointer to an existing struct layout.
 * @field_name:	The name of the field to update.
 * @new_data:	The new field data (a string. Format depends on the field)
 *
 * Returns: 0 on success, negative error value on failure.
 */
સ્થિર પૂર્ણાંક eeprom_layout_update_field(સંરચના eeprom_layout *layout,
				      અક્ષર *field_name, અક્ષર *new_data)
{
	સંરચના eeprom_field *field;
	પૂર્ણાંક err;

	જો (new_data == ખાલી)
		પાછા 0;

	જો (field_name == ખાલી)
		પાછા -1;

	field = eeprom_layout_find_field(layout, field_name, true);
	જો (field == ખાલી)
		પાછા -1;

	err = field->update(field, new_data);
	જો (err)
		printf("Invalid data for field %s\n", field_name);

	પાછા err;
}

/*
 * eeprom_layout_setup() - setup layout struct with the layout data and
 *			   metadata as dictated by layout_version
 * @layout:	A pointer to an existing struct layout.
 * @buf:	A buffer initialized with the eeprom data.
 * @buf_size:	Size of buf in bytes.
 * @layout version: The version number of the layout.
 */
શૂન્ય eeprom_layout_setup(સંરચના eeprom_layout *layout, unsigned અક્ષર *buf,
			 unsigned પૂર્ણાંક buf_size, પૂર્ણાંક layout_version)
{
	પૂર્ણાંક i;

	જો (layout_version == LAYOUT_VERSION_AUTODETECT)
		layout->layout_version = eeprom_layout_detect(buf);
	નહીંતર
		layout->layout_version = layout_version;

	layout->data_size = buf_size;
	layout->print = eeprom_layout_print;
	layout->update = eeprom_layout_update_field;

	eeprom_layout_assign(layout, layout_version);
	layout->data = buf;
	માટે (i = 0; i < layout->અંક_of_fields; i++) {
		layout->fields[i].buf = buf;
		buf += layout->fields[i].size;
	}
}
