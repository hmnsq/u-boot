// SPDX-License-Identifier: GPL-2.0+

#ಅನ್ತರ್ಭಾವ <dm.h>
#ಅನ್ತರ್ಭಾವ <init.h>
#ಅನ್ತರ್ಭಾವ <sysinfo.h>
#ಅನ್ತರ್ಭಾವ <asm/global_data.h>
#ಅನ್ತರ್ಭಾವ <linux/libfdt.h>
#ಅನ್ತರ್ಭಾವ <linux/compiler.h>

DECLARE_GLOBAL_DATA_PTR;

ಪೂರ್ಣಾಙ್ಕ __weak checkboard(ಶೂನ್ಯ)
{
	ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಧ್ರುವ ಸಂರಚನಾ to_show {
	ಧ್ರುವ ವರ್ಣ *name;
	ಗಣನಾ sysinfo_id id;
} to_show[] = {
	{ "Manufacturer", SYSID_BOARD_MANUFACTURER},
	{ "Prior-stage version", SYSID_PRIOR_STAGE_VERSION },
	{ "Prior-stage date", SYSID_PRIOR_STAGE_DATE },
	{ /* sentinel */ }
};

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ try_sysinfo(ಶೂನ್ಯ)
{
	ಸಂರಚನಾ udevice *dev;
	ವರ್ಣ str[80];
	ಪೂರ್ಣಾಙ್ಕ ret;

	/* This might provide more detail */
	ret = sysinfo_get(&dev);
	ಯದಿ (ret)
		ಪ್ರತ್ಯಾಗಮ ret;

	ret = sysinfo_detect(dev);
	ಯದಿ (ret)
		ಪ್ರತ್ಯಾಗಮ ret;

	ret = sysinfo_get_str(dev, SYSID_BOARD_MODEL, ಪರಿಮಾಣ(str), str);
	ಯದಿ (ret)
		ಪ್ರತ್ಯಾಗಮ ret;
	printf("Model: %s\n", str);

	ಯದಿ (IS_ENABLED(CONFIG_SYSINFO_EXTRA)) {
		ಧ್ರುವ ಸಂರಚನಾ to_show *item;

		ಪ್ರತಿ (item = to_show; item->id; item++) {
			ret = sysinfo_get_str(dev, item->id, ಪರಿಮಾಣ(str), str);
			ಯದಿ (!ret)
				printf("%s: %s\n", item->name, str);
		}
	}

	ಪ್ರತ್ಯಾಗಮ 0;
}

ಪೂರ್ಣಾಙ್ಕ show_board_info(ಶೂನ್ಯ)
{
	ಯದಿ (IS_ENABLED(CONFIG_OF_CONTROL)) {
		ಪೂರ್ಣಾಙ್ಕ ret = -ENOSYS;

		ಯದಿ (IS_ENABLED(CONFIG_SYSINFO))
			ret = try_sysinfo();

		/* Fail back to the main 'model' if available */
		ಯದಿ (ret) {
			ಧ್ರುವ ವರ್ಣ *model;

			model = fdt_getprop(gd->fdt_blob, 0, "model", ರಿಕ್ತ);
			ಯದಿ (model)
				printf("Model: %s\n", model);
		}
	}

	ಪ್ರತ್ಯಾಗಮ checkboard();
}
