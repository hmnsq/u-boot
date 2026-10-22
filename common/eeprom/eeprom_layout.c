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

#ವ್ಯಾಖ್ಯಾನಿಸು NO_LAYOUT_FIELDS	"Unknown layout. Dumping raw data\n"

ರಚನೆ eeprom_ಕ್ಷೇತ್ರ layout_unknown[1] = {
	{ NO_LAYOUT_FIELDS, 256, ಖಾಲಿ, eeprom_ಕ್ಷೇತ್ರ_print_bin,
				       eeprom_ಕ್ಷೇತ್ರ_update_bin },
};

/*
 * eeprom_layout_detect() - detect layout based on the contents of the data.
 * @data: Pointer to the data to be analyzed.
 *
 * Returns: the detected layout version.
 */
__weak ಪೂರ್ಣಾಂಕ eeprom_layout_detect(ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *ದತ್ತಾಂಶ)
{
	ಹಿಂತಿರುಗಿ LAYOUT_ಆವೃತ್ತಿ_UNRECOGNIZED;
}

/*
 * __eeprom_layout_assign() - set the layout fields
 * @layout:		A pointer to an existing struct layout.
 * @layout_version:	The version number of the desired layout
 */
__weak ಶೂನ್ಯ __eeprom_layout_assign(ರಚನೆ eeprom_layout *layout,
				   ಪೂರ್ಣಾಂಕ layout_ಆವೃತ್ತಿ)
{
	layout->fields = layout_unknown;
	layout->ಅಂಕೆ_ದ_fields = ARRAY_ಗಾತ್ರ(layout_unknown);
}
ಶೂನ್ಯ eeprom_layout_ನಿಯೋಜಿಸು(ರಚನೆ eeprom_layout *layout, ಪೂರ್ಣಾಂಕ layout_ಆವೃತ್ತಿ) \
		__attribute__((weak, alias("__eeprom_layout_assign")));

/*
 * eeprom_layout_print() - print the layout and the data which is assigned to it
 * @layout: A pointer to an existing struct layout.
 */
ಸ್ಥಿರ ಶೂನ್ಯ eeprom_layout_print(ಸ್ಥಿರಾಂಕ ರಚನೆ eeprom_layout *layout)
{
	ಪೂರ್ಣಾಂಕ i;
	ರಚನೆ eeprom_ಕ್ಷೇತ್ರ *fields = layout->fields;

	ಗಾಗಿ (i = 0; i < layout->ಅಂಕೆ_ದ_fields; i++)
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
ರಚನೆ eeprom_ಕ್ಷೇತ್ರ *eeprom_layout_ಕಂಡುಹಿಡಿ_ಕ್ಷೇತ್ರ(ರಚನೆ eeprom_layout *layout,
					      ಅಕ್ಷರ *ಕ್ಷೇತ್ರ_name, bool warn)
{
	ಗಾಗಿ (ಪೂರ್ಣಾಂಕ i = 0; i < layout->ಅಂಕೆ_ದ_fields; i++)
		ಆದರೆ (layout->fields[i].name != RESERVED_FIELDS &&
		    !strcmp(layout->fields[i].name, ಕ್ಷೇತ್ರ_name))
			ಹಿಂತಿರುಗಿ &layout->fields[i];

	ಆದರೆ (warn)
		ಬರೆ("No such field '%s'\n", ಕ್ಷೇತ್ರ_name);

	ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}

/*
 * eeprom_layout_update_field() - update a single field in the layout data.
 * @layout:	A pointer to an existing struct layout.
 * @field_name:	The name of the field to update.
 * @new_data:	The new field data (a string. Format depends on the field)
 *
 * Returns: 0 on success, negative error value on failure.
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ eeprom_layout_update_ಕ್ಷೇತ್ರ(ರಚನೆ eeprom_layout *layout,
				      ಅಕ್ಷರ *ಕ್ಷೇತ್ರ_name, ಅಕ್ಷರ *new_ದತ್ತಾಂಶ)
{
	ರಚನೆ eeprom_ಕ್ಷೇತ್ರ *ಕ್ಷೇತ್ರ;
	ಪೂರ್ಣಾಂಕ err;

	ಆದರೆ (new_ದತ್ತಾಂಶ == ಖಾಲಿ)
		ಹಿಂತಿರುಗಿ 0;

	ಆದರೆ (ಕ್ಷೇತ್ರ_name == ಖಾಲಿ)
		ಹಿಂತಿರುಗಿ -1;

	ಕ್ಷೇತ್ರ = eeprom_layout_ಕಂಡುಹಿಡಿ_ಕ್ಷೇತ್ರ(layout, ಕ್ಷೇತ್ರ_name, true);
	ಆದರೆ (ಕ್ಷೇತ್ರ == ಖಾಲಿ)
		ಹಿಂತಿರುಗಿ -1;

	err = ಕ್ಷೇತ್ರ->update(ಕ್ಷೇತ್ರ, new_ದತ್ತಾಂಶ);
	ಆದರೆ (err)
		ಬರೆ("Invalid data for field %s\n", ಕ್ಷೇತ್ರ_name);

	ಹಿಂತಿರುಗಿ err;
}

/*
 * eeprom_layout_setup() - setup layout struct with the layout data and
 *			   metadata as dictated by layout_version
 * @layout:	A pointer to an existing struct layout.
 * @buf:	A buffer initialized with the eeprom data.
 * @buf_size:	Size of buf in bytes.
 * @layout version: The version number of the layout.
 */
ಶೂನ್ಯ eeprom_layout_ಸಿದ್ಧತೆ(ರಚನೆ eeprom_layout *layout, ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *buf,
			 ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ buf_ಗಾತ್ರ, ಪೂರ್ಣಾಂಕ layout_ಆವೃತ್ತಿ)
{
	ಪೂರ್ಣಾಂಕ i;

	ಆದರೆ (layout_ಆವೃತ್ತಿ == LAYOUT_ಆವೃತ್ತಿ_AUTODETECT)
		layout->layout_ಆವೃತ್ತಿ = eeprom_layout_detect(buf);
	ಇಲ್ಲದಿದ್ದರೆ
		layout->layout_ಆವೃತ್ತಿ = layout_ಆವೃತ್ತಿ;

	layout->ದತ್ತಾಂಶ_ಗಾತ್ರ = buf_ಗಾತ್ರ;
	layout->print = eeprom_layout_print;
	layout->update = eeprom_layout_update_ಕ್ಷೇತ್ರ;

	eeprom_layout_ನಿಯೋಜಿಸು(layout, layout_ಆವೃತ್ತಿ);
	layout->ದತ್ತಾಂಶ = buf;
	ಗಾಗಿ (i = 0; i < layout->ಅಂಕೆ_ದ_fields; i++) {
		layout->fields[i].buf = buf;
		buf += layout->fields[i].ಗಾತ್ರ_2;
	}
}
