// SPDX-License-Identifier: GPL-2.0+

#অন্তর্ভাৱ <dm.h>
#অন্তর্ভাৱ <init.h>
#অন্তর্ভাৱ <sysinfo.h>
#অন্তর্ভাৱ <asm/global_data.h>
#অন্তর্ভাৱ <linux/libfdt.h>
#অন্তর্ভাৱ <linux/compiler.h>

DECLARE_GLOBAL_DATA_PTR;

পূর্ণাঙ্ক __weak checkboard(শূন্য)
{
	ঘূৰাই 0;
}

স্থির ধ্রুৱ সংরচনা to_show {
	ধ্রুৱ ৱর্ণ *name;
	গণনা sysinfo_id id;
} to_show[] = {
	{ "Manufacturer", SYSID_BOARD_MANUFACTURER},
	{ "Prior-stage version", SYSID_PRIOR_STAGE_VERSION },
	{ "Prior-stage date", SYSID_PRIOR_STAGE_DATE },
	{ /* sentinel */ }
};

স্থির পূর্ণাঙ্ক try_sysinfo(শূন্য)
{
	সংরচনা udevice *dev;
	ৱর্ণ str[80];
	পূর্ণাঙ্ক ret;

	/* This might provide more detail */
	ret = sysinfo_get(&dev);
	যদি (ret)
		ঘূৰাই ret;

	ret = sysinfo_detect(dev);
	যদি (ret)
		ঘূৰাই ret;

	ret = sysinfo_get_str(dev, SYSID_BOARD_MODEL, পরিমাণ(str), str);
	যদি (ret)
		ঘূৰাই ret;
	printf("Model: %s\n", str);

	যদি (IS_ENABLED(CONFIG_SYSINFO_EXTRA)) {
		ধ্রুৱ সংরচনা to_show *item;

		বাবে (item = to_show; item->id; item++) {
			ret = sysinfo_get_str(dev, item->id, পরিমাণ(str), str);
			যদি (!ret)
				printf("%s: %s\n", item->name, str);
		}
	}

	ঘূৰাই 0;
}

পূর্ণাঙ্ক show_board_info(শূন্য)
{
	যদি (IS_ENABLED(CONFIG_OF_CONTROL)) {
		পূর্ণাঙ্ক ret = -ENOSYS;

		যদি (IS_ENABLED(CONFIG_SYSINFO))
			ret = try_sysinfo();

		/* Fail back to the main 'model' if available */
		যদি (ret) {
			ধ্রুৱ ৱর্ণ *model;

			model = fdt_getprop(gd->fdt_blob, 0, "model", রিক্ত);
			যদি (model)
				printf("Model: %s\n", model);
		}
	}

	ঘূৰাই checkboard();
}
