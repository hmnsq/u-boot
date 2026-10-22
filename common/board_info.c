// SPDX-License-Identifier: GPL-2.0+

#ضمن <dm.h>
#ضمن <init.h>
#ضمن <sysinfo.h>
#ضمن <asm/global_data.h>
#ضمن <linux/libfdt.h>
#ضمن <linux/compiler.h>

DECLARE_GLOBAL_DATA_PTR;

صحيح __weak checkboard(فراغ)
{
	إرجاع 0;
}

ساكن ثابت بنية to_show {
	ثابت حرف *name;
	تعداد sysinfo_id id;
} to_show[] = {
	{ "Manufacturer", SYSID_BOARD_MANUFACTURER},
	{ "Prior-stage version", SYSID_PRIOR_STAGE_VERSION },
	{ "Prior-stage date", SYSID_PRIOR_STAGE_DATE },
	{ /* sentinel */ }
};

ساكن صحيح try_sysinfo(فراغ)
{
	بنية udevice *dev;
	حرف str[80];
	صحيح ret;

	/* This might provide more detail */
	ret = sysinfo_get(&dev);
	إذا (ret)
		إرجاع ret;

	ret = sysinfo_detect(dev);
	إذا (ret)
		إرجاع ret;

	ret = sysinfo_get_str(dev, SYSID_BOARD_MODEL, حجم(str), str);
	إذا (ret)
		إرجاع ret;
	printf("Model: %s\n", str);

	إذا (IS_ENABLED(CONFIG_SYSINFO_EXTRA)) {
		ثابت بنية to_show *item;

		لكل (item = to_show; item->id; item++) {
			ret = sysinfo_get_str(dev, item->id, حجم(str), str);
			إذا (!ret)
				printf("%s: %s\n", item->name, str);
		}
	}

	إرجاع 0;
}

صحيح show_board_info(فراغ)
{
	إذا (IS_ENABLED(CONFIG_OF_CONTROL)) {
		صحيح ret = -ENOSYS;

		إذا (IS_ENABLED(CONFIG_SYSINFO))
			ret = try_sysinfo();

		/* Fail back to the main 'model' if available */
		إذا (ret) {
			ثابت حرف *model;

			model = fdt_getprop(gd->fdt_blob, 0, "model", عدم);
			إذا (model)
				printf("Model: %s\n", model);
		}
	}

	إرجاع checkboard();
}
