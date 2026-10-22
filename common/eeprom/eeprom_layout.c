// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2009-2016 CompuLab, Ltd.
 *
 * Authors: Nikita Kiryanov <nikita@compulab.co.il>
 *	    Igor Grinberg <grinberg@compulab.co.il>
 */

#ضمن <linux/kernel.h>
#ضمن <linux/string.h>
#ضمن <eeprom_layout.h>
#ضمن <eeprom_field.h>

#وصف_ڪريو NO_LAYOUT_FIELDS	"Unknown layout. Dumping raw data\n"

بنية eeprom_ميدان layout_unknown[1] = {
	{ NO_LAYOUT_FIELDS, 256, عدم, eeprom_ميدان_print_bin,
				       eeprom_ميدان_update_bin },
};

/*
 * eeprom_layout_detect() - detect layout based on the contents of the data.
 * @data: Pointer to the data to be analyzed.
 *
 * Returns: the detected layout version.
 */
__weak صحيح eeprom_layout_detect(بنانشان حرف *معلومات)
{
	أرجع LAYOUT_نسخو_UNRECOGNIZED;
}

/*
 * __eeprom_layout_assign() - set the layout fields
 * @layout:		A pointer to an existing struct layout.
 * @layout_version:	The version number of the desired layout
 */
__weak فراغ __eeprom_layout_assign(بنية eeprom_layout *layout,
				   صحيح layout_نسخو)
{
	layout->fields = layout_unknown;
	layout->انگ_جو_fields = ARRAY_سائيز(layout_unknown);
}
فراغ eeprom_layout_سونپيو(بنية eeprom_layout *layout, صحيح layout_نسخو) \
		__attribute__((weak, alias("__eeprom_layout_assign")));

/*
 * eeprom_layout_print() - print the layout and the data which is assigned to it
 * @layout: A pointer to an existing struct layout.
 */
ساكن فراغ eeprom_layout_print(ثابت بنية eeprom_layout *layout)
{
	صحيح i;
	بنية eeprom_ميدان *fields = layout->fields;

	لكل (i = 0; i < layout->انگ_جو_fields; i++)
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
بنية eeprom_ميدان *eeprom_layout_ڳوليو_ميدان(بنية eeprom_layout *layout,
					      حرف *ميدان_name, bool warn)
{
	لكل (صحيح i = 0; i < layout->انگ_جو_fields; i++)
		إذا (layout->fields[i].name != RESERVED_FIELDS &&
		    !strcmp(layout->fields[i].name, ميدان_name))
			أرجع &layout->fields[i];

	إذا (warn)
		لکو("No such field '%s'\n", ميدان_name);

	أرجع عدم;
}

/*
 * eeprom_layout_update_field() - update a single field in the layout data.
 * @layout:	A pointer to an existing struct layout.
 * @field_name:	The name of the field to update.
 * @new_data:	The new field data (a string. Format depends on the field)
 *
 * Returns: 0 on success, negative error value on failure.
 */
ساكن صحيح eeprom_layout_update_ميدان(بنية eeprom_layout *layout,
				      حرف *ميدان_name, حرف *new_معلومات)
{
	بنية eeprom_ميدان *ميدان;
	صحيح err;

	إذا (new_معلومات == عدم)
		أرجع 0;

	إذا (ميدان_name == عدم)
		أرجع -1;

	ميدان = eeprom_layout_ڳوليو_ميدان(layout, ميدان_name, true);
	إذا (ميدان == عدم)
		أرجع -1;

	err = ميدان->update(ميدان, new_معلومات);
	إذا (err)
		لکو("Invalid data for field %s\n", ميدان_name);

	أرجع err;
}

/*
 * eeprom_layout_setup() - setup layout struct with the layout data and
 *			   metadata as dictated by layout_version
 * @layout:	A pointer to an existing struct layout.
 * @buf:	A buffer initialized with the eeprom data.
 * @buf_size:	Size of buf in bytes.
 * @layout version: The version number of the layout.
 */
فراغ eeprom_layout_تياري(بنية eeprom_layout *layout, بنانشان حرف *buf,
			 بنانشان صحيح buf_سائيز, صحيح layout_نسخو)
{
	صحيح i;

	إذا (layout_نسخو == LAYOUT_نسخو_AUTODETECT)
		layout->layout_نسخو = eeprom_layout_detect(buf);
	وإلا
		layout->layout_نسخو = layout_نسخو;

	layout->معلومات_سائيز = buf_سائيز;
	layout->print = eeprom_layout_print;
	layout->update = eeprom_layout_update_ميدان;

	eeprom_layout_سونپيو(layout, layout_نسخو);
	layout->معلومات = buf;
	لكل (i = 0; i < layout->انگ_جو_fields; i++) {
		layout->fields[i].buf = buf;
		buf += layout->fields[i].سائيز;
	}
}
