// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2009-2016 CompuLab, Ltd.
 *
 * Authors: Nikita Kiryanov <nikita@compulab.co.il>
 *	    Igor Grinberg <grinberg@compulab.co.il>
 */

#അന്തര്ഭാവ <linux/kernel.h>
#അന്തര്ഭാവ <linux/string.h>
#അന്തര്ഭാവ <eeprom_layout.h>
#അന്തര്ഭാവ <eeprom_field.h>

#define NO_LAYOUT_FIELDS	"Unknown layout. Dumping raw data\n"

ഘടന eeprom_field layout_unknown[1] = {
	{ NO_LAYOUT_FIELDS, 256, ശൂന്യ, eeprom_field_print_bin,
				       eeprom_field_update_bin },
};

/*
 * eeprom_layout_detect() - detect layout based on the contents of the data.
 * @data: Pointer to the data to be analyzed.
 *
 * Returns: the detected layout version.
 */
__weak പൂർണ്ണസംഖ്യ eeprom_layout_detect(unsigned അക്ഷരം *data)
{
	തിരികെ LAYOUT_VERSION_UNRECOGNIZED;
}

/*
 * __eeprom_layout_assign() - set the layout fields
 * @layout:		A pointer to an existing struct layout.
 * @layout_version:	The version number of the desired layout
 */
__weak ശൂന്യം __eeprom_layout_assign(ഘടന eeprom_layout *layout,
				   പൂർണ്ണസംഖ്യ layout_version)
{
	layout->fields = layout_unknown;
	layout->അക്കം_of_fields = ARRAY_SIZE(layout_unknown);
}
ശൂന്യം eeprom_layout_assign(ഘടന eeprom_layout *layout, പൂർണ്ണസംഖ്യ layout_version) \
		__attribute__((weak, alias("__eeprom_layout_assign")));

/*
 * eeprom_layout_print() - print the layout and the data which is assigned to it
 * @layout: A pointer to an existing struct layout.
 */
സ്ഥിതം ശൂന്യം eeprom_layout_print(സ്ഥിരം ഘടന eeprom_layout *layout)
{
	പൂർണ്ണസംഖ്യ i;
	ഘടന eeprom_field *fields = layout->fields;

	വേണ്ടി (i = 0; i < layout->അക്കം_of_fields; i++)
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
ഘടന eeprom_field *eeprom_layout_find_field(ഘടന eeprom_layout *layout,
					      അക്ഷരം *field_name, bool warn)
{
	വേണ്ടി (പൂർണ്ണസംഖ്യ i = 0; i < layout->അക്കം_of_fields; i++)
		എങ്കിൽ (layout->fields[i].name != RESERVED_FIELDS &&
		    !strcmp(layout->fields[i].name, field_name))
			തിരികെ &layout->fields[i];

	എങ്കിൽ (warn)
		printf("No such field '%s'\n", field_name);

	തിരികെ ശൂന്യ;
}

/*
 * eeprom_layout_update_field() - update a single field in the layout data.
 * @layout:	A pointer to an existing struct layout.
 * @field_name:	The name of the field to update.
 * @new_data:	The new field data (a string. Format depends on the field)
 *
 * Returns: 0 on success, negative error value on failure.
 */
സ്ഥിതം പൂർണ്ണസംഖ്യ eeprom_layout_update_field(ഘടന eeprom_layout *layout,
				      അക്ഷരം *field_name, അക്ഷരം *new_data)
{
	ഘടന eeprom_field *field;
	പൂർണ്ണസംഖ്യ err;

	എങ്കിൽ (new_data == ശൂന്യ)
		തിരികെ 0;

	എങ്കിൽ (field_name == ശൂന്യ)
		തിരികെ -1;

	field = eeprom_layout_find_field(layout, field_name, true);
	എങ്കിൽ (field == ശൂന്യ)
		തിരികെ -1;

	err = field->update(field, new_data);
	എങ്കിൽ (err)
		printf("Invalid data for field %s\n", field_name);

	തിരികെ err;
}

/*
 * eeprom_layout_setup() - setup layout struct with the layout data and
 *			   metadata as dictated by layout_version
 * @layout:	A pointer to an existing struct layout.
 * @buf:	A buffer initialized with the eeprom data.
 * @buf_size:	Size of buf in bytes.
 * @layout version: The version number of the layout.
 */
ശൂന്യം eeprom_layout_setup(ഘടന eeprom_layout *layout, unsigned അക്ഷരം *buf,
			 unsigned പൂർണ്ണസംഖ്യ buf_size, പൂർണ്ണസംഖ്യ layout_version)
{
	പൂർണ്ണസംഖ്യ i;

	എങ്കിൽ (layout_version == LAYOUT_VERSION_AUTODETECT)
		layout->layout_version = eeprom_layout_detect(buf);
	അല്ലെങ്കിൽ
		layout->layout_version = layout_version;

	layout->data_size = buf_size;
	layout->print = eeprom_layout_print;
	layout->update = eeprom_layout_update_field;

	eeprom_layout_assign(layout, layout_version);
	layout->data = buf;
	വേണ്ടി (i = 0; i < layout->അക്കം_of_fields; i++) {
		layout->fields[i].buf = buf;
		buf += layout->fields[i].size;
	}
}
