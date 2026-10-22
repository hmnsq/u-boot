// SPDX-License-Identifier: GPL-2.0+

#شامل <dm.h>
#شامل <init.h>
#شامل <sysinfo.h>
#شامل <asm/global_data.h>
#شامل <linux/libfdt.h>
#شامل <linux/compiler.h>

DECLARE_GLOBAL_DATA_PTR;

صحیح __weak checkboard(تهی)
{
	بازگشت 0;
}

ایستا ثابت ساختار to_show {
	ثابت نویسه *name;
	شمارش sysinfo_id id;
} to_show[] = {
	{ "Manufacturer", SYSID_BOARD_MANUFACTURER},
	{ "Prior-stage version", SYSID_PRIOR_STAGE_VERSION },
	{ "Prior-stage date", SYSID_PRIOR_STAGE_DATE },
	{ /* sentinel */ }
};

ایستا صحیح try_sysinfo(تهی)
{
	ساختار udevice *dev;
	نویسه str[80];
	صحیح ret;

	/* This might provide more detail */
	ret = sysinfo_get(&dev);
	اگر (ret)
		بازگشت ret;

	ret = sysinfo_detect(dev);
	اگر (ret)
		بازگشت ret;

	ret = sysinfo_get_str(dev, SYSID_BOARD_MODEL, اندازه(str), str);
	اگر (ret)
		بازگشت ret;
	printf("Model: %s\n", str);

	اگر (IS_ENABLED(CONFIG_SYSINFO_EXTRA)) {
		ثابت ساختار to_show *item;

		برای (item = to_show; item->id; item++) {
			ret = sysinfo_get_str(dev, item->id, اندازه(str), str);
			اگر (!ret)
				printf("%s: %s\n", item->name, str);
		}
	}

	بازگشت 0;
}

صحیح show_board_info(تهی)
{
	اگر (IS_ENABLED(CONFIG_OF_CONTROL)) {
		صحیح ret = -ENOSYS;

		اگر (IS_ENABLED(CONFIG_SYSINFO))
			ret = try_sysinfo();

		/* Fail back to the main 'model' if available */
		اگر (ret) {
			ثابت نویسه *model;

			model = fdt_getprop(gd->fdt_blob, 0, "model", پوچ);
			اگر (model)
				printf("Model: %s\n", model);
		}
	}

	بازگشت checkboard();
}
