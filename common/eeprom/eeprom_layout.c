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

#تعریف_کن NO_LAYOUT_FIELDS	"Unknown layout. Dumping raw data\n"

بنية eeprom_فیلد layout_unknown[1] = {
	{ NO_LAYOUT_FIELDS, 256, عدم, eeprom_فیلد_print_bin,
				       eeprom_فیلد_update_bin },
};

/*
 * eeprom_layout_detect() - detect layout based on the contents of the data.
 * @data: Pointer to the data to be analyzed.
 *
 * Returns: the detected layout version.
 */
__weak صحيح eeprom_layout_detect(بیعلامت حرف *داده)
{
	أرجع LAYOUT_نسخه_UNRECOGNIZED;
}

/*
 * __eeprom_layout_assign() - set the layout fields
 * @layout:		A pointer to an existing struct layout.
 * @layout_version:	The version number of the desired layout
 */
__weak فراغ __eeprom_layout_assign(بنية eeprom_layout *layout,
				   صحيح layout_نسخه)
{
	layout->fields = layout_unknown;
	layout->رقم_از_fields = ARRAY_اندازه(layout_unknown);
}
فراغ eeprom_layout_تخصیص_بده(بنية eeprom_layout *layout, صحيح layout_نسخه) \
		__attribute__((weak, alias("__eeprom_layout_assign")));

/*
 * eeprom_layout_print() - print the layout and the data which is assigned to it
 * @layout: A pointer to an existing struct layout.
 */
ساكن فراغ eeprom_layout_print(ثابت بنية eeprom_layout *layout)
{
	صحيح i;
	بنية eeprom_فیلد *fields = layout->fields;

	لكل (i = 0; i < layout->رقم_از_fields; i++)
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
بنية eeprom_فیلد *eeprom_layout_پیدا_کن_فیلد(بنية eeprom_layout *layout,
					      حرف *فیلد_name, bool warn)
{
	لكل (صحيح i = 0; i < layout->رقم_از_fields; i++)
		إذا (layout->fields[i].name != RESERVED_FIELDS &&
		    !strcmp(layout->fields[i].name, فیلد_name))
			أرجع &layout->fields[i];

	إذا (warn)
		چاپ_کن("No such field '%s'\n", فیلد_name);

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
ساكن صحيح eeprom_layout_update_فیلد(بنية eeprom_layout *layout,
				      حرف *فیلد_name, حرف *new_داده)
{
	بنية eeprom_فیلد *فیلد;
	صحيح err;

	إذا (new_داده == عدم)
		أرجع 0;

	إذا (فیلد_name == عدم)
		أرجع -1;

	فیلد = eeprom_layout_پیدا_کن_فیلد(layout, فیلد_name, true);
	إذا (فیلد == عدم)
		أرجع -1;

	err = فیلد->update(فیلد, new_داده);
	إذا (err)
		چاپ_کن("Invalid data for field %s\n", فیلد_name);

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
فراغ eeprom_layout_راهاندازی(بنية eeprom_layout *layout, بیعلامت حرف *buf,
			 بیعلامت صحيح buf_اندازه, صحيح layout_نسخه)
{
	صحيح i;

	إذا (layout_نسخه == LAYOUT_نسخه_AUTODETECT)
		layout->layout_نسخه = eeprom_layout_detect(buf);
	وإلا
		layout->layout_نسخه = layout_نسخه;

	layout->داده_اندازه = buf_اندازه;
	layout->print = eeprom_layout_print;
	layout->update = eeprom_layout_update_فیلد;

	eeprom_layout_تخصیص_بده(layout, layout_نسخه);
	layout->داده = buf;
	لكل (i = 0; i < layout->رقم_از_fields; i++) {
		layout->fields[i].buf = buf;
		buf += layout->fields[i].اندازه;
	}
}
