// SPDX-License-Identifier: GPL-2.0+

#অন্তর্ভাব <dm.h>
#অন্তর্ভাব <init.h>
#অন্তর্ভাব <sysinfo.h>
#অন্তর্ভাব <asm/global_data.h>
#অন্তর্ভাব <linux/libfdt.h>
#অন্তর্ভাব <linux/compiler.h>

DECLARE_GLOBAL_তথ্য_PTR;

পূর্ণসংখ্যা __weak checkboard(শূন্য)
{
	ফেরত 0;
}

স্থির ধ্রুবক কাঠামো to_show {
	ধ্রুবক অক্ষর *name;
	গণনা sysinfo_id id;
} to_show[] = {
	{ "Manufacturer", SYSID_BOARD_MANUFACTURER},
	{ "Prior-stage version", SYSID_PRIOR_STAGE_VERSION },
	{ "Prior-stage date", SYSID_PRIOR_STAGE_DATE },
	{ /* sentinel */ }
};

স্থির পূর্ণসংখ্যা try_sysinfo(শূন্য)
{
	কাঠামো udevice *dev;
	অক্ষর str[80];
	পূর্ণসংখ্যা ret;

	/* This might provide more detail */
	ret = sysinfo_get(&dev);
	যদি (ret)
		ফেরত ret;

	ret = sysinfo_detect(dev);
	যদি (ret)
		ফেরত ret;

	ret = sysinfo_get_str(dev, SYSID_BOARD_MODEL, আকার(str), str);
	যদি (ret)
		ফেরত ret;
	printf("Model: %s\n", str);

	যদি (IS_ENABLED(CONFIG_SYSINFO_EXTRA)) {
		ধ্রুবক কাঠামো to_show *item;

		জন্য (item = to_show; item->id; item++) {
			ret = sysinfo_get_str(dev, item->id, আকার(str), str);
			যদি (!ret)
				printf("%s: %s\n", item->name, str);
		}
	}

	ফেরত 0;
}

পূর্ণসংখ্যা show_board_info(শূন্য)
{
	যদি (IS_ENABLED(CONFIG_OF_CONTROL)) {
		পূর্ণসংখ্যা ret = -ENOSYS;

		যদি (IS_ENABLED(CONFIG_SYSINFO))
			ret = try_sysinfo();

		/* Fail back to the main 'model' if available */
		যদি (ret) {
			ধ্রুবক অক্ষর *model;

			model = fdt_getprop(gd->fdt_blob, 0, "model", ফাঁকা);
			যদি (model)
				printf("Model: %s\n", model);
		}
	}

	ফেরত checkboard();
}
