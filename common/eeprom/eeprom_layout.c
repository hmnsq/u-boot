// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2009-2016 CompuLab, Ltd.
 *
 * Authors: Nikita Kiryanov <nikita@compulab.co.il>
 *	    Igor Grinberg <grinberg@compulab.co.il>
 */

#అన్తర్భావ <linux/kernel.h>
#అన్తర్భావ <linux/string.h>
#అన్తర్భావ <eeprom_layout.h>
#అన్తర్భావ <eeprom_field.h>

#నిర్వచించు NO_LAYOUT_FIELDS	"Unknown layout. Dumping raw data\n"

నిర్మాణం eeprom_క్షేత్రం layout_unknown[1] = {
	{ NO_LAYOUT_FIELDS, 256, ఖాళీ, eeprom_క్షేత్రం_print_bin,
				       eeprom_క్షేత్రం_update_bin },
};

/*
 * eeprom_layout_detect() - detect layout based on the contents of the data.
 * @data: Pointer to the data to be analyzed.
 *
 * Returns: the detected layout version.
 */
__weak పూర్ణాంకం eeprom_layout_detect(గుర్తులేని అక్షరం *దత్తాంశం)
{
	తిరిగి LAYOUT_సంస్కరణ_UNRECOGNIZED;
}

/*
 * __eeprom_layout_assign() - set the layout fields
 * @layout:		A pointer to an existing struct layout.
 * @layout_version:	The version number of the desired layout
 */
__weak శూన్య __eeprom_layout_assign(నిర్మాణం eeprom_layout *layout,
				   పూర్ణాంకం layout_సంస్కరణ)
{
	layout->fields = layout_unknown;
	layout->అంకె_యొక్క_fields = ARRAY_కొలత(layout_unknown);
}
శూన్య eeprom_layout_కేటాయించు(నిర్మాణం eeprom_layout *layout, పూర్ణాంకం layout_సంస్కరణ) \
		__attribute__((weak, alias("__eeprom_layout_assign")));

/*
 * eeprom_layout_print() - print the layout and the data which is assigned to it
 * @layout: A pointer to an existing struct layout.
 */
స్థిర శూన్య eeprom_layout_print(స్థిరరాశి నిర్మాణం eeprom_layout *layout)
{
	పూర్ణాంకం i;
	నిర్మాణం eeprom_క్షేత్రం *fields = layout->fields;

	కొరకు (i = 0; i < layout->అంకె_యొక్క_fields; i++)
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
నిర్మాణం eeprom_క్షేత్రం *eeprom_layout_కనుగొను_క్షేత్రం(నిర్మాణం eeprom_layout *layout,
					      అక్షరం *క్షేత్రం_name, bool warn)
{
	కొరకు (పూర్ణాంకం i = 0; i < layout->అంకె_యొక్క_fields; i++)
		ఒకవేళ (layout->fields[i].name != RESERVED_FIELDS &&
		    !strcmp(layout->fields[i].name, క్షేత్రం_name))
			తిరిగి &layout->fields[i];

	ఒకవేళ (warn)
		రాయి("No such field '%s'\n", క్షేత్రం_name);

	తిరిగి ఖాళీ;
}

/*
 * eeprom_layout_update_field() - update a single field in the layout data.
 * @layout:	A pointer to an existing struct layout.
 * @field_name:	The name of the field to update.
 * @new_data:	The new field data (a string. Format depends on the field)
 *
 * Returns: 0 on success, negative error value on failure.
 */
స్థిర పూర్ణాంకం eeprom_layout_update_క్షేత్రం(నిర్మాణం eeprom_layout *layout,
				      అక్షరం *క్షేత్రం_name, అక్షరం *new_దత్తాంశం)
{
	నిర్మాణం eeprom_క్షేత్రం *క్షేత్రం;
	పూర్ణాంకం err;

	ఒకవేళ (new_దత్తాంశం == ఖాళీ)
		తిరిగి 0;

	ఒకవేళ (క్షేత్రం_name == ఖాళీ)
		తిరిగి -1;

	క్షేత్రం = eeprom_layout_కనుగొను_క్షేత్రం(layout, క్షేత్రం_name, true);
	ఒకవేళ (క్షేత్రం == ఖాళీ)
		తిరిగి -1;

	err = క్షేత్రం->update(క్షేత్రం, new_దత్తాంశం);
	ఒకవేళ (err)
		రాయి("Invalid data for field %s\n", క్షేత్రం_name);

	తిరిగి err;
}

/*
 * eeprom_layout_setup() - setup layout struct with the layout data and
 *			   metadata as dictated by layout_version
 * @layout:	A pointer to an existing struct layout.
 * @buf:	A buffer initialized with the eeprom data.
 * @buf_size:	Size of buf in bytes.
 * @layout version: The version number of the layout.
 */
శూన్య eeprom_layout_అమరిక(నిర్మాణం eeprom_layout *layout, గుర్తులేని అక్షరం *buf,
			 గుర్తులేని పూర్ణాంకం buf_కొలత, పూర్ణాంకం layout_సంస్కరణ)
{
	పూర్ణాంకం i;

	ఒకవేళ (layout_సంస్కరణ == LAYOUT_సంస్కరణ_AUTODETECT)
		layout->layout_సంస్కరణ = eeprom_layout_detect(buf);
	లేకపోతే
		layout->layout_సంస్కరణ = layout_సంస్కరణ;

	layout->దత్తాంశం_కొలత = buf_కొలత;
	layout->print = eeprom_layout_print;
	layout->update = eeprom_layout_update_క్షేత్రం;

	eeprom_layout_కేటాయించు(layout, layout_సంస్కరణ);
	layout->దత్తాంశం = buf;
	కొరకు (i = 0; i < layout->అంకె_యొక్క_fields; i++) {
		layout->fields[i].buf = buf;
		buf += layout->fields[i].కొలత;
	}
}
