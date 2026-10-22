// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2009-2016 CompuLab, Ltd.
 *
 * Authors: Nikita Kiryanov <nikita@compulab.co.il>
 *	    Igor Grinberg <grinberg@compulab.co.il>
 */

#شامل <linux/kernel.h>
#شامل <linux/string.h>
#شامل <eeprom_layout.h>
#شامل <eeprom_field.h>

#تعریف_کرو NO_LAYOUT_FIELDS	"Unknown layout. Dumping raw data\n"

ساخت eeprom_میدان layout_unknown[1] = {
	{ NO_LAYOUT_FIELDS, 256, تہی, eeprom_میدان_print_bin,
				       eeprom_میدان_update_bin },
};

/*
 * eeprom_layout_detect() - detect layout based on the contents of the data.
 * @data: Pointer to the data to be analyzed.
 *
 * Returns: the detected layout version.
 */
__weak صحیح eeprom_layout_detect(غیرعلامتی حرف *ڈیٹا)
{
	واپس LAYOUT_نسخہ_UNRECOGNIZED;
}

/*
 * __eeprom_layout_assign() - set the layout fields
 * @layout:		A pointer to an existing struct layout.
 * @layout_version:	The version number of the desired layout
 */
__weak خالی __eeprom_layout_assign(ساخت eeprom_layout *layout,
				   صحیح layout_نسخہ)
{
	layout->fields = layout_unknown;
	layout->عدد_کا_fields = ARRAY_سائز(layout_unknown);
}
خالی eeprom_layout_تفویض(ساخت eeprom_layout *layout, صحیح layout_نسخہ) \
		__attribute__((weak, alias("__eeprom_layout_assign")));

/*
 * eeprom_layout_print() - print the layout and the data which is assigned to it
 * @layout: A pointer to an existing struct layout.
 */
ساکن خالی eeprom_layout_print(ثابت ساخت eeprom_layout *layout)
{
	صحیح i;
	ساخت eeprom_میدان *fields = layout->fields;

	برائے (i = 0; i < layout->عدد_کا_fields; i++)
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
ساخت eeprom_میدان *eeprom_layout_ڈھونڈو_میدان(ساخت eeprom_layout *layout,
					      حرف *میدان_name, bool warn)
{
	برائے (صحیح i = 0; i < layout->عدد_کا_fields; i++)
		اگر (layout->fields[i].name != RESERVED_FIELDS &&
		    !strcmp(layout->fields[i].name, میدان_name))
			واپس &layout->fields[i];

	اگر (warn)
		لکھو("No such field '%s'\n", میدان_name);

	واپس تہی;
}

/*
 * eeprom_layout_update_field() - update a single field in the layout data.
 * @layout:	A pointer to an existing struct layout.
 * @field_name:	The name of the field to update.
 * @new_data:	The new field data (a string. Format depends on the field)
 *
 * Returns: 0 on success, negative error value on failure.
 */
ساکن صحیح eeprom_layout_update_میدان(ساخت eeprom_layout *layout,
				      حرف *میدان_name, حرف *new_ڈیٹا)
{
	ساخت eeprom_میدان *میدان;
	صحیح err;

	اگر (new_ڈیٹا == تہی)
		واپس 0;

	اگر (میدان_name == تہی)
		واپس -1;

	میدان = eeprom_layout_ڈھونڈو_میدان(layout, میدان_name, true);
	اگر (میدان == تہی)
		واپس -1;

	err = میدان->update(میدان, new_ڈیٹا);
	اگر (err)
		لکھو("Invalid data for field %s\n", میدان_name);

	واپس err;
}

/*
 * eeprom_layout_setup() - setup layout struct with the layout data and
 *			   metadata as dictated by layout_version
 * @layout:	A pointer to an existing struct layout.
 * @buf:	A buffer initialized with the eeprom data.
 * @buf_size:	Size of buf in bytes.
 * @layout version: The version number of the layout.
 */
خالی eeprom_layout_تیاری(ساخت eeprom_layout *layout, غیرعلامتی حرف *buf,
			 غیرعلامتی صحیح buf_سائز, صحیح layout_نسخہ)
{
	صحیح i;

	اگر (layout_نسخہ == LAYOUT_نسخہ_AUTODETECT)
		layout->layout_نسخہ = eeprom_layout_detect(buf);
	ورنہ
		layout->layout_نسخہ = layout_نسخہ;

	layout->ڈیٹا_سائز = buf_سائز;
	layout->print = eeprom_layout_print;
	layout->update = eeprom_layout_update_میدان;

	eeprom_layout_تفویض(layout, layout_نسخہ);
	layout->ڈیٹا = buf;
	برائے (i = 0; i < layout->عدد_کا_fields; i++) {
		layout->fields[i].buf = buf;
		buf += layout->fields[i].سائز;
	}
}
