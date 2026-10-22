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

#تعریف_کرو NO_LAYOUT_FIELDS	"Unknown layout. Dumping raw data\n"

بنية eeprom_میدان layout_unknown[1] = {
	{ NO_LAYOUT_FIELDS, 256, عدم, eeprom_میدان_print_bin,
				       eeprom_میدان_update_bin },
};

/*
 * eeprom_layout_detect() - detect layout based on the contents of the data.
 * @data: Pointer to the data to be analyzed.
 *
 * Returns: the detected layout version.
 */
__weak صحيح eeprom_layout_detect(بنانشانی حرف *انکڑے)
{
	أرجع LAYOUT_نسخہ_UNRECOGNIZED;
}

/*
 * __eeprom_layout_assign() - set the layout fields
 * @layout:		A pointer to an existing struct layout.
 * @layout_version:	The version number of the desired layout
 */
__weak فراغ __eeprom_layout_assign(بنية eeprom_layout *layout,
				   صحيح layout_نسخہ)
{
	layout->fields = layout_unknown;
	layout->عدد_دا_fields = ARRAY_ناپ(layout_unknown);
}
فراغ eeprom_layout_سونپو(بنية eeprom_layout *layout, صحيح layout_نسخہ) \
		__attribute__((weak, alias("__eeprom_layout_assign")));

/*
 * eeprom_layout_print() - print the layout and the data which is assigned to it
 * @layout: A pointer to an existing struct layout.
 */
ساكن فراغ eeprom_layout_print(ثابت بنية eeprom_layout *layout)
{
	صحيح i;
	بنية eeprom_میدان *fields = layout->fields;

	لكل (i = 0; i < layout->عدد_دا_fields; i++)
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
بنية eeprom_میدان *eeprom_layout_لبھو_میدان(بنية eeprom_layout *layout,
					      حرف *میدان_name, bool warn)
{
	لكل (صحيح i = 0; i < layout->عدد_دا_fields; i++)
		إذا (layout->fields[i].name != RESERVED_FIELDS &&
		    !strcmp(layout->fields[i].name, میدان_name))
			أرجع &layout->fields[i];

	إذا (warn)
		لکھو("No such field '%s'\n", میدان_name);

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
ساكن صحيح eeprom_layout_update_میدان(بنية eeprom_layout *layout,
				      حرف *میدان_name, حرف *new_انکڑے)
{
	بنية eeprom_میدان *میدان;
	صحيح err;

	إذا (new_انکڑے == عدم)
		أرجع 0;

	إذا (میدان_name == عدم)
		أرجع -1;

	میدان = eeprom_layout_لبھو_میدان(layout, میدان_name, true);
	إذا (میدان == عدم)
		أرجع -1;

	err = میدان->update(میدان, new_انکڑے);
	إذا (err)
		لکھو("Invalid data for field %s\n", میدان_name);

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
فراغ eeprom_layout_تیاری(بنية eeprom_layout *layout, بنانشانی حرف *buf,
			 بنانشانی صحيح buf_ناپ, صحيح layout_نسخہ)
{
	صحيح i;

	إذا (layout_نسخہ == LAYOUT_نسخہ_AUTODETECT)
		layout->layout_نسخہ = eeprom_layout_detect(buf);
	وإلا
		layout->layout_نسخہ = layout_نسخہ;

	layout->انکڑے_ناپ = buf_ناپ;
	layout->print = eeprom_layout_print;
	layout->update = eeprom_layout_update_میدان;

	eeprom_layout_سونپو(layout, layout_نسخہ);
	layout->انکڑے = buf;
	لكل (i = 0; i < layout->عدد_دا_fields; i++) {
		layout->fields[i].buf = buf;
		buf += layout->fields[i].ناپ;
	}
}
